#pragma once
#include "IModuleMode.h"
#include "EnvFollower.h"
#include "Utils.h"

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
    ReverbSc     reverb;
    _EnvFollower ef;
};