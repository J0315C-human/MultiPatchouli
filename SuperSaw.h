#pragma once
#include "IModuleMode.h"
#include "Utils.h"

class SuperSaw : public IModuleMode
{
  public:
    static constexpr int NUM_SUPERSAW_MODES = 4;

    SuperSaw();
    ~SuperSaw();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;
    void UpdateWaveForm();

    void OnSubmodeButtonPress() override;

  private:
    Oscillator osc_main, osc_a, osc_b, osc_c, osc_d, osc_e, osc_f, osc_g, osc_h;
    int        n_extra_voices;
    float      detune_incr;
    float      scaleFactor;
    float      loudnessFudge;
    float      mid_freq;
};