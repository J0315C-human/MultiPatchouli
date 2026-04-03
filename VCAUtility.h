#pragma once
#include "IModuleMode.h"

class VCAUtility : public IModuleMode
{
  public:
    VCAUtility();
    ~VCAUtility();

    void Init() override;

    void DacCallback(uint16_t **output, size_t size) override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    float scaleL;
    float scaleR;
};