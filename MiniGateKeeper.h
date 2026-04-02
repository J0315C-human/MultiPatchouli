#pragma once
#include "IModuleMode.h"

extern DaisyPatchSM patch;

class MiniGateKeeper : public IModuleMode
{
  public:
    MiniGateKeeper() {}
    ~MiniGateKeeper() {}

    void Init() override { gateInOpen = false; }

    void DacCallback(uint16_t **output, size_t size) override
    {
        bool newGateInOpen = GetCurrentGateInputState();

        if(newGateInOpen == gateInOpen)
            return;

        if(newGateInOpen)
        {
            bool tr = patch.GetRandomFloat(0.f, 0.5f)
                      <= 0.6667f; // trigger 2/3 of the time
            dsy_gpio_write(&patch.gate_out_1, tr);
            dsy_gpio_write(&patch.gate_out_2, !tr);
        }
        else
        {
            dsy_gpio_write(&patch.gate_out_1, false);
            dsy_gpio_write(&patch.gate_out_2, false);
        }

        gateInOpen = newGateInOpen;
    }


  private:
    bool gateInOpen;

    bool GetCurrentGateInputState()
    { return patch.gate_in_1.State() || patch.gate_in_2.State(); }
};