#include "Quantizer.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;

static const Scale SC_CHROM = Scale({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
static const Scale SC_MAJOR = Scale({2, 2, 1, 2, 2, 2, 1});
static const Scale SC_MINOR = Scale({2, 1, 2, 2, 1, 2, 2});
static const Scale SC_HARMONIC_MINOR = Scale({2, 1, 2, 2, 1, 3, 1});
static const Scale SC_PENTMIN        = Scale({3, 2, 2, 3});
static const Scale SC_PENTMAJ        = Scale({2, 2, 3, 2});
static const Scale SC_BLUES          = Scale({3, 2, 1, 1, 3});
static const Scale SC_WHOLETONE      = Scale({2, 2, 2, 2, 2});
static const Scale SC_DIMINISHED     = Scale({2, 1, 2, 1, 2, 1, 2});
static const Scale CH_SUS            = Scale({5, 2});
static const Scale CH_MAJOR          = Scale({4, 3});
static const Scale CH_MAJOR7         = Scale({4, 3, 4});
static const Scale CH_DOMINANT7      = Scale({4, 3, 3});
static const Scale CH_MINOR          = Scale({3, 4});
static const Scale CH_MINOR7         = Scale({3, 4, 3});
static const Scale CH_DIM7           = Scale({3, 3, 3});
static const Scale CH_AUG            = Scale({4, 4, 4});
static const Scale CH_HD7            = Scale({3, 3, 4});
static const Scale CH_5TH            = Scale({7});
static const Scale OCTAVE            = Scale({});

static const Scale* SCALES[] = {
    &OCTAVE,       &CH_5TH,   &CH_MAJOR,     &CH_MAJOR7,         &CH_AUG,
    &CH_DOMINANT7, &CH_SUS,   &CH_MINOR,     &CH_MINOR7,         &CH_HD7,
    &CH_DIM7,      &SC_MAJOR, &SC_PENTMAJ,   &SC_HARMONIC_MINOR, &SC_MINOR,
    &SC_PENTMIN,   &SC_BLUES, &SC_WHOLETONE, &SC_DIMINISHED,     &SC_CHROM,
};

static const int NUM_SCALES = sizeof(SCALES) / sizeof(SCALES[0]);

Quantizer::Quantizer() {}
Quantizer::~Quantizer() {}

void Quantizer::Init() {}

float Quantizer::GetNearestNote(const Scale& scale, float note, int rootOffset)
{
    // find nearest allowed semitone in scale
    float outputSemitone = 0.f;
    float delta          = 99999.f;

    for(int s = 0; s <= 60; s++)
    {
        if(!scale.notes[(s - rootOffset + 12) % 12])
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
    float voctIn       = GetCombinedKnobCv(KNOB_OFFSET, CV_VOCT_IN);
    float scaleInput   = GetCombinedKnobCv(KNOB_SCALE, CV_SCALE);
    float rootOffsetIn = GetCombinedKnobCv(KNOB_ROOT_OFFSET, CV_ROOT_OFFSET);

    // select scale from knob
    int scaleIndex = (int)fmap(scaleInput, 0.f, (float)NUM_SCALES - 0.01f);
    int rootOffset = (int)fmap(rootOffsetIn, -6.f, 6.f);

    const Scale& scale = *SCALES[scaleIndex];

    // combine offset and input, clamp to 0-1 (5 octaves = 60 semitones)
    float note    = (DSY_CLAMP(voctIn, 0.f, 1.f)) * 60.f;
    float outNote = GetNearestNote(scale, note, rootOffset);

    // convert back to voltage and output
    float outputVal     = ((outNote / 60.f) * MAX_VOUT) / CALIBRATE_VOCT;
    float cvout         = VoltageToCvValue(outputVal);
    CV_OUT_LOWPRIORITY  = cvout;
    LED_OUT_LOWPRIORITY = cvout;
}