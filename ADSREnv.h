#pragma once
#include "IModuleMode.h"

class _ADSREnv
{
  public:
    _ADSREnv();
    ~_ADSREnv();

    void Init(float sampleRate);
    void Process();
    void SetADSR(float attackMs, float decayMs, float sustain, float releaseMs);
    void Gate(bool high);
    float Value();

  private:
    enum class Stage
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    float sRate;
    float attackCoef;
    float decayCoef;
    float sustainLevel;
    float releaseCoef;
    float envelope;
    bool  gate;
    Stage stage;
};


class ADSREnv : public IModuleMode
{
  public:
    ADSREnv();
    ~ADSREnv();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    _ADSREnv adsr;
    float    scaleMult;
};