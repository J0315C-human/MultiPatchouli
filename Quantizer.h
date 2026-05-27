#pragma once
#include "IModuleMode.h"
#include "Blinker.h"

class Scale
{
  public:
    bool notes[12];

    Scale(std::initializer_list<int> intervals)
    {
        memset(notes, 0, sizeof(notes));
        int pos  = 0;
        notes[0] = true; // root is always active
        for(int interval : intervals)
        {
            pos        = (pos + interval) % 12;
            notes[pos] = true;
        }
    }
};

class Quantizer : public IModuleMode
{
  public:
    static constexpr float MAX_VOUT = 5.f;

    enum
    {
        KNOB_OFFSET = CV_1,
        KNOB_SCALE,
        KNOB_ROOT_OFFSET,
        KNOB_4,
        CV_VOCT_IN,
        CV_SCALE,
        CV_ROOT_OFFSET,
        CV_4
    };
    Quantizer();
    ~Quantizer();

    void Init() override;
    void DacCallback(uint16_t **output, size_t size) override;

  private:
    float GetNearestNote(const Scale &scale, float note, int rootOffset);
};