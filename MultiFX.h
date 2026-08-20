#pragma once
#include "IModuleMode.h"
#include "EnvFollower.h"
#include "Utils.h"

class MultiFX : public IModuleMode
{
  public:
    static constexpr int NUM_FX_MODES = 4;

    MultiFX();
    ~MultiFX();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;
    void GetDelaySample(float &outl, float &outr, float inl, float inr);

    void OnSubModeButtonPress() override;
    void SetSubMode(int subMode) override;
    int  GetSubMode() override;

    void AttachReverb(ReverbSc *revb);

  private:
    enum EffectMode
    {
        Reverb,
        Delay,
        PitchShift,
        Crush
    };
    ReverbSc    *reverb;
    PitchShifter pitchShifterL;
    PitchShifter pitchShifterR;
    Bitcrush     bitcrushL;
    Bitcrush     bitcrushR;
    _EnvFollower ef;
    float        dry_level;
    float        send_level;
    float        delay_current;
    float        delay_target;
    float        delay_feedback;
};