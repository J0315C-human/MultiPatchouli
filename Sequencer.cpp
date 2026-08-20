#include "Sequencer.h"
#include "Utils.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;

Sequencer::Sequencer() {}
Sequencer::~Sequencer() {}

void Sequencer::Init() {}

void Sequencer::DacCallback(uint16_t** output, size_t size)
{
    int numValues = toggle.Pressed() ? 4 : 3;

    // trigger to move forwards
    bool oldAdvanceState = advanceGateOn;
    advanceGateOn        = patch.gate_in_1.State();
    if(!oldAdvanceState && advanceGateOn)
    {
        curValueIdx = (curValueIdx + 1) % numValues;
    }

    // trigger to move backwards
    bool oldBackState = backwardGateOn;
    backwardGateOn    = patch.gate_in_2.State();
    if(!oldBackState && backwardGateOn)
    {
        curValueIdx = (curValueIdx + numValues - 1) % numValues;
    }

    int knob = KNOB_V1;
    int cv   = CV_V1;

    switch(curValueIdx)
    {
        case 1:
        {
            knob = KNOB_V2;
            cv   = CV_V2;
            break;
        }
        case 2:
        {
            knob = KNOB_V3;
            cv   = CV_V3;
            break;
        }
        case 3:
        {
            knob = KNOB_V4;
            cv   = CV_V4;
            break;
        }
    }

    float cvOut = GetCombinedKnobCv(knob, cv) * MAX_VOUT;

    CV_OUT_LOWPRIORITY = LED_OUT_LOWPRIORITY = VoltageToCvValue(cvOut);
}

void Sequencer::AudioCallback(AudioHandle::InputBuffer  in,
                              AudioHandle::OutputBuffer out,
                              size_t                    size)
{
}