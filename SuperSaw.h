#pragma once
#include "IModuleMode.h"


class SuperSaw : public IModuleMode
{
  public:
    SuperSaw();
    ~SuperSaw();

    void Init() override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    Oscillator osc_main, osc_a, osc_b, osc_c, osc_d, osc_e, osc_f, osc_g, osc_h, osc_i,
        osc_j, osc_k, osc_l;
    int waveForm;
};