#pragma once

class GateTool
{
  private:
    int   samplesRemaining;
    bool  state;
    int   gateSamples;
    float sampleRate;

  public:
    GateTool()
    {
        samplesRemaining = 0;
        state            = false;
        sampleRate       = 48000.f;
        gateSamples      = 0;
    }

    void Init(float sampleRate, float gateLengthMs = 10.f)
    {
        this->sampleRate = sampleRate;
        SetGateLength(gateLengthMs);
    }

    void SetGateLength(float ms)
    {
        gateSamples = (ms / 1000.f) * sampleRate;
    }

    void Trigger()
    {
        samplesRemaining = gateSamples;
        state            = true;
    }

    void Process()
    {
        if(!state)
            return;

        if(--samplesRemaining <= 0)
            state = false;
    }

    bool State() { return state; }
};