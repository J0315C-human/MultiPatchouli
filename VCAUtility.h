#pragma once
#include "IModuleMode.h"

class VCAUtility : public IModuleMode
{
  public:
    VCAUtility();
    ~VCAUtility();

    void Init() override;

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
};