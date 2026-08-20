#pragma once
#include "IModuleMode.h"
#include "Utils.h"

class MiniReverb : public IModuleMode
{
  public:
    static constexpr float DRY_LEVEL_L  = 0.65f;
    static constexpr float DRY_LEVEL_R  = 0;
    static constexpr float SEND_LEVEL_L = 0.45f;
    static constexpr float SEND_LEVEL_R = 0.999f;

    MiniReverb() {}
    ~MiniReverb() {}

    void Init() override {}

    void AttachReverb(ReverbSc *revb) { reverb = revb; }

    void DacCallback(uint16_t **output, size_t size) override
    {
        reverb->SetFeedback(0.83f);
        reverb->SetLpFreq(8000.f);
    }

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override
    {
        for(size_t i = 0; i < size; i++)
        {
            float wetl = 0, wetr = 0;

            reverb->Process(
                IN_L[i] * SEND_LEVEL_L, IN_R[i] * SEND_LEVEL_R, &wetl, &wetr);

            OUT_L[i] = IN_L[i] * DRY_LEVEL_L + wetl;
            OUT_R[i] = IN_R[i] * DRY_LEVEL_R + wetr;
        }
    }

  private:
    ReverbSc *reverb;
};