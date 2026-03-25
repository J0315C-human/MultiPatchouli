#pragma once

class Blinker
{
  private:
    enum Phase
    {
        IDLE,
        MARGIN_IN,
        BLINKING,
        MARGIN_OUT
    };

    int   remainingBlinks;
    int   samplesUntilNext;
    bool  state;
    int   onOffSamples;
    int   marginSamples;
    float sampleRate;
    Phase phase;

  public:
    Blinker()
    {
        remainingBlinks  = 0;
        samplesUntilNext = 0;
        state            = false;
        sampleRate       = 48000.f;
        phase            = IDLE;
    }

    void Init(float sampleRate)
    {
        this->sampleRate = sampleRate;
        onOffSamples     = (100.f / 1000.f) * sampleRate;
        marginSamples    = (300.f / 1000.f) * sampleRate;
    }

    void Trigger(int count)
    {
        remainingBlinks  = count * 2; // each blink = 1 on + 1 off
        samplesUntilNext = marginSamples;
        state            = false;
        phase            = MARGIN_IN;
    }

    void Process()
    {
        if(phase == IDLE)
            return;

        samplesUntilNext--;

        if(samplesUntilNext > 0)
            return;

        switch(phase)
        {
            case MARGIN_IN:
                phase            = BLINKING;
                state            = true;
                samplesUntilNext = onOffSamples;
                break;

            case BLINKING:
                state = !state;
                remainingBlinks--;

                if(remainingBlinks <= 0)
                {
                    state            = false;
                    phase            = MARGIN_OUT;
                    samplesUntilNext = marginSamples;
                }
                else
                {
                    samplesUntilNext = onOffSamples;
                }
                break;

            case MARGIN_OUT:
                phase = IDLE;
                state = false;
                break;

            case IDLE: break;
        }
    }

    bool State() { return state; }
    bool IsActive() { return phase != IDLE; }
};