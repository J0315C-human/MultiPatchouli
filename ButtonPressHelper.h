#pragma once
#include "daisy_patch_sm.h"

using namespace daisy;

extern Switch button7;

class ButtonPressHelper
{
  public:
    ButtonPressHelper() {}
    ~ButtonPressHelper() {}

    void Init(int triggerLength)
    {
        samplesSincePressed = 0;
        samplesToTrigger    = triggerLength;
        buttonWasDown       = false;
    }

    bool ProcessAndCheckTrigger()
    {
        if(button7.Pressed())
        {
            if(!buttonWasDown) // rising edge — reset counter cleanly
            {
                samplesSincePressed = 0;
            }

            samplesSincePressed++;
            buttonWasDown = true;
            return false;
        }
        else
        {
            if(!buttonWasDown) // nothing has changed
            {
                return false;
            }
            buttonWasDown = false;

            // trigger on falling edge if enough time has passed
            if(samplesSincePressed >= samplesToTrigger)
            {
                samplesSincePressed = 0;
                return true;
            }
            else
            {
                samplesSincePressed = 0;
                return false;
            }
        }
    }


  private:
    int  samplesSincePressed;
    int  samplesToTrigger;
    bool buttonWasDown;
};