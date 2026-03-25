# Multi Patchouli

Firmware for the Daisy Patch.Init() with multiple "module modes" with different behaviour.

### Selecting Mode

Press Button B7 to cycle between the modes. The LED will blink a number of times indicating which selection you're on.

1. "GateKeeper" mode
2. "SuperSaw" mode
3. Reverb mode
4. VCA Utility

> **Todo:** do selection by holding the button and moving a CV knob (or maybe two in sequence to have 12 or 16 presets?)

---

## Module Modes

### GateKeeper

Logic to choose whether or not to "let" triggers through. Also provides a nice way to combine 2 trigger sources together. The two decisions algorithms are Random (a trigger is randomly let thru based on a percentage) and Cycled (for stuff like "let three triggers thru, then block 5, repeat"). You can "fade" between these two algorithms - at 50%, Random is used for a (also random) half of the trigger decisions and Cycled for the rest.

**Inputs:**
- `B10`: trigger/gate input
- `B9`: trigger/gate input
- `B8`: combination setting — ORs triggers together (up) or ANDs them (down) before gatekeeping
- `CV_1` + `CV_5`: Weighting of Random vs. Cycle mode (full left is Cycle, right is Random)
- `CV_2` + `CV_6`: Random density for Random Mode
- `CV_3` + `CV_7`: Cycle length, ranged 2 to 16 triggers
- `CV_4` + `CV_8`: On/Off duty cycle for Cycle Mode

**Outputs:**
- `gate_out_1`: Trigger Output
- `gate_out_2`: Inverse Trigger Output (lets thru the "other" triggers)
- `CV_OUT_1`: combined triggers with AND or OR logic (all thru)
- `LED`: On when gate is being let thru

**Unused:**
- Audio ins and outs

**Todo:**
- Make audio out 1 duck audio based on the trigger and an envelope
- Apply same envelope to audio out 2

---

### SuperSaw

This is just the TripleSaw example from the Daisy repo, ported over to fit into my code. I also added CV controls for number of extra voices, which goes from 2 up to 12, and "voice scaling" which makes the more detuned voices slightly quieter relative to the central "main" frequency. Also added a triangle option.

**Inputs:**
- `CV_1`: Tuning
- `CV_2`: Detune amt (goes up to 10%!)
- `CV_3`: Num Voices
- `CV_4`: amt to scale down detuned voices
- `CV_5`: v/oct input
- `B8`: Switches between Saw and Triangle voices

**Outputs:**
- `audio_1`: left channel out
- `audio_2`: right channel out

**Unused:**
- Audio ins
- gate ins/outs
- `CV_OUT_1`
- `CV_6` – `CV_8`

**Ideas:**
- Maybe cut down to max 8 extra voices
- Add MiniGateKeeper
- Have button switch waveforms

---

### Reverb

This is a basic reverb.

**Inputs:**
- `CV_1`: Time
- `CV_2`: Damping
- `CV_3`: Dry Level
- `CV_4`: Send Level
- `audio`: stereo audio input

**Outputs:**
- `audio`: stereo audio output

**Unused:**
- `CV_5` – `CV_8`
- Switch `B8`
- gate ins/outs
- `CV_OUT_1`

**Todo:**
- Add CV control for time/damping/dry/send
- Add MiniGateKeeper
- Make `CV_OUT_1` an envelope that follows the Wet signal level

---

### VCA Utility

This provides 2 audio-rate VCAs (stereo ins/outs) with CV control. And a unipolar CV VCA.

**Inputs:**
- `CV_1`: Level of audio 1
- `CV_2`: Level of audio 2
- `CV_3`: attenuverter of audio 1's CV control
- `CV_4`: attenuverter of audio 2's CV control
- `CV_5`: CV control for audio 1
- `CV_6`: CV control for audio 2
- `CV_7`: CV input
- `CV_8`: CV control for CV input (unipolar)

**Outputs:**
- `audio_left`: audio 1 processed
- `audio_right`: audio 2 processed
- `CV_OUT_1`: CV input processed (unipolar)

**Unused:**
- Switch `B8`
- gate ins/outs

**Todo:**
- Add MiniGateKeeper
