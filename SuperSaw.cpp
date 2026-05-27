#include "SuperSaw.h"
#include "SettingsManager.h"

extern DaisyPatchSM patch;
extern Switch       toggle;
extern Settings     settings;

SuperSaw::SuperSaw() {}
SuperSaw::~SuperSaw() {}

void SuperSaw::Init()
{
    osc_main.Init(patch.AudioSampleRate());
    osc_a.Init(patch.AudioSampleRate());
    osc_b.Init(patch.AudioSampleRate());
    osc_c.Init(patch.AudioSampleRate());
    osc_d.Init(patch.AudioSampleRate());
    osc_e.Init(patch.AudioSampleRate());
    osc_f.Init(patch.AudioSampleRate());
    osc_g.Init(patch.AudioSampleRate());
    osc_h.Init(patch.AudioSampleRate());
    UpdateWaveForm();
}

void SuperSaw::UpdateWaveForm()
{
    int newWF = 0;
    switch(settings.superSawMode)
    {
        case(0): newWF = Oscillator::WAVE_POLYBLEP_SAW; break;
        case(1): newWF = Oscillator::WAVE_POLYBLEP_SQUARE; break;
        case(2): newWF = Oscillator::WAVE_POLYBLEP_TRI; break;
        case(3): newWF = Oscillator::WAVE_SIN; break;
    }
    osc_main.SetWaveform(newWF);
    osc_a.SetWaveform(newWF);
    osc_b.SetWaveform(newWF);
    osc_c.SetWaveform(newWF);
    osc_d.SetWaveform(newWF);
    osc_e.SetWaveform(newWF);
    osc_f.SetWaveform(newWF);
    osc_g.SetWaveform(newWF);
    osc_h.SetWaveform(newWF);
}

void SuperSaw::DacCallback(uint16_t **output, size_t size)
{
    float knob_extraVoices = GetCombinedKnobCv(CV_3, CV_7);
    bool  lfo              = !toggle.Pressed();

    // Get number of voices
    // this just chops off the non-int part of the number
    n_extra_voices = fmap(knob_extraVoices, 0.75f, 4.99f);
    n_extra_voices *= 2; // it needs to be an even number
    n_extra_voices = DSY_CLAMP(n_extra_voices, 0.f, 8.f);

    // Adjust loudness of detuned voices
    float knob_scaleFactor = GetCombinedKnobCv(CV_4, CV_8);
    scaleFactor            = fmap(knob_scaleFactor, 1.f, 2.f);

    // Get pitch, based on inputs and LFO mode
    float cv_voct     = patch.GetAdcValue(CV_5);
    float voct        = fmap(cv_voct, 0, 60);
    float knob_coarse = patch.GetAdcValue(CV_1);
    float coarse_tune = fmap(knob_coarse, lfo ? 0.1f : 24, lfo ? 20 : 72);
    mid_freq
        = lfo ? coarse_tune + voct : VoltageToFrequency(coarse_tune + voct);

    // Set detuning variables
    float detune_amt = GetCombinedKnobCv(CV_2, CV_6);
    detune_incr      = (0.2 * mid_freq * detune_amt) / (n_extra_voices / 2);

    // fudgy adjust for percieved loss of volume when adding voices,
    // and for scaled down detuned voices.
    loudnessFudge
        = 1.f + (n_extra_voices * 0.18f) + ((1 - knob_scaleFactor) * 0.25f);
}

void SuperSaw::AudioCallback(AudioHandle::InputBuffer  in,
                             AudioHandle::OutputBuffer out,
                             size_t                    size)
{
    float offset = 0;
    /** Set all oscillators' frequencies */
    osc_main.SetFreq(mid_freq);

    // Danger: intentional fall-thru cases ahead
    switch(n_extra_voices)
    {
        case 8:
            offset += detune_incr;
            osc_h.SetFreq(mid_freq + offset);
            osc_g.SetFreq(mid_freq - offset);
        case 6:
            offset += detune_incr;
            osc_f.SetFreq(mid_freq + offset);
            osc_e.SetFreq(mid_freq - offset);
        case 4:
            offset += detune_incr;
            osc_d.SetFreq(mid_freq + offset);
            osc_c.SetFreq(mid_freq - offset);
        case 2:
            offset += detune_incr;
            osc_b.SetFreq(mid_freq + offset);
            osc_a.SetFreq(mid_freq - offset);
    }


    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = 0;

        float totalScale = 0;
        float _f         = 1;

        switch(n_extra_voices)
        {
            case 8:
                _f *= scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_h.Process() + osc_g.Process());
            case 6:
                _f *= scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_f.Process() + osc_e.Process());
            case 4:
                _f *= scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_d.Process() + osc_c.Process());
            case 2:
                _f *= scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_b.Process() + osc_a.Process());
        }

        _f *= scaleFactor;
        totalScale += _f;
        sig += _f * osc_main.Process(); // should be the loudest one

        sig = (sig / totalScale) * loudnessFudge;

        OUT_L[i] = sig;
        OUT_R[i] = sig;
    }
}