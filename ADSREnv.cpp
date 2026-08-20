#include "ADSREnv.h"
#include "Utils.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern Switch       button7;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;

ADSREnv::ADSREnv() {}
ADSREnv::~ADSREnv() {}

void ADSREnv::Init()
{ adsr.Init(patch.AudioSampleRate()); }

void ADSREnv::DacCallback(uint16_t **output, size_t size)
{
    float param1    = GetCombinedKnobCv(CV_1, CV_5);
    float param2    = GetCombinedKnobCv(CV_2, CV_6);
    float param3    = GetCombinedKnobCv(CV_3, CV_7);
    float param4    = GetCombinedKnobCv(CV_4, CV_8);
    float attackMs  = fmap(param1, 10.f, 2500.f, Mapping::LOG);
    float decayMs   = fmap(param2, 10.f, 2500.f, Mapping::LOG);
    float sustain   = fmap(param3, 0.f, 1.f);
    float releaseMs = fmap(param4, 10.f, 4000.f, Mapping::LOG);
    // toggle down gives a half-loudness envelope
    scaleMult = toggle.Pressed() ? 1.f : 0.5f;

    adsr.SetADSR(attackMs, decayMs, sustain, releaseMs);

    // Gate comes from the first gate input or the button
    adsr.Gate(patch.gate_in_1.State() || button7.Pressed());

    uint16_t cvVal      = VoltageToCvValue(adsr.Value() * 5.f * scaleMult);
    LED_OUT_LOWPRIORITY = cvVal;
    CV_OUT_LOWPRIORITY  = cvVal;
}

void ADSREnv::AudioCallback(AudioHandle::InputBuffer  in,
                            AudioHandle::OutputBuffer out,
                            size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        adsr.Process();
        // L: input * envelope (VCA-style)
        OUT_L[i] = IN_L[i] * adsr.Value() * scaleMult;
        // R: ducks using the inverse of the envelope, soft-clipped
        OUT_R[i] = cheapTanh((1.f - (adsr.Value() * scaleMult)) * IN_R[i]);
    }
}

_ADSREnv::_ADSREnv() {}
_ADSREnv::~_ADSREnv() {}

void _ADSREnv::Init(float sampleRate)
{
    sRate        = sampleRate;
    envelope     = 0.f;
    attackCoef   = 0.f;
    decayCoef    = 0.f;
    sustainLevel = 0.f;
    releaseCoef  = 0.f;
    gate         = false;
    stage        = Stage::Idle;
}

void _ADSREnv::Gate(bool high)
{
    if(high && !gate)
    {
        stage = Stage::Attack; // rising edge → restart
    }
    else if(!high && gate)
    {
        stage = Stage::Release; // falling edge → release
    }

    gate = high;
}

void _ADSREnv::Process()
{
    switch(stage)
    {
        case Stage::Attack:
            envelope += attackCoef * (1.f - envelope);
            if(envelope >= 0.999f)
            {
                envelope = 1.f;
                stage    = Stage::Decay;
            }
            break;

        case Stage::Decay:
            envelope += decayCoef * (sustainLevel - envelope);
            if(fabsf(envelope - sustainLevel) < 0.001f)
            {
                envelope = sustainLevel;
                stage    = Stage::Sustain;
            }
            break;

        case Stage::Sustain: envelope = sustainLevel; break;

        case Stage::Release:
            envelope += releaseCoef * (0.f - envelope);
            if(envelope < 0.001f)
            {
                envelope = 0.f;
                stage    = Stage::Idle;
            }
            break;

        case Stage::Idle:
        default: envelope = 0.f; break;
    }
}

void _ADSREnv::SetADSR(float attackMs,
                       float decayMs,
                       float sustain,
                       float releaseMs)
{
    attackCoef   = 1.f - expf(-1.f / (sRate * attackMs / 1000.f));
    decayCoef    = 1.f - expf(-1.f / (sRate * decayMs / 1000.f));
    sustainLevel = sustain;
    releaseCoef  = 1.f - expf(-1.f / (sRate * releaseMs / 1000.f));
}

float _ADSREnv::Value()
{ return envelope; }