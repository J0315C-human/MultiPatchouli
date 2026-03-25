#pragma once
#include "IModuleMode.h"

class Reverb : public IModuleMode
{
  public:
    Reverb();
    ~Reverb();

    void Init() override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    ReverbSc reverb;
};