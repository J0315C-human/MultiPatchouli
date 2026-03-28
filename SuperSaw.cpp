#include "SuperSaw.h"

extern DaisyPatchSM patch;
extern Switch       toggle;

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

    waveForm = toggle.Pressed() ? Oscillator::WAVE_POLYBLEP_SAW
                                : Oscillator::WAVE_POLYBLEP_TRI;
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

void SuperSaw::AudioCallback(AudioHandle::InputBuffer  in,
                             AudioHandle::OutputBuffer out,
                             size_t                    size)
{
    /** Get Coarse, Fine, and V/OCT inputs from hardware 
     *  MIDI Note number are easy to use for defining ranges */
    float knob_coarse      = patch.GetAdcValue(CV_1);
    float detune_amt       = patch.GetAdcValue(CV_2);
    float knob_extraVoices = patch.GetAdcValue(CV_3);
    float knob_scaleFactor = patch.GetAdcValue(CV_4);

    if(toggle.Pressed() && waveForm != Oscillator::WAVE_POLYBLEP_SAW)
    {
        waveForm = Oscillator::WAVE_POLYBLEP_SAW;
        osc_main.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_d.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_e.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_f.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_g.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_h.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    }
    else if(!toggle.Pressed() && waveForm != Oscillator::WAVE_POLYBLEP_TRI)
    {
        waveForm = Oscillator::WAVE_POLYBLEP_TRI;
        osc_main.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_d.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_e.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_f.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_g.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
        osc_h.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    }

    float coarse_tune = fmap(knob_coarse, 24, 72);
    float cv_voct     = patch.GetAdcValue(CV_5);
    float voct        = fmap(cv_voct, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nn  = fclamp((coarse_tune + voct) * CALIBRATE_VOCT, 0.f, 127.f);
    float mid_freq = mtof(midi_nn);

    // this just chops off the non-int part of the number
    int n_extra_voices = fmap(knob_extraVoices, 1.f, 4.9f);
    n_extra_voices *= 2; // it needs to be an even number
    n_extra_voices = DSY_CLAMP(n_extra_voices, 2, 12);

    // there's probably real math that could be here
    float loudnessFudge
        = 1.f + (n_extra_voices * 0.18f) + ((1 - knob_scaleFactor) * 0.25f);

    float increment = (0.1 * mid_freq * detune_amt) / (n_extra_voices / 2);
    float offset    = 0;
    /** Set all oscillators' frequencies */
    osc_main.SetFreq(mid_freq);

    // Danger: intentional fall-thru cases ahead
    switch(n_extra_voices)
    {
        case 8:
            offset += increment;
            osc_h.SetFreq(mid_freq + offset);
            osc_g.SetFreq(mid_freq - offset);
        case 6:
            offset += increment;
            osc_f.SetFreq(mid_freq + offset);
            osc_e.SetFreq(mid_freq - offset);
        case 4:
            offset += increment;
            osc_d.SetFreq(mid_freq + offset);
            osc_c.SetFreq(mid_freq - offset);
        case 2:
            offset += increment;
            osc_b.SetFreq(mid_freq + offset);
            osc_a.SetFreq(mid_freq - offset);
    }

    float _scaleFactor = fmap(knob_scaleFactor, 1.f, 2.f);

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = 0;

        float totalScale = 0;
        float _f         = 1;

        switch(n_extra_voices)
        {
            case 8:
                _f *= _scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_h.Process() + osc_g.Process());
            case 6:
                _f *= _scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_f.Process() + osc_e.Process());
            case 4:
                _f *= _scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_d.Process() + osc_c.Process());
            case 2:
                _f *= _scaleFactor;
                totalScale += 2 * _f;
                sig += _f * (osc_b.Process() + osc_a.Process());
        }

        _f *= _scaleFactor;
        totalScale += _f;
        sig += _f * osc_main.Process(); // should be the loudest one

        sig = (sig / totalScale) * loudnessFudge;

        OUT_L[i] = sig;
        OUT_R[i] = sig;
    }
}