#pragma once

extern uint16_t LED_OUT_LOWPRIORITY;

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
    bool  isFirstBlink;
    int   onSamples;
    int   offSamples;
    int   marginInSamples;
    int   marginOutSamples;
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
        isFirstBlink     = false;
    }

    void Init(float sampleRate)
    {
        this->sampleRate = sampleRate;
        onSamples        = (100.f / 1000.f) * sampleRate;
        offSamples       = (200.f / 1000.f) * sampleRate;
        marginInSamples  = (250.f / 1000.f) * sampleRate;
        marginOutSamples = (400.f / 1000.f) * sampleRate;
    }

    void Trigger(int count)
    {
        remainingBlinks  = count * 2; // each blink = 1 on + 1 off
        samplesUntilNext = isFirstBlink ? marginInSamples * 5
                                        : marginInSamples; // delay first blink
        state            = false;
        phase            = MARGIN_IN;
        isFirstBlink     = false;
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
                samplesUntilNext = onSamples;
                break;

            case BLINKING:
                state = !state;
                remainingBlinks--;

                if(remainingBlinks <= 0)
                {
                    state            = false;
                    phase            = MARGIN_OUT;
                    samplesUntilNext = marginOutSamples;
                }
                else
                {
                    samplesUntilNext = state ? onSamples : offSamples;
                }
                break;

            case MARGIN_OUT:
                // turn off "low Priority" led in case it's on from prev modes
                LED_OUT_LOWPRIORITY = 0.f;
                phase               = IDLE;
                state               = false;
                break;

            case IDLE: break;
        }
    }

    bool State() { return state; }
    bool IsActive() { return phase != IDLE; }
};