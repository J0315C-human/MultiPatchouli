#include "FX.h"
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

FX::FX() {}
FX::~FX() {}

void FX::Init()
{
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
void FX::SetEffectChainMode(bool onOrOff)
{ dualModeOn = onOrOff; }

void FX::AttachEffectProcessors(ReverbSc     *revb,
                                PitchShifter *pitchL,
                                PitchShifter *pitchR)
{
    reverb        = revb;
    pitchShifterR = pitchR;
    pitchShifterL = pitchL;
}

void FX::SetSubMode(int subMode)
{
    if(dualModeOn)
    {
        settings.dualEffectMode = subMode <= NUM_DUAL_MODES ? subMode : 0;
    }
    else
    {
        settings.effectMode = subMode <= NUM_FX_MODES ? subMode : 0;
    }
    shouldSave = true;
}

int FX::GetSubMode()
{ return dualModeOn ? settings.dualEffectMode : settings.effectMode; }

void FX::OnSubModeButtonPress()
{
    if(dualModeOn)
    {
        // advance to next chain
        settings.dualEffectMode
            = (settings.dualEffectMode + 1) % NUM_DUAL_MODES;
    }
    else
    {
        // advance to next effect
        settings.effectMode = (settings.effectMode + 1) % NUM_FX_MODES;
    }
    shouldSave = true;
}

void FX::DacCallback(uint16_t **output, size_t size)
{
    float param1 = GetCombinedKnobCv(CV_1, CV_5);
    float param2 = GetCombinedKnobCv(CV_2, CV_6);

    float param3 = GetCombinedKnobCv(CV_3, CV_7);
    float param4 = GetCombinedKnobCv(CV_4, CV_8);

    if(dualModeOn)
    {
        // param 3 is effect balance (0: all first effect, >=1: all second effect)
        dualEffectBalance = DSY_CLAMP(param3, 0, 1);
        // param 4 is dry/wet balance
        send_level = DSY_CLAMP(param4, 0, 1.5f);
        dry_level  = 1.f - DSY_CLAMP(send_level, 0, 1);
    }
    else
    {
        // param 3 is dry level, param 4 is send/wet level
        dry_level  = DSY_CLAMP(param3, 0, 1.5f);
        send_level = DSY_CLAMP(param4, 0, 1.5f);
    }

    // set reverb parameters
    float time = fmap(param1, 0.45f, 0.99f);
    float damp = fmap(param2, 1000.f, 19000.f, Mapping::LOG);

    reverb->SetFeedback(DSY_CLAMP(time, 0.001f, 0.99f));
    reverb->SetLpFreq(DSY_CLAMP(damp, 500.f, 22000.f));

    // delay
    delay_target
        = fmap(param1, patch.AudioSampleRate() * 0.1f, MAX_DELAY, Mapping::LOG);
    delay_feedback = DSY_CLAMP(param2, 0.f, 1.f);

    // pitch shift
    float shiftL = fmap(param1, -6.f, 26.f, Mapping::EXP);
    float shiftR = fmap(param2, -6.f, 26.f, Mapping::EXP);

    pitchShifterL->SetTransposition(shiftL);
    pitchShifterR->SetTransposition(shiftR);

    // bit crush
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

    // set CV to follow envelope of full-wet effect (of just L channel)
    CV_OUT_LOWPRIORITY
        = VoltageToCvValue(cheapTanh(ef.Value() * ENV_SCALE) * 5.f);
}

void FX::GetDelaySample(float &outl, float &outr, float inl, float inr)
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

void FX::AudioCallback(AudioHandle::InputBuffer  in,
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

        if(dualModeOn)
        {
            // both effect levels are proportional to the main send level, the last scalar is fudge
            float lvl_B = dualEffectBalance * send_level * 1.3f;
            float lvl_A = (1.f - dualEffectBalance) * send_level * 1.3f;

            // PROCESS EFFECT A
            switch(settings.dualEffectMode)
            {
                case DualEffectMode::PitchShiftReverb:
                case DualEffectMode::PitchShiftDelay:
                {
                    wetl = pitchShifterL->Process(sendl) * lvl_A;
                    wetr = pitchShifterR->Process(sendr) * lvl_A;
                    break;
                }
                case DualEffectMode::DelayReverb:
                {
                    GetDelaySample(wetl, wetr, sendl * lvl_A, sendr * lvl_A);
                    break;
                }
            }
            // effectA output is effectB input
            dryl += wetl;
            dryr += wetr;
            sendl = dryl;
            sendr = dryr;

            // PROCESS EFFECT B
            switch(settings.dualEffectMode)
            {
                case DualEffectMode::PitchShiftDelay:
                {
                    GetDelaySample(wetl, wetr, sendl * lvl_B, sendr * lvl_B);
                    break;
                }
                case DualEffectMode::PitchShiftReverb:
                case DualEffectMode::DelayReverb:
                {
                    reverb->Process(sendl * lvl_B, sendr * lvl_B, &wetl, &wetr);
                    break;
                }
            }
        }
        else
        {
            switch(settings.effectMode)
            {
                case EffectMode::Reverb:
                {
                    reverb->Process(
                        sendl * send_level, sendr * send_level, &wetl, &wetr);
                    break;
                }
                case EffectMode::Delay:
                {
                    GetDelaySample(
                        wetl, wetr, sendl * send_level, sendr * send_level);
                    break;
                }
                case EffectMode::PitchShift:
                {
                    wetl = pitchShifterL->Process(sendl) * send_level;
                    wetr = pitchShifterR->Process(sendr) * send_level;
                    break;
                }
                case EffectMode::Crush:
                {
                    wetl = cheapTanh((float)(bitcrushL.Process(sendl)))
                           * send_level;
                    wetr = cheapTanh((float)(bitcrushR.Process(sendr)))
                           * send_level;
                    break;
                }
            }
        }
        OUT_L[i] = dryl + wetl;
        OUT_R[i] = dryr + wetr;
        // Env follower follows left wet output
        ef.Process(wetl);
    }
}