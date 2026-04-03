#pragma once
#include "IModuleMode.h"
#include "ButtonPressHelper.h"
#include "Utils.h"

class SuperSaw : public IModuleMode
{
  public:
    SuperSaw();
    ~SuperSaw();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;
    void SwitchWaveForm();

  private:
    Oscillator osc_main, osc_a, osc_b, osc_c, osc_d, osc_e, osc_f, osc_g, osc_h;
    ButtonPressHelper btn;
    int               waveForm;
    int               n_extra_voices;
    float             detune_incr;
    float             scaleFactor;
    float             loudnessFudge;
    float             mid_freq;
};