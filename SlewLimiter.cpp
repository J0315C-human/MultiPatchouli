#include "SlewLimiter.h"
#include "SettingsManager.h"
#include "Constants.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;
extern Settings     settings;
extern bool         shouldSave;

static constexpr float kSwing = 1.f; // tune to your CV voltage convention

SlewLimiter::SlewLimiter() {}
SlewLimiter::~SlewLimiter() {}

void SlewLimiter::Init()
{
    lastOutput        = 0.f;
    riseRatePerSample = 1.f;
    fallRatePerSample = 1.f;
    target            = 0.f;
}

void SlewLimiter::DacCallback(uint16_t **output, size_t size)
{
    bool fastRange = toggle.Pressed();

    target
        = GetCombinedKnobCv(CV_1, CV_5, 0.25, 1); // weigh the knob less
    float knob_rise = GetCombinedKnobCv(CV_2, CV_6);
    float knob_fall = GetCombinedKnobCv(CV_4, CV_8);

    float riseTime = fastRange ? fmap(knob_rise, 0.0001f, 0.2f, Mapping::EXP)
                               : fmap(knob_rise, 0.01f, 20.f, Mapping::EXP);

    float fallTime = fastRange ? fmap(knob_fall, 0.0001f, 0.2f, Mapping::EXP)
                               : fmap(knob_fall, 0.01f, 20.f, Mapping::EXP);

    float sr          = patch.AudioSampleRate();
    riseRatePerSample = (kSwing / (riseTime * sr));
    fallRatePerSample = (kSwing / (fallTime * sr));

    float outputVal = (lastOutput * MAX_VOUT);
    //  *CALIBRATE_VOCT;
    float cvout = VoltageToCvValue(outputVal);

    CV_OUT_LOWPRIORITY = LED_OUT_LOWPRIORITY = cvout;
}

void SlewLimiter::AudioCallback(AudioHandle::InputBuffer  in,
                                AudioHandle::OutputBuffer out,
                                size_t                    size)
{
    if(target > lastOutput)
    {
        lastOutput += riseRatePerSample * static_cast<float>(size);
        if(lastOutput > target)
            lastOutput = target;
    }
    else if(target < lastOutput)
    {
        lastOutput -= fallRatePerSample * static_cast<float>(size);
        if(lastOutput < target)
            lastOutput = target;
    }
}