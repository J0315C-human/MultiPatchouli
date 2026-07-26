#include "Quantizer.h"
#include "SettingsManager.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern uint16_t     CV_OUT_LOWPRIORITY;
extern uint16_t     LED_OUT_LOWPRIORITY;
extern Settings     settings;

static const Scale _SUS7    = Scale({0, 5, 2, 3});
static const Scale _MAJOR   = Scale({0, 4, 3});
static const Scale _MAJOR7  = Scale({0, 4, 3, 4});
static const Scale _DOM7    = Scale({0, 4, 3, 3});
static const Scale _MINOR   = Scale({0, 3, 4});
static const Scale _MINOR7  = Scale({0, 3, 4, 3});
static const Scale _MINMAJ7 = Scale({0, 3, 4, 4});
static const Scale _DIM7    = Scale({0, 3, 3, 3});
static const Scale _AUG     = Scale({0, 4, 4, 4});
static const Scale _HD7     = Scale({0, 3, 3, 4});
static const Scale _5TH     = Scale({0, 7});

// transposed scales
static const Scale Cs_HD7    = Scale(_HD7, 1);
static const Scale D_DOM7    = Scale(_DOM7, 2);
static const Scale D_MINOR   = Scale(_MINOR, 2);
static const Scale D_MINOR7  = Scale(_MINOR7, 2);
static const Scale D_MAJOR   = Scale(_MAJOR, 2);
static const Scale Eb_MINOR7 = Scale(_MINOR7, 3);
static const Scale Eb_MAJOR  = Scale(_MAJOR, 3);
static const Scale E_SUS7    = Scale(_SUS7, 4);
static const Scale E_MINOR   = Scale(_MINOR, 4);
static const Scale E_MINOR7  = Scale(_MINOR7, 4);
static const Scale E_DOM7    = Scale(_DOM7, 4);
static const Scale F_DOM7    = Scale(_DOM7, 5);
static const Scale F_MAJOR   = Scale(_MAJOR, 5);
static const Scale F_MAJOR7  = Scale(_MAJOR7, 5);
static const Scale F_MINOR   = Scale(_MINOR, 5);
static const Scale F_SUS7    = Scale(_SUS7, 5);
static const Scale Fs_HD7    = Scale(_HD7, 6);
static const Scale G_MAJOR   = Scale(_MAJOR, 7);
static const Scale G_MINOR   = Scale(_MINOR, 7);
static const Scale G_MINOR7  = Scale(_MINOR7, 7);
static const Scale G_AUG     = Scale(_AUG, 7);
static const Scale G_SUS7    = Scale(_SUS7, 7);
static const Scale G_DOM7    = Scale(_DOM7, 7);
static const Scale Ab_MAJOR  = Scale(_MAJOR, 8);
static const Scale A_MINOR   = Scale(_MINOR, 9);
static const Scale A_MINOR7  = Scale(_MINOR7, 9);
static const Scale A_HD7     = Scale(_HD7, 9);
static const Scale Bb_DOM7   = Scale(_DOM7, -2);
static const Scale Bb_MAJ7   = Scale(_MAJOR7, -2);
static const Scale B_HD7     = Scale(_HD7, -1);
static const Scale B_DIM7    = Scale(_DIM7, -1);

// clang-format off
static const Scale* CHORDS_A[]      = {
    &B_HD7,     &F_MAJOR,      &E_MINOR,
    &A_MINOR,   &_MAJOR,       &F_MINOR,
    &D_DOM7,    &G_MAJOR,      &D_MINOR
};
static const Scale* CHORDS_B[]      = {
    &_MAJOR7,   &F_MINOR,      &Ab_MAJOR,
    &Cs_HD7,    &Eb_MAJOR,     &G_MINOR,
    &G_DOM7,    &Bb_DOM7,      &A_HD7
};
static const Scale* CHORDS_C[]      = {
    &_AUG,      &_MAJOR,       &G_SUS7,
    &D_MINOR7,  &Bb_MAJ7,      &_MINOR7,
    &G_MINOR7,  &Eb_MINOR7,    &F_DOM7
};
static const Scale* CHORDS_D[]      = {
    &F_SUS7,    &A_MINOR,      &Fs_HD7,
    &G_MAJOR,   &_SUS7,        &F_DOM7,
    &E_SUS7,    &D_MINOR,      &G_AUG
};
// clang-format on

// these are combined to get a "3d index" into the above structure
static const int SCALE_X_SIZE  = 3;
static const int SCALE_Y_SIZE  = 3;
static const int SCALE_N_PAGES = 4;

Quantizer::Quantizer() {}
Quantizer::~Quantizer() {}

void Quantizer::Init()
{
    noteGateOn      = false;
    chordPageGateOn = false;
    newNoteGate.Init(patch.AudioSampleRate());
    melodicDirectionGate.Init(patch.AudioSampleRate());
    samplesSinceLastNote = 0;
}
void Quantizer::AdvanceChordPage()
{
    settings.quantizePage = (settings.quantizePage + 1) % NUM_CHORD_PAGES;
    settings.shouldSave   = true;
}

void Quantizer::OnSubModeButtonPress()
{ AdvanceChordPage(); }

void Quantizer::SetSubMode(int subMode)
{
    settings.quantizePage = subMode <= NUM_CHORD_PAGES ? subMode : 0;
    settings.shouldSave   = true;
}

int Quantizer::GetSubMode()
{ return settings.quantizePage; }

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

bool Quantizer::GetNewChordPageTrigger()
{
    bool oldState   = chordPageGateOn;
    chordPageGateOn = patch.gate_in_2.State();
    return !oldState && chordPageGateOn;
}

bool Quantizer::GetNewNoteTrigger()
{
    bool oldState = noteGateOn;
    noteGateOn    = patch.gate_in_1.State();
    return !oldState && noteGateOn;
}

void Quantizer::DacCallback(uint16_t** output, size_t size)
{
    if(GetNewChordPageTrigger())
    {
        AdvanceChordPage();
    }

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

        // select scale using a "3d" cv selection
        int scaleY = (int)fmap(scaleInputA, 0.f, (float)SCALE_Y_SIZE - 0.01f);
        int scaleX = (int)fmap(scaleInputB, 0.f, (float)SCALE_X_SIZE - 0.01f);

        int page = settings.quantizePage;

        const Scale& scale
            = page == 0   ? *CHORDS_A[scaleY * SCALE_X_SIZE + scaleX]
              : page == 1 ? *CHORDS_B[scaleY * SCALE_X_SIZE + scaleX]
              : page == 2 ? *CHORDS_C[scaleY * SCALE_X_SIZE + scaleX]
                          : *CHORDS_D[scaleY * SCALE_X_SIZE + scaleX];


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