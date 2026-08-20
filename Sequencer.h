#pragma once
#include "IModuleMode.h"

class Sequencer : public IModuleMode
{
  public:
    static constexpr float MAX_VOUT = 5.f;

    enum
    {
        KNOB_V1 = CV_1,
        KNOB_V2,
        KNOB_V3,
        KNOB_V4,
        CV_V1,
        CV_V2,
        CV_V3,
        CV_V4
    };

    Sequencer();
    ~Sequencer();

    void Init() override;
    void DacCallback(uint16_t **output, size_t size) override;
    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    int  curValueIdx = 0;
    bool advanceGateOn;
    bool backwardGateOn;
};