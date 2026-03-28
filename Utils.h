#pragma once
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

extern DaisyPatchSM    patch;

/* this was found with trial-and-error with my particular unit. */
static constexpr float CALIBRATE_VOCT = 0.9790673f;

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

/* For smooth limiting to (-1, 1)*/
inline float cheapTanh(float x)
{
    x = DSY_CLAMP(x, -3.f, 3.f);
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}