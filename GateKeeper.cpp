#include "GateKeeper.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern Blinker      blinker;
extern uint16_t     LED_OUT_LOWPRIORITY;
extern uint16_t     CV_OUT_LOWPRIORITY;

GateKeeper::GateKeeper() {}
GateKeeper::~GateKeeper() {}

void GateKeeper::Init()
{
    triggerInCount = 0;
    gateInOpen     = false;
    lightState     = false;
}

bool GateKeeper::GetRandomChanceByCV(int knob, int cv)
{
    float density = GetCombinedKnobCv(knob, cv);
    float r       = patch.GetRandomFloat(0.f, 0.5f);
    return r <= density;
}

bool GateKeeper::ShouldTrigger_Cycled()
{
    float fLen  = GetCombinedKnobCv(KNOB_CYCLE_LENGTH, CV_CYCLE_LENGTH);
    float fDuty = GetCombinedKnobCv(KNOB_CYCLE_DUTY, CV_CYCLE_DUTY);

    int length            = fmap(fLen, 2.f, 16.99f);
    triggerInCount        = triggerInCount % length;
    float transitionPoint = length * fDuty;

    return triggerInCount < transitionPoint;
}

bool GateKeeper::GetCurrentGateInputState()
{
    return toggle.Pressed()
               ? (patch.gate_in_1.State() || patch.gate_in_2.State())
               : (patch.gate_in_1.State() && patch.gate_in_2.State());
}

void GateKeeper::DacCallback(uint16_t **output, size_t size)
{
    bool newGateInOpen = GetCurrentGateInputState();

    LED_OUT_LOWPRIORITY = (lightState * 4095.0);
    CV_OUT_LOWPRIORITY  = (newGateInOpen * 4095.0);

    if(newGateInOpen == gateInOpen)
        return;

    if(newGateInOpen)
    {
        triggerInCount++;
        bool tr;

        if(GetRandomChanceByCV(KNOB_RANDOMNESS, CV_RANDOMNESS))
            tr = GetRandomChanceByCV(KNOB_RANDOM_DENSITY, CV_RAND_DENSITY);
        else
            tr = ShouldTrigger_Cycled();

        dsy_gpio_write(&patch.gate_out_1, tr);
        dsy_gpio_write(&patch.gate_out_2, !tr);
        lightState = tr;
    }
    else
    {
        dsy_gpio_write(&patch.gate_out_1, false);
        dsy_gpio_write(&patch.gate_out_2, false);
        lightState = false;
    }

    gateInOpen = newGateInOpen;
}
