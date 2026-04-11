# Multi Patchouli

Firmware for the Daisy Patch.Init() with multiple "module modes" with different behaviour.

### Selecting Mode

Long-Press button B7 to cycle between the modes. The LED will blink a number of times indicating which selection you're on.

1. "GateKeeper" mode
2. "SuperSaw" mode
3. Reverb mode
4. VCA Utility
5. Envelope Follower    

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
- Make audio out 1 duck audio based on the trigger and an envelope?
- Apply same envelope to audio out 2, but only as a positive multiplier
- OR, audio in/out could just be an average reverb

---

### SuperSaw

This is just the TripleSaw example from the Daisy repo, ported over to fit into my code. I also added CV controls for number of extra voices, which goes from 2 up to 8, and "voice scaling" which makes the more detuned voices slightly quieter relative to the central "main" frequency. Also added a triangle option.

**Inputs:**
- `CV_1`: Tuning
- `CV_2` + `CV_6`: Detune amt (goes up to 10%!)
- `CV_3` + `CV_7`: Num extra voices (rounds to 0, 2, 4, 6, or 8)
- `CV_4` + `CV_8`: amt to scale down detuned voices
- `CV_5`: v/oct input
- `B7`: Switches between Saw/Square/Tri/Sine
- `B8`: Up is VCO range, Down is LFO range

**Outputs:**
- `audio L`: left channel out
- `audio R`: right channel out

**Added layers:**
- Mini GateKeeper (see below)
- Mini Envelope Follower (see below)

---

### Reverb

This is a basic reverb.

**Inputs:**
- `CV_1` + `CV_5`: Time
- `CV_2` + `CV_6`: Damping
- `CV_3` + `CV_7`: Dry Level
- `CV_4` + `CV_8`: Send Level
- `audio R/L`: stereo audio input
- `CV_OUT_1`: envelope follower of wet effect signal

**Outputs:**
- `audio R/L`: stereo audio output

**Added layers:**
- Mini GateKeeper (see below)

**Unused:**
- Switch `B8`

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
- `audio L`: audio 1 input
- `audio R`: audio 2 input

**Outputs:**
- `audio L`: audio 1 processed
- `audio R`: audio 2 processed
- `CV_OUT_1`: CV input processed (unipolar)

**Added layers:**
- Mini GateKeeper (see below)

**Unused:**
- Switch `B8`

<!-- **Todo:** -->

### Envelope Follower

This provides 2 audio-rate VCAs (stereo ins/outs) with CV control. And a unipolar CV VCA.

**Inputs:**a
- `audio L`: audio input for follower
- `audio R`: audio input to be ducked
- `CV_1`: MS attack (1 to 500ms)
- `CV_2`: MS release (1 to 2000ms)
- `CV_3`: envelope scaling (also affects ducking)
- `CV_4`: audio R makeup gain

**Outputs:**
- `audio L`: passthru audio of L
- `audio R`: audio R with ducking applied using the envelope of audio L
- `CV_OUT_1`: follower envelope

**Unused:**
- Switch `B8`
- `CV_5` - `CV_8`
- gate ins/outs

**Todo:**
- add Gate utilities (thresholds)

---

### Mini Gatekeeper (Layer Only)

This is a mini version of the Gatekeeper, that just splits the triggers 2/3 and 1/3 randomly. The two trigger ins are ANDed together before gatekeeping. Note: this isn't its own mode, but is layered on top of some other modes that had the gate ins/outs free.

**Inputs:**
- `B10`: trigger/gate input
- `B9`: trigger/gate input

**Outputs:**
- `gate_out_1`: Trigger Output
- `gate_out_2`: Inverse Trigger Output (lets thru the "other" triggers)

### Mini Envelope Follower (Layer Only)

This provides a simple envelope follower. Envelope params are preset with good "general use case" values.

**Inputs:**
- `audio L`: audio Input
- `audio R`: audio Input

**Outputs:**
- `CV_OUT_1`: follower envelope of audio L
- `audio`: audio passthru (unless overridden by other module)

---

### Other Plans for Future...

- Save current Mode when switching
- Quantizer w/ "new chord" trigger
- Pitch shift/Granular Processor
- EG - use Basic Example
- CV Slew Limiter - basically do the Up/Down slewing that Maths does
- Note generator - CV output for melodies
- garbled/granular effect idea
- Drum w/ 4 trigger inputs and 2 CV inputs? 


## Various Notes

- This repo expects libDaisy and DaisySP to be in the root directory.