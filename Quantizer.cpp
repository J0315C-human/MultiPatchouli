#include "Quantizer.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;

static const Scale SC_CHROM = Scale({0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
static const Scale SC_MAJOR = Scale({0, 2, 2, 1, 2, 2, 2, 1});
static const Scale SC_MINOR = Scale({0, 2, 1, 2, 2, 1, 2, 2});
static const Scale SC_HARMONIC_MINOR = Scale({0, 2, 1, 2, 2, 1, 3, 1});
static const Scale SC_PENTMIN        = Scale({0, 3, 2, 2, 3});
static const Scale SC_PENTMAJ        = Scale({0, 2, 2, 3, 2});
static const Scale SC_BLUES          = Scale({0, 3, 2, 1, 1, 3});
static const Scale SC_WHOLETONE      = Scale({0, 2, 2, 2, 2, 2});
static const Scale SC_DIMINISHED     = Scale({0, 2, 1, 2, 1, 2, 1, 2});
static const Scale CH_SUS            = Scale({0, 5, 2});
static const Scale CH_MAJOR          = Scale({0, 4, 3});
static const Scale CH_MAJOR7         = Scale({0, 4, 3, 4});
static const Scale CH_DOMINANT7      = Scale({0, 4, 3, 3});
static const Scale CH_MINOR          = Scale({0, 3, 4});
static const Scale CH_MINOR7         = Scale({0, 3, 4, 3});
static const Scale CH_MINMAJ7         = Scale({0, 3, 4, 4});
static const Scale CH_DIM7           = Scale({0, 3, 3, 3});
static const Scale CH_AUG            = Scale({0, 4, 4, 4});
static const Scale CH_HD7            = Scale({0, 3, 3, 4});
static const Scale CH_5TH            = Scale({0, 7});
static const Scale OCTAVE            = Scale({0});


static const Scale C_MAJOR     = Scale(CH_MAJOR, 0);
static const Scale D_MINOR7    = Scale(CH_MINOR7, 2);
static const Scale D_MAJOR     = Scale(CH_MAJOR, 2);
static const Scale E_MINOR7    = Scale(CH_MINOR7, 4);
static const Scale E_DOMINANT7 = Scale(CH_DOMINANT7, 4);
static const Scale F_MAJOR7    = Scale(CH_MAJOR7, 5);
static const Scale F_MINOR     = Scale(CH_MINOR, 5);
static const Scale Fs_HD7      = Scale(CH_HD7, 6);
static const Scale G_MAJOR     = Scale(CH_MAJOR, 7);
static const Scale G_MINOR7    = Scale(CH_MINOR7, 7);
static const Scale G_DOMINANT7 = Scale(CH_DOMINANT7, 7);
static const Scale A_MINOR7    = Scale(CH_MINOR7, 9);
static const Scale B_HD7       = Scale(CH_HD7, -1);
static const Scale B_DIM7      = Scale(CH_DIM7, -1);

// clang-format off
static const Scale* SCALES[]      = {
    &E_DOMINANT7,    &A_MINOR7,    &B_DIM7,        
    &D_MAJOR,       &F_MAJOR7,      &B_HD7,        
    &A_MINOR7,      &C_MAJOR,       &E_MINOR7,       
    &Fs_HD7,        &G_DOMINANT7,     &F_MINOR,   
    &G_MINOR7,      &D_MINOR7,      &G_MAJOR,   
};
// clang-format on

// these are combined to get a "2d index" into the above structure
static const int SCALE_X_SIZE = 3;
static const int SCALE_Y_SIZE = 5;

Quantizer::Quantizer() {}
Quantizer::~Quantizer() {}

void Quantizer::Init()
{
    gateOn = false;
    newNoteGate.Init(patch.AudioSampleRate());
    melodicDirectionGate.Init(patch.AudioSampleRate());
    samplesSinceLastNote = 0;
}

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

bool Quantizer::GetNewNoteTrigger()
{
    bool oldState = gateOn;
    gateOn        = patch.gate_in_1.State();
    return !oldState && gateOn;
}

void Quantizer::DacCallback(uint16_t** output, size_t size)
{
    if((toggle.Pressed()
        && !GetNewNoteTrigger()) // if switch is up, only re-quantize when triggered
       || samplesSinceLastNote < NOTE_DEBOUNCE_SAMPLES)
    {
        CV_OUT_LOWPRIORITY = LED_OUT_LOWPRIORITY = curCvOut;
    }
    else
    {
        float voctIn      = GetCombinedKnobCv(KNOB_OFFSET, CV_VOCT_IN);
        float scaleInputA = GetCombinedKnobCv(KNOB_SCALEA, CV_SCALEA);
        float scaleInputB = GetCombinedKnobCv(KNOB_SCALEB, CV_SCALEB);
        float rootOffsetIn
            = GetCombinedKnobCv(KNOB_ROOT_OFFSET, CV_ROOT_OFFSET);

        // select scale using a "2d" cv selection
        int scaleY = (int)fmap(scaleInputA, 0.f, (float)SCALE_Y_SIZE - 0.01f);
        int scaleX = (int)fmap(scaleInputB, 0.f, (float)SCALE_X_SIZE - 0.01f);

        const Scale& scale = *SCALES[scaleY * SCALE_X_SIZE + scaleX];

        int rootOffset = (int)fmap(rootOffsetIn, -6.f, 6.f);

        // combine offset and input, clamp to 0-1 (5 octaves = 60 semitones)
        float note    = (DSY_CLAMP(voctIn, 0.f, 1.f)) * 60.f;
        float outNote = GetNearestNote(scale, note, rootOffset);

        // convert back to voltage and output
        float outputVal = ((outNote / 60.f) * MAX_VOUT) / CALIBRATE_VOCT;
        float cvout     = VoltageToCvValue(outputVal);

        if(cvout != curCvOut)
        {
            newNoteGate.Trigger();
            samplesSinceLastNote = 0;

            if((cvout > curCvOut && !melodyIsAscending)
               || (cvout < curCvOut && melodyIsAscending))
            {
                melodyIsAscending = !melodyIsAscending;
                melodicDirectionGate.Trigger();
            }
        }

        CV_OUT_LOWPRIORITY = LED_OUT_LOWPRIORITY = curCvOut = cvout;
    }

    dsy_gpio_write(&patch.gate_out_1, newNoteGate.State());
    dsy_gpio_write(&patch.gate_out_2, melodicDirectionGate.State());
}


void Quantizer::AudioCallback(AudioHandle::InputBuffer  in,
                              AudioHandle::OutputBuffer out,
                              size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        newNoteGate.Process();
        melodicDirectionGate.Process();
        samplesSinceLastNote++;
    }
}