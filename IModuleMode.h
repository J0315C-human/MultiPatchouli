#pragma once
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "Utils.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

class IModuleMode
{
  public:
    virtual ~IModuleMode() = default;
    virtual void Init()    = 0;
    virtual void DacCallback(uint16_t **output, size_t size) {}
    virtual void AudioCallback(AudioHandle::InputBuffer  in,
                               AudioHandle::OutputBuffer out,
                               size_t                    size)
    {
        // passthru
        for(size_t i = 0; i < size; i++)
        {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
    }
};