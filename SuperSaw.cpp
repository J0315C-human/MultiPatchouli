#include "SuperSaw.h"

extern DaisyPatchSM patch;
extern Switch       toggle;

SuperSaw::SuperSaw() {}
SuperSaw::~SuperSaw() {}

void SuperSaw::Init()
{
    btn.Init(ButtonPressHelper::SHORT_PRESS);

    osc_main.Init(patch.AudioSampleRate());
    osc_a.Init(patch.AudioSampleRate());
    osc_b.Init(patch.AudioSampleRate());
    osc_c.Init(patch.AudioSampleRate());
    osc_d.Init(patch.AudioSampleRate());
    osc_e.Init(patch.AudioSampleRate());
    osc_f.Init(patch.AudioSampleRate());
    osc_g.Init(patch.AudioSampleRate());
    osc_h.Init(patch.AudioSampleRate());

    waveForm = -1; // default us to Saw
    SwitchWaveForm();
}

void SuperSaw::SwitchWaveForm()
{
    int newWF = 0;
    switch(waveForm)
    {
        case(Oscillator::WAVE_POLYBLEP_SAW):
            newWF = Oscillator::WAVE_POLYBLEP_SQUARE;
            break;
        case(Oscillator::WAVE_POLYBLEP_SQUARE):
            newWF = Oscillator::WAVE_POLYBLEP_TRI;
            break;
        case(Oscillator::WAVE_POLYBLEP_TRI):
            newWF = Oscillator::WAVE_SIN;
            break;
        default: newWF = Oscillator::WAVE_POLYBLEP_SAW;
    }
    waveForm = newWF;
    osc_main.SetWaveform(waveForm);
    osc_a.SetWaveform(waveForm);
    osc_b.SetWaveform(waveForm);
    osc_c.SetWaveform(waveForm);
    osc_d.SetWaveform(waveForm);
    osc_e.SetWaveform(waveForm);
    osc_f.SetWaveform(waveForm);
    osc_g.SetWaveform(waveForm);
    osc_h.SetWaveform(waveForm);
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
    detune_incr      = (0.1 * mid_freq * detune_amt) / (n_extra_voices / 2);

    // adjust for percieved loss of volume when adding voices,
    // and for detuned voices being scaled down. [there's probably real math that could be here]
    loudnessFudge
        = 1.f + (n_extra_voices * 0.18f) + ((1 - knob_scaleFactor) * 0.25f);

    // settings
    if(btn.ProcessAndCheckTrigger())
    {
        SwitchWaveForm();
    }
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