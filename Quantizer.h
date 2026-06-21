#pragma once
#include "IModuleMode.h"
#include "GateTool.h"

class Scale
{
  public:
    bool notes[12];

    Scale(std::initializer_list<int> intervals)
    {
        memset(notes, 0, sizeof(notes));
        int pos = 0;
        for(int interval : intervals)
        {
            pos        = (pos + interval) % 12;
            notes[pos] = true;
        }
    }

    Scale(const Scale &other, int transpose)
    {
        memset(notes, 0, sizeof(notes));
        for(int i = 0; i < 12; i++)
        {
            if(other.notes[i])
                notes[((i + transpose) % 12 + 12) % 12] = true;
        }
    }
};

class Quantizer : public IModuleMode
{
  public:
    static constexpr int   NOTE_DEBOUNCE_SAMPLES = 100;
    static constexpr float MAX_VOUT              = 5.f;

    enum
    {
        KNOB_OFFSET = CV_1,
        KNOB_SCALEA,
        KNOB_SCALEB,
        KNOB_ROOT_OFFSET,
        CV_VOCT_IN,
        CV_SCALEA,
        CV_SCALEB,
        CV_ROOT_OFFSET
    };
    Quantizer();
    ~Quantizer();

    void Init() override;
    void DacCallback(uint16_t **output, size_t size) override;
    void AudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size) override;

  private:
    float GetNearestNote(const Scale &scale, float note, int rootOffset);
    bool  GetNewNoteTrigger();

    GateTool newNoteGate;
    GateTool melodicDirectionGate;
    float    curCvOut;
    bool     gateOn;
    bool     melodyIsAscending;
    int      samplesSinceLastNote;
};