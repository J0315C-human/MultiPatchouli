#pragma once
#include "IModuleMode.h"
#include "EnvFollower.h"
#include "Utils.h"

extern uint16_t CV_OUT_LOWPRIORITY;

class MiniEnvFollower : public IModuleMode
{
  public:
    MiniEnvFollower() {}
    ~MiniEnvFollower() {}

    void Init() override
    {
        // assumes patch has been Initialized
        ef.Init();
        ef.SetAttackRelease(ENV_ATT, ENV_REL);
    }

    void DacCallback(uint16_t **output, size_t size) override
    {
        // set CV to follow envelope of just L channel
        CV_OUT_LOWPRIORITY
            = VoltageToCvValue(cheapTanh(ef.Value() * ENV_SCALE) * 5.f);
    }

    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override
    {
        for(size_t i = 0; i < size; i++)
        {
            OUT_L[i] = IN_L[i];
            OUT_R[i] = IN_R[i];

            ef.Process(IN_L[i]);
        }
    }


  private:
    _EnvFollower ef;
};