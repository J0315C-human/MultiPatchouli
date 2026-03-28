#pragma once
#include "IModuleMode.h"
#include "EnvFollower.h"
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
    static constexpr float ENV_ATT   = 60.f;
    static constexpr float ENV_REL   = 350.f;
    static constexpr float ENV_SCALE = 2.f;

    ReverbSc     reverb;
    _EnvFollower ef;
};