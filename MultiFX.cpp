#include "MultiFX.h"
#include "SettingsManager.h"
#include "daisysp-lgpl.h"

extern DaisyPatchSM patch;
extern Settings     settings;
extern uint16_t     CV_OUT_LOWPRIORITY;

MultiFX::MultiFX() {}
MultiFX::~MultiFX() {}

void MultiFX::Init()
{
    reverb.Init(patch.AudioSampleRate());
    pitchShifterR.Init(patch.AudioSampleRate());
    pitchShifterL.Init(patch.AudioSampleRate());
    ef.Init();
    ef.SetAttackRelease(ENV_ATT, ENV_REL);
}

void MultiFX::DacCallback(uint16_t **output, size_t size)
{
    /** Update Params with the four knobs */
    float param1 = GetCombinedKnobCv(CV_1, CV_5);

    float param2 = GetCombinedKnobCv(CV_2, CV_6);

    float knob_dry_level  = GetCombinedKnobCv(CV_3, CV_7);
    float knob_send_level = GetCombinedKnobCv(CV_4, CV_8);
    dry_level             = DSY_CLAMP(knob_dry_level, 0, 2);
    send_level            = DSY_CLAMP(knob_send_level, 0, 2);

    switch(settings.effectMode)
    {
        case EffectMode::Reverb:
        {
            float time = fmap(param1, 0.3f, 0.99f);
            float damp = fmap(param2, 1000.f, 19000.f, Mapping::LOG);

            reverb.SetFeedback(DSY_CLAMP(time, 0.001f, 0.99f));
            reverb.SetLpFreq(DSY_CLAMP(damp, 500.f, 22000.f));
            break;
        }
        case EffectMode::PitchShift:
        {
            float shiftL = fmap(param1, -8.f, 26.f, Mapping::EXP);
            float shiftR = fmap(param2, -8.f, 26.f, Mapping::EXP);

            pitchShifterL.SetTransposition(shiftL);
            pitchShifterR.SetTransposition(shiftR);
            break;
        }
        default:
            // no processing
            break;
    }

    // set CV to follow envelope of full-wet effect (of just L channel)
    CV_OUT_LOWPRIORITY
        = VoltageToCvValue(cheapTanh(ef.Value() * ENV_SCALE) * 5.f);
}

void MultiFX::AudioCallback(AudioHandle::InputBuffer  in,
                            AudioHandle::OutputBuffer out,
                            size_t                    size)
{
    switch(settings.effectMode)
    {
        case EffectMode::Reverb:
        {
            for(size_t i = 0; i < size; i++)
            {
                float dryl  = IN_L[i] * dry_level;
                float dryr  = IN_R[i] * dry_level;
                float sendl = IN_L[i] * send_level;
                float sendr = IN_R[i] * send_level;
                float wetl, wetr;
                reverb.Process(sendl, sendr, &wetl, &wetr);
                ef.Process(wetl);
                OUT_L[i] = dryl + wetl;
                OUT_R[i] = dryr + wetr;
            }
            break;
        }
        case EffectMode::PitchShift:
        {
            for(size_t i = 0; i < size; i++)
            {
                float dryl  = IN_L[i] * dry_level;
                float dryr  = IN_R[i] * dry_level;
                float sendl = IN_L[i] * send_level;
                float sendr = IN_R[i] * send_level;
                float wetl  = pitchShifterL.Process(sendl);
                float wetr  = pitchShifterR.Process(sendr);
                ef.Process(wetl);
                OUT_L[i] = dryl + wetl;
                OUT_R[i] = dryr + wetr;
            }
            break;
        }
        default:
        {
            for(size_t i = 0; i < size; i++)
            {
                OUT_L[i] = IN_L[i];
                OUT_R[i] = IN_R[i];
            }
        }
    }
}