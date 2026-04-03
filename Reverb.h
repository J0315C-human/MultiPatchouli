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

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    ReverbSc     reverb;
    _EnvFollower ef;
    float        dry_level;
    float        send_level;
};