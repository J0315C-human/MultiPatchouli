#pragma once
#include "IModuleMode.h"
#include "Blinker.h"


class GateKeeper : public IModuleMode
{
  public:
    enum
    {
        KNOB_RANDOMNESS = CV_1,
        KNOB_RANDOM_DENSITY,
        KNOB_CYCLE_LENGTH,
        KNOB_CYCLE_DUTY,
        CV_RANDOMNESS,
        CV_RAND_DENSITY,
        CV_CYCLE_LENGTH,
        CV_CYCLE_DUTY
    };

    GateKeeper();
    ~GateKeeper();

    void Init() override;
    void DacCallback(uint16_t **output, size_t size) override;

  private:
    int  triggerInCount;
    bool gateInOpen;
    bool lightState;

    bool  GetRandomChanceByCV(int knob, int cv);
    bool  ShouldTrigger_Cycled();
    bool  GetCurrentGateInputState();
};