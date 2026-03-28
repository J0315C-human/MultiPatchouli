#pragma once
#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

extern DaisyPatchSM patch;
extern Switch       button7;
extern uint16_t     LED_OUT_LOWPRIORITY;

class ModeSelector
{
  public:
    ModeSelector()
    {
        currentMode     = 0;
        firstKnobMoved  = -1;
        secondKnobMoved = -1;
        wasPressed      = false;

        for(int i = 0; i < 4; i++)
        {
            prevKnobValues[i]   = 0.f;
            cumulativeMotion[i] = 0.f;
            knobSettled[i]      = true;
        }
    }

    void ProcessDac()
    {
        bool pressed = button7.Pressed();

        // button7 just released
        if(wasPressed && !pressed)
        {
            if(firstKnobMoved != -1 && secondKnobMoved != -1)
                currentMode = firstKnobMoved * 4 + secondKnobMoved;

            firstKnobMoved  = -1;
            secondKnobMoved = -1;
            for(int i = 0; i < 4; i++)
            {
                cumulativeMotion[i] = 0.f;
                knobSettled[i]      = true;
            }
            LED_OUT_LOWPRIORITY = 0;
            wasPressed          = false;
            return;
        }

        wasPressed = pressed;

        if(!pressed)
            return;

        for(int i = 0; i < 4; i++)
        {
            float val         = patch.GetAdcValue(CV_1 + i);
            float diff        = fabsf(val - prevKnobValues[i]);
            prevKnobValues[i] = val;

            if(diff > NOISE_FLOOR)
            {
                cumulativeMotion[i] += diff;
                knobSettled[i] = false;

                if(cumulativeMotion[i] > MOVE_THRESHOLD)
                {
                    cumulativeMotion[i] = 0.f;
                    firstKnobMoved      = secondKnobMoved;
                    secondKnobMoved     = i;
                }
            }
            else
            {
                if(!knobSettled[i])
                {
                    knobSettled[i]      = true;
                    cumulativeMotion[i] = 0.f;
                }
            }
        }
        // LED reflects state after scanning
        if(firstKnobMoved == -1 && secondKnobMoved != -1)
            LED_OUT_LOWPRIORITY = 4095.0;
        else
            LED_OUT_LOWPRIORITY = 0;
    }

    int Mode() { return currentMode; }

  private:
    static constexpr float NOISE_FLOOR    = 0.001f;
    static constexpr float MOVE_THRESHOLD = 0.15f;

    int   currentMode;
    int   firstKnobMoved;
    int   secondKnobMoved;
    bool  wasPressed;
    float prevKnobValues[4];
    float cumulativeMotion[4];
    bool  knobSettled[4];
};