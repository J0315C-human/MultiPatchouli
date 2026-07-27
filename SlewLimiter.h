#pragma once
#include "IModuleMode.h"
#include "Utils.h"

class SlewLimiter : public IModuleMode
{
  public:
    static constexpr float MAX_VOUT = 5.f;

    SlewLimiter();
    ~SlewLimiter();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    float riseRatePerSample;
    float fallRatePerSample;
    float lastOutput;
    float target;
};