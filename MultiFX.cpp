#include "MultiFX.h"
#include "SettingsManager.h"
#include "daisysp-lgpl.h"

extern DaisyPatchSM patch;
extern Settings     settings;
extern bool         shouldSave;
extern uint16_t     CV_OUT_LOWPRIORITY;

// Set max delay time to 0.75 of samplerate.
#define MAX_DELAY static_cast<size_t>(48000 * 2.5f)

static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS dell;
static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delr;

MultiFX::MultiFX() {}
MultiFX::~MultiFX() {}

void MultiFX::Init()
{
    pitchShifterR.Init(patch.AudioSampleRate());
    pitchShifterL.Init(patch.AudioSampleRate());
    bitcrushL.Init(patch.AudioSampleRate());
    bitcrushR.Init(patch.AudioSampleRate());
    ef.Init();
    ef.SetAttackRelease(ENV_ATT, ENV_REL);

    // delay stuff
    dell.Init();
    delr.Init();
    delay_current = delay_target = patch.AudioSampleRate() * 0.75f;
    dell.SetDelay(delay_current);
    delr.SetDelay(delay_current);
}

void MultiFX::AttachReverb(ReverbSc *revb)
{ reverb = revb; }

void MultiFX::SetSubMode(int subMode)
{
    settings.effectMode = subMode <= NUM_FX_MODES ? subMode : 0;
    shouldSave          = true;
}

int MultiFX::GetSubMode()
{ return settings.effectMode; }

void MultiFX::OnSubModeButtonPress()
{
    // advance to next effect
    settings.effectMode = (settings.effectMode + 1) % NUM_FX_MODES;
    shouldSave          = true;
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

            reverb->SetFeedback(DSY_CLAMP(time, 0.001f, 0.99f));
            reverb->SetLpFreq(DSY_CLAMP(damp, 500.f, 22000.f));
            break;
        }
        case EffectMode::Delay:
        {
            delay_target   = fmap(param1,
                                  patch.AudioSampleRate() * 0.1f,
                                  MAX_DELAY,
                                  Mapping::LOG);
            delay_feedback = DSY_CLAMP(param2, 0.f, 1.f);
            break;
        }
        case EffectMode::PitchShift:
        {
            float shiftL = fmap(param1, -6.f, 26.f, Mapping::EXP);
            float shiftR = fmap(param2, -6.f, 26.f, Mapping::EXP);

            pitchShifterL.SetTransposition(shiftL);
            pitchShifterR.SetTransposition(shiftR);
            break;
        }
        case EffectMode::Crush:
        {
            float bitDepth  = fmap(param1, 0, 16.99f);
            float crushRate = fmap(param2, 100, 48000, Mapping::LOG);

            float weirdOne = 1;
            if(bitDepth > weirdOne && bitDepth <= weirdOne + 0.5f)
            {
                bitDepth = weirdOne - 1.f;
            }
            else if(bitDepth > weirdOne + 0.5f && bitDepth < weirdOne + 1.f)
            {
                bitDepth = weirdOne + 1.f;
            }
            bitcrushL.SetBitDepth(bitDepth);
            bitcrushL.SetCrushRate(crushRate);
            bitcrushR.SetBitDepth(bitDepth);
            bitcrushR.SetCrushRate(crushRate);
            break;
        }
    }

    // set CV to follow envelope of full-wet effect (of just L channel)
    CV_OUT_LOWPRIORITY
        = VoltageToCvValue(cheapTanh(ef.Value() * ENV_SCALE) * 5.f);
}

void MultiFX::GetDelaySample(float &outl, float &outr, float inl, float inr)
{
    fonepole(delay_current, delay_target, .00007f);
    delr.SetDelay(delay_current);
    dell.SetDelay(delay_current);
    outl = dell.Read();
    outr = delr.Read();

    dell.Write((delay_feedback * outl) + inl);
    outl = (delay_feedback * outl) + ((1.0f - delay_feedback) * inl);

    delr.Write((delay_feedback * outr) + inr);
    outr = (delay_feedback * outr) + ((1.0f - delay_feedback) * inr);
}

void MultiFX::AudioCallback(AudioHandle::InputBuffer  in,
                            AudioHandle::OutputBuffer out,
                            size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dryl  = IN_L[i] * dry_level;
        float dryr  = IN_R[i] * dry_level;
        float sendl = IN_L[i];
        float sendr = IN_R[i];
        float wetl = 0, wetr = 0;

        switch(settings.effectMode)
        {
            case EffectMode::Reverb:
            {
                // sends scaled by the send param
                sendl *= send_level;
                sendr *= send_level;
                reverb->Process(sendl, sendr, &wetl, &wetr);
                break;
            }
            case EffectMode::Delay:
            {
                // sends scaled by the send param
                sendl *= send_level;
                sendr *= send_level;
                GetDelaySample(wetl, wetr, sendl, sendr);
                break;
            }
            case EffectMode::PitchShift:
            {
                // sends 100%, but uses the send param to scale down the wet mix

                wetl = pitchShifterL.Process(sendl) * send_level;
                wetr = pitchShifterR.Process(sendr) * send_level;
                break;
            }
            case EffectMode::Crush:
            {
                // sends 100%, but uses the send param to scale down the wet mix

                wetl
                    = cheapTanh((float)(bitcrushL.Process(sendl))) * send_level;
                wetr
                    = cheapTanh((float)(bitcrushR.Process(sendr))) * send_level;
                break;
            }
        }
        OUT_L[i] = dryl + wetl;
        OUT_R[i] = dryr + wetr;
        // Env follower follows left wet output
        ef.Process(wetl);
    }
}