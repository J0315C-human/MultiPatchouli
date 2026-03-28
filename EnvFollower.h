#pragma once
#include "IModuleMode.h"
#include "Blinker.h"


class _EnvFollower : public IModuleMode
{
  public:
    _EnvFollower();
    ~_EnvFollower();

    void  Init();
    void  Process(float input);
    void  SetAttackRelease(float attackCoef, float releaseCoef);
    float Value();

  private:
    float attackCoef;
    float releaseCoef;
    float envelope;
    float sRate;
};


class EnvFollower : public IModuleMode
{
  public:
    EnvFollower();
    ~EnvFollower();

    void Init() override;
    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    _EnvFollower ef;
};