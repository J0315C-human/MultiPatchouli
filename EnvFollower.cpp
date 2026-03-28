#include "EnvFollower.h"
#include "Utils.h"

extern DaisyPatchSM patch;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;

EnvFollower::EnvFollower() {}
EnvFollower::~EnvFollower() {}

void EnvFollower::Init()
{
    // assumes patch has been Initialized
    ef.Init();
}

void EnvFollower::AudioCallback(AudioHandle::InputBuffer  in,
                                AudioHandle::OutputBuffer out,
                                size_t                    size)
{
    float knob_attackMS   = patch.GetAdcValue(CV_1);
    float knob_releaseMS  = patch.GetAdcValue(CV_2);
    float envelopeScaling = fmap(patch.GetAdcValue(CV_3), 0.f, 3.f);
    float makeupGain_R    = fmap(patch.GetAdcValue(CV_4), 0.f, 2.f);

    float attackMs  = fmap(knob_attackMS, 1.f, 500.f, Mapping::LOG);
    float releaseMs = fmap(knob_releaseMS, 1.f, 2000.f, Mapping::LOG);

    ef.SetAttackRelease(attackMs, releaseMs);

    for(size_t i = 0; i < size; i++)
    {
        // audio L just passes through
        ef.Process(IN_L[i]);
        OUT_L[i] = IN_L[i];
        // audio R ducks using the inverse of the envelope
        float sample
            = (1.f - (ef.Value() * envelopeScaling)) * IN_R[i] * makeupGain_R;
        // naturally limits to (-1, 1), gets softer near the edges
        OUT_R[i] = cheapTanh(sample);
    }
    uint16_t cvVal      = VoltageToCvValue(ef.Value() * envelopeScaling * 5.f);
    LED_OUT_LOWPRIORITY = cvVal;
    CV_OUT_LOWPRIORITY  = cvVal;
}


_EnvFollower::_EnvFollower() {}
_EnvFollower::~_EnvFollower() {}

void _EnvFollower::Init()
{
    // assumes patch has been Initialized
    sRate       = patch.AudioSampleRate();
    envelope    = 0.f;
    attackCoef  = 0.f;
    releaseCoef = 0.f;
}

void _EnvFollower::Process(float input)
{
    float abs  = fabsf(input);
    float coef = abs > envelope ? attackCoef : releaseCoef;
    envelope   = abs + coef * (envelope - abs);
}

void _EnvFollower::SetAttackRelease(float attackMs, float releaseMs)
{
    attackCoef  = expf(-1.f / (sRate * attackMs / 1000.f));
    releaseCoef = expf(-1.f / (sRate * releaseMs / 1000.f));
}

float _EnvFollower::Value()
{ return envelope; }