# Multi Patchouli

Firmware for the Daisy Patch.Init() with multiple "module modes" with different behaviour.

### Selecting Mode

Long-Press button B7 to cycle between the modes. The LED will blink a number of times indicating which selection you're on. When the module turns on, the current mode will be indicated as well.

1. SuperSaw
2. Multi FX
3. VCA Utility
4. Envelope Follower
5. ADSR Envelope
6. Quantizer
7. GateKeeper

---

## Module Modes

### GateKeeper

Logic to choose whether or not to "let" triggers through. Also provides a nice way to combine 2 trigger sources together. The two decisions algorithms are Random (a trigger is randomly let thru based on a percentage) and Cycled (for stuff like "let three triggers thru, then block 5, repeat"). You can "fade" between these two algorithms - at 50%, Random is used for a (also random) half of the trigger decisions and Cycled for the rest.

**Inputs:**

- `Toggle B8`: combination setting — ORs triggers together (up) or ANDs them (down) before gatekeeping
- `game_in_1`: trigger/gate input
- `gate_in_2`: trigger/gate input
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
- OR, audio in/out could just be a basic reverb

---

### SuperSaw

This is just the TripleSaw example from the Daisy repo, ported over to fit into my code. I also added CV controls for number of extra voices, which goes from 2 up to 8, and "voice scaling" which makes the more detuned voices slightly quieter relative to the central "main" frequency. Also added a triangle option.

**Inputs:**

- `CV_1`: Tuning
- `CV_2` + `CV_6`: Detune amt (exponential, goes up to 50%!)
- `CV_3` + `CV_7`: Num extra voices (rounds to 0, 2, 4, 6, or 8)
- `CV_4` + `CV_8`: amt to scale down detuned voices
- `CV_5`: v/oct input
- `Button B7`: Switches between Saw/Square/Tri/Sine
- `Toggle B8`: Up is VCO range, Down is LFO range

**Outputs:**

- `audio L`: left channel out
- `audio R`: right channel out

**Memory:**

- Waveform mode is saved to SD

**Added layers:**

- Mini GateKeeper (see below)
- Mini Envelope Follower (see below)

---

### Multi FX

This is an effects mode with a basic Reverb, Delay, Pitch Shifter, and Bit Crusher.

**Inputs:**

- `CV_1` + `CV_5`: Param 1
- `CV_2` + `CV_6`: Param 2
- `CV_3` + `CV_7`: Param 3
- `CV_4` + `CV_8`: Param 4
- Params in...
  - Reverb mode: `Time, Damping, DryLevel, SendLevel`
  - Delay mode: `Time, Feedback, DryLevel, SendLevel`
  - Pitch Shift mode: `LeftPitch, RightPitch, DryLevel, WetLevel`
  - Bitcrush mode: `BitDepth, CrushRate, DryLevel, WetLevel`
- `audio R/L`: stereo audio input
- `Button B7`: Switches between Effect Type

**Outputs:**

- `audio R/L`: stereo audio output
- `CV_OUT_1`: envelope follower of wet effect signal

**Memory:**

- Effect mode is saved to SD

**Added layers:**

- Mini GateKeeper (see below)

**Unused:**

- `Toggle B8`

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

- `Toggle B8`

---

### Envelope Follower

This is an envelope follower for the left input and an auto-ducker for the right input (ducking the envelope from the left). CV output is the envelope, with some parameters for attack/release and scaling.

**Inputs:**a

- `CV_1`: MS attack (1 to 500ms)
- `CV_2`: MS release (1 to 2000ms)
- `CV_3`: envelope scaling (also affects ducking)
- `CV_4`: audio R makeup gain
- `audio L`: audio input for follower
- `audio R`: audio input to be ducked

**Outputs:**

- `audio L`: passthru audio of L
- `audio R`: audio R with ducking applied using the envelope of audio L
- `CV_OUT_1`: follower envelope

**Unused:**

- `Toggle B8`
- `CV_5` - `CV_8`
- gate ins/outs

**Todo:**

- add Gate utilities (thresholds)

---

### ADSR Envelope

Simple ADSR Envelope. The audio outputs either apply it as a VCA (left) or "duck" a signal using the inverse of the envelope (R).

**Inputs:**a

- `CV_1` + `CV_5`: Attack time
- `CV_2` + `CV_6`: Decay time
- `CV_3` + `CV_7`: Sustain level
- `CV_4` + `CV_8`: Release time
- `audio L`: audio input for follower
- `audio R`: audio input to be ducked
- `Button B7`: manual gate input
- `Toggle B8`: when down, envelope is 1/2 size
- `gate_in_1`: gate input

**Outputs:**

- `audio L`: audio with VCA applied
- `audio R`: audio with ducking applied using the envelope
- `CV_OUT_1` and LED: envelope CV output

**Unused:**

- gate input 2

---

### Quantizer

CV Quantizer with a bunch of preset chord changes in a somewhat arbitrary 3d arrangement.

There are 4 chord "pages", each a 3x3 grid of chords. The controls move around on the current page, trigger input "turns" the page.

So from every chord, you have between 3 and 4 chords you can switch to depending on what input you change. More if you count the root offset.

**Inputs:**

- `CV_1` + `CV_5`: V/oct input
- `CV_2` + `CV_6`: chord select X axis
- `CV_3` + `CV_7`: Root offset (changes what "key" it thinks you're quantizing to)
- `CV_4` + `CV_8`: chord select Y axis
- `Button B7`: go to next chord page
- `Toggle B8`: when down, constantly re-quantize and ignore trigger
- `gate_in_1`: trigger to re-quantize
- `gate_in_2`: trigger to change chord page

**Outputs:**

- `CV_OUT_1` and LED: V/oct output
- `gate_out_1`: trigger when note changes
- `gate_out_2`: trigger when melody direction changes - ascending to descending or vice versa

**Unused:**

- audio ins/outs

---

### Mini Gatekeeper (Layer Only)

This is a mini version of the Gatekeeper, that just splits the triggers 2/3 and 1/3 randomly. The two trigger ins are ANDed together before gatekeeping. Note: this isn't its own mode, but is layered on top of some other modes that had the gate ins/outs free.

**Inputs:**

- `game_in_1`: trigger/gate input
- `gate_in_2`: trigger/gate input

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

- Granular Processor effect
- CV Slew Limiter - basically do the Up/Down slewing that Maths does
- Note generator - CV output for melodies
- Drum w/ 4 trigger inputs and 2 CV inputs?

## Various Notes

- This repo expects libDaisy and DaisySP to be in the root directory.
- Due to SRAM space limitations, I had to switch to set `SHIFT_BUFFER_SIZE` to `8192` in DaisySP's pitchshifter.h and `DSY_REVERBSC_MAX_SIZE` to `70000` in reverbsc.h
- An SD card is needed to save settings.
- This doesn't fit on the internal flash of the Patch SM, so the flash sequence is:
  - Enter DFU mode by holding `boot` then `reset`, then releasing `reset` then `boot`
  - Run `make program-boot` to flash the bootloader
  - press+release `reset` then press+release `boot`
  - `build_and_program_dfu` will now load this code into the QSPI region.
