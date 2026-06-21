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
    static constexpr int   NUM_CHORD_PAGES       = 4;
    static constexpr int   NOTE_DEBOUNCE_SAMPLES = 100;
    static constexpr float MAX_VOUT              = 5.f;
    int                    curChordPage;

    enum
    {
        KNOB_OFFSET = CV_1,
        KNOB_SCALEA,
        KNOB_ROOT_OFFSET,
        KNOB_SCALEB,
        CV_VOCT_IN,
        CV_SCALEA,
        CV_ROOT_OFFSET,
        CV_SCALEB
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
    bool  GetNewChordPageTrigger();

    GateTool newNoteGate;
    GateTool melodicDirectionGate;
    float    curCvOut;
    bool     noteGateOn;
    bool     chordPageGateOn;
    bool     melodyIsAscending;
    int      samplesSinceLastNote;
};