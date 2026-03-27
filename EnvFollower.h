#pragma once
#include "IModuleMode.h"
#include "Blinker.h"


class EnvFollower : public IModuleMode
{
  public:
    EnvFollower();
    ~EnvFollower();

    void Init() override;
    void Process(float input);
    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    float attackCoef;
    float releaseCoef;
    float envelope;
    float sRate;
};