#include "Quantizer.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;

static const Scale SCALE_CHROM = Scale({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
static const Scale SCALE_MAJOR = Scale({2, 2, 1, 2, 2, 2, 1});
static const Scale SCALE_MINOR = Scale({2, 1, 2, 2, 1, 2, 2});
static const Scale SCALE_HARMONIC_MINOR = Scale({2, 1, 2, 2, 1, 3, 1});
static const Scale SCALE_PENTATONIC     = Scale({2, 3, 2, 2, 3});
static const Scale SCALE_BLUES          = Scale({2, 3, 2, 1, 1, 3});
static const Scale SCALE_WHOLETONE      = Scale({2, 2, 2, 2, 2, 2});
static const Scale SCALE_DIMINISHED     = Scale({2, 1, 2, 1, 2, 1, 2, 1});
static const Scale CHORD_MAJOR          = Scale({4, 3, 5});
static const Scale CHORD_MAJOR7         = Scale({4, 3, 4});
static const Scale CHORD_DOMINANT7      = Scale({4, 3, 3});
static const Scale CHORD_MINOR          = Scale({3, 4, 5});
static const Scale CHORD_MINOR7         = Scale({3, 4, 3});
static const Scale CHORD_DIM7           = Scale({3, 3, 3});
static const Scale CHORD_AUG            = Scale({4, 4, 4});

static const Scale* SCALES[] = {&SCALE_CHROM,
                                &SCALE_MAJOR,
                                &SCALE_MINOR,
                                &SCALE_HARMONIC_MINOR,
                                &SCALE_PENTATONIC,
                                &SCALE_BLUES,
                                &SCALE_WHOLETONE,
                                &SCALE_DIMINISHED,
                                &CHORD_MAJOR,
                                &CHORD_MAJOR7,
                                &CHORD_DOMINANT7,
                                &CHORD_MINOR,
                                &CHORD_MINOR7,
                                &CHORD_DIM7,
                                &CHORD_AUG};

static const int NUM_SCALES = sizeof(SCALES) / sizeof(SCALES[0]);

Quantizer::Quantizer() {}
Quantizer::~Quantizer() {}

void Quantizer::Init() {}

float Quantizer::GetNearestNote(const Scale& scale, float note)
{
    // find nearest allowed semitone in scale
    float outputSemitone = 0.f;
    float delta          = 99999.f;

    for(int s = 0; s <= 60; s++)
    {
        if(!scale.notes[s % 12])
            continue;

        float thisDelta = fabsf(note - s);
        if(thisDelta < delta)
        {
            outputSemitone = s;
            delta          = thisDelta;
        }
        else
        {
            break;
        }
    }
    return outputSemitone;
}

void Quantizer::DacCallback(uint16_t** output, size_t size)
{
    float offsetIn   = patch.GetAdcValue(KNOB_OFFSET);
    float scaleInput = patch.GetAdcValue(KNOB_SCALE);
    float inputIn    = patch.GetAdcValue(CV_VOCT_IN);

    // select scale from knob
    int scaleIndex     = (int)fmap(scaleInput, 0.f, (float)NUM_SCALES - 0.01f);
    const Scale& scale = *SCALES[scaleIndex];

    // combine offset and input, clamp to 0-1
    float inputVal = DSY_CLAMP(offsetIn + inputIn, 0.f, 1.f);

    float outNote
        = GetNearestNote(scale, inputVal * 60.f); // (5 octaves = 60 semitones)

    // convert back to voltage and output
    float outputVal    = (outNote / 60.f) * MAX_VOUT;
    CV_OUT_LOWPRIORITY = VoltageToCvValue(outputVal);
}