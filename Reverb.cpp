#include "Reverb.h"
#include "daisysp-lgpl.h"

extern DaisyPatchSM patch;
extern uint16_t     CV_OUT_LOWPRIORITY;

Reverb::Reverb() {}
Reverb::~Reverb() {}

void Reverb::Init()
{
    reverb.Init(patch.AudioSampleRate());
    ef.Init();
    ef.SetAttackRelease(ENV_ATT, ENV_REL);
}

void Reverb::AudioCallback(AudioHandle::InputBuffer  in,
                           AudioHandle::OutputBuffer out,
                           size_t                    size)
{
    /** Update Params with the four knobs */
    float time_knob = patch.GetAdcValue(CV_1);
    float time      = fmap(time_knob, 0.3f, 0.99f);

    float damp_knob = patch.GetAdcValue(CV_2);
    float damp      = fmap(damp_knob, 1000.f, 19000.f, Mapping::LOG);

    float dry_level  = patch.GetAdcValue(CV_3);
    float send_level = patch.GetAdcValue(CV_4);

    reverb.SetFeedback(time);
    reverb.SetLpFreq(damp);

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

    // set CV to follow envelope of full-wet reverb (of just L channel)
    CV_OUT_LOWPRIORITY
        = VoltageToCvValue(cheapTanh(ef.Value() * ENV_SCALE) * 5.f);
}