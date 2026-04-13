#pragma once
#include "IModuleMode.h"
#include "EnvFollower.h"
#include "Utils.h"

class MultiFX : public IModuleMode
{
  public:
    static constexpr int NUM_FX_MODES = 2;

    MultiFX();
    ~MultiFX();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    enum EffectMode
    {
        Reverb,
        PitchShift
    };
    ReverbSc     reverb;
    PitchShifter pitchShifterL;
    PitchShifter pitchShifterR;
    _EnvFollower ef;
    float        dry_level;
    float        send_level;
};