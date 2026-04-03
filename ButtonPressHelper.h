#pragma once
#include "daisy_patch_sm.h"

using namespace daisy;

extern Switch button7;

class ButtonPressHelper
{
  public:
    enum PressType
    {
        SHORT_PRESS,
        LONG_PRESS,
        EXTRA_LONG_PRESS
    };
    ButtonPressHelper() {}
    ~ButtonPressHelper() {}

    void Init(PressType type)
    {
        samplesSincePressed = 0;

        // MAGIC NUMBERS
        switch(type)
        {
            case SHORT_PRESS:
                minSamplesToTrigger = 0;
                maxSamplesToTrigger = 999;
                break;
            case LONG_PRESS:
                minSamplesToTrigger = 1000;
                maxSamplesToTrigger = 4999;
                break;
            case EXTRA_LONG_PRESS:
                minSamplesToTrigger = 5000;
                maxSamplesToTrigger = 100000000;
                break;
        }
        buttonWasDown = false;
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

            // trigger on falling edge if within time window
            if(samplesSincePressed >= minSamplesToTrigger
               && samplesSincePressed <= maxSamplesToTrigger)
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
    int  minSamplesToTrigger;
    int  maxSamplesToTrigger;
    bool buttonWasDown;
};