# Multi Patchouli

Firmware for the Daisy Patch.Init() with multiple "module modes" with different behaviour.

### Selecting Mode

When the module turns on, the mode will be indicated by blinking.

To change modes, while holding button B7, set the 4 knobs to a binary representation of the number corresponding to the mode . When you release the button, the LED will blink a number of times indicating which selection you're on.

0. ["Favorite" slot]
1. SuperSaw
2. Multi FX
3. VCA Utility
4. Envelope Follower
5. ADSR Envelope
6. Quantizer
7. GateKeeper
8. SlewLimiter

A knob value past 12 o'clock is treated as a 1. The knobs are greatest-to-lowest bit, from top left to bottom right. So for instance, to select mode 5, you'd set them "left-right-left-right", or 0101, which is 5 in binary.

(Yeah I know, it's absurd, but we don't have a lot to work with interface-wise)

To save a favorite (which just sets it as mode 0), press for 5+ seconds and release. After that point, selecting mode 0 will recall the saved mode. Some modes have submodes that are cycled by short-pressing the button; these are also recalled in mode 0, so you could favorite "Multi FX - BitCrusher" for instance.

When a mode starts, its submode is whatever was last used in that mode, with the exception of slot #0, which recalls the favorited setting.

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

**Added layers:**

- Mini Reverb (see below)

---

### SuperSaw

SuperSaw voice with CV controls for number of extra voices, which goes from 2 up to 8, and "voice scaling" which makes the more detuned voices slightly quieter relative to the central "main" frequency. The submode has multiple waveform options.

**Inputs:**

- `CV_1`: Tuning
- `CV_2` + `CV_6`: Detune amt (exponential, goes up to 50%!)
- `CV_3` + `CV_7`: Num extra voices (rounds to 0, 2, 4, 6, or 8)
- `CV_4` + `CV_8`: amt to scale down detuned voice loudness
- `CV_5`: v/oct input
- `Button B7 / SubMode`: Switches between Saw/Square/Tri/Sine
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
  - Reverb submode: `Time, Damping, DryLevel, SendLevel`
  - Delay submode: `Time, Feedback, DryLevel, SendLevel`
  - Pitch Shift submode: `LeftPitch, RightPitch, DryLevel, WetLevel`
  - Bitcrush mode: `BitDepth, CrushRate, DryLevel, WetLevel`
- `audio R/L`: stereo audio input
- `Button B7 / SubMode`: Cycles through Effect Types

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

**Inputs:**

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

**Added layers:**

- Mini GateKeeper (see below)

**Unused:**

- `Button B7`
- `Toggle B8`
- `CV_5` - `CV_8`

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
- `Button B7`: manual gate
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
- `Button B7 / SubMode`: cycles through chord "page"
- `Toggle B8`: when down, constantly re-quantize and ignore trigger
- `gate_in_1`: trigger to re-quantize
- `gate_in_2`: trigger to change chord page

**Outputs:**

- `CV_OUT_1` and LED: V/oct output
- `gate_out_1`: trigger when note changes
- `gate_out_2`: trigger when melody direction changes - ascending to descending or vice versa

**Added layers:**

- Mini Reverb (see below)

---

### Sequencer

A simple 4-value CV sequencer, trigger-controlled.

**Inputs:**

- `CV_1` + `CV_5`: Value 1 input
- `CV_2` + `CV_6`: Value 2 input
- `CV_3` + `CV_7`: Value 3 input
- `CV_4` + `CV_8`: Value 4 input
- `Toggle B8`: when down, ignores value 4 so it's a 3-value sequence
- `gate_in_1`: trigger to advance to next value
- `gate_in_2`: trigger to go back a value

**Outputs:**

- `CV_OUT_1` and LED: CV output

**Added layers:**

- Mini Reverb (see below)

**Unused:**

- `Button B7`
- gate outputs

---

### Slew Limiter

CV Slew Limiter with basic rise/fall controls.

**Inputs:**

- `CV_1`: offset
- `CV_5`: CV input to be slewed
- `CV_2` + `CV_6`: Rise time
- `CV_4` + `CV_8`: Fall time
- `Toggle B8`: up is fast mode, down is slow mode. Just scales the rise/fall values

**Outputs:**

- `CV_OUT_1` and LED: slewed output

**Added layers:**

- Mini GateKeeper (see below)
- Mini Reverb (see below)

**Unused:**

- `CV_3`
- `CV_7`
- `Button B7 / SubMode`

---

## "Mini" Module Modes

These are extra goodies that aren't their own modes, but are layered onto the main modules to make use of inputs/outputs unused by that module. For instance, the Quantizer has no audio function, so it has a Mini Reverb slapped onto it.

### Mini Gatekeeper (Layer Only)

This is a mini version of the Gatekeeper that just splits the triggers 2/3 and 1/3 randomly. The two trigger ins are ORed together before gatekeeping. 

**Inputs:**

- `gate_in_1`: trigger/gate input
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
- `audio`: audio passthru (unless overridden by current main module mode)

### Mini Reverb (Layer Only)

Just a simple reverb with preset middle-of-the-road levels.

**Inputs:**

- `audio L/R`: audio Inputs

**Outputs:**

- `audio L`: dry signal + reverb
- `audio R`: fully wet reverb

---

### Other Plans for Future...

- Granular effect
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
