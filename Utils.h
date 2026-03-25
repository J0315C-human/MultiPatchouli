#pragma once
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

extern DaisyPatchSM patch;
// extern Switch       toggle;
// extern uint16_t     LED_OUT_LOWPRIORITY;
// extern uint16_t     CV_OUT_LOWPRIORITY;

inline float GetCombinedKnobCv(int knob, int cv)
{
    float vK = patch.GetAdcValue(knob);
    float vC = patch.GetAdcValue(cv);
    return vK + (vC / 2);
}

inline uint16_t VoltageToCvValue(float input)
{
    float pre = input * 819.f;
    if(pre > 4095.f)
        pre = 4095.f;
    else if(pre < 0.f)
        pre = 0.f;
    return (uint16_t)pre;
}