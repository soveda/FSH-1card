# F1shnet

An FSH-1-inspired modulation effect card for the Music Thing Modular Workshop
Computer.

This version is now aimed much more directly at the Maestro FSH-1 / modern
F1shy style idea: a voltage-controlled low-pass filter that can be driven either
by synth dynamics or by stepped sample-and-hold modulation.
The normal middle/up behavior is an envelope-controlled low-pass filter with
ratio-style cutoff movement, while the S&H gesture uses a separate pitch-like
low-pass filter path.
The envelope/manual range can close down into low-pass auto-wah territory, while
the S&H path keeps a separate floor for articulation.
The envelope follower is scaled for modular/euro-level sources, so a normal
5-6Vpp oscillator should create usable movement instead of pinning the filter
fully open.
The S&H gesture is tuned as a pitch-like filter movement: random steps move the
filter frequency around the stored base range, up to roughly an octave below and
above before panel depth scaling and output clipping.

Rather than cloning a specific schematic, it adapts that behavior to the
Workshop Computer panel and patch points, using one core voice plus two control
pages and the Computer's momentary `DOWN` switch position:

- `UP` alternate control page
- `MIDDLE` main control page
- `DOWN` held sample-and-hold gesture

The implementation follows the `ComputerCard` guidance in
`Workshop_Computer/Demonstrations+HelloWorlds/AI/WORKSHOP_COMPUTER_AI_DIRECTIVE.md`:
fixed-rate interrupt DSP, integer-first math, a self-contained release-style
folder, and the standard Workshop Computer panel mapping.

## Controls

`Main`

In the main page this is filter range / base cutoff.
In the alternate page this is output trim / gain, scaled to tame hot modular
oscillators before the output clips.
In the momentary down position this is live S&H clock speed only.
Down `Main` minimum is the slowest internal S&H clock; down `Main` maximum is
the fastest. Down `Main` uses soft pickup, so entering the gesture will not jump
the clock until the physical `Main` knob crosses the stored S&H speed.

`X`

In the main page this is depth.
In the alternate page this is envelope sensitivity.
The sensitivity range is tuned for modular-level oscillators and drum machines:
low settings should stay restrained, while high settings should produce a broad
auto-wah sweep without needing guitar-pedal-level gain staging.

`Y`

In the main page this is resonance.
In the alternate page this is envelope decay only.

`Switch`

- `UP`: alternate control page
- `MIDDLE`: main control page
- `DOWN`: held sample and hold

The `UP` and `MIDDLE` pages now use soft pickup:
when you switch pages, a knob keeps the stored value for that page until the
physical knob crosses near it, preventing sudden jumps.
`UP` exposes setup controls but does not change the stored middle-page filter
range, depth, or resonance just by switching.
`DOWN` is a gesture layer: it uses the stored middle-page filter settings and
lets `Main` control S&H clock speed without changing the middle-page filter
range.
While held, the random S&H values move the filter frequency with an octave-style
ratio around the stored middle-page range rather than moving the envelope.
The S&H path has its own moderate lower cutoff floor so low `Main` settings stay bubbly
rather than muddy.
The S&H gesture also uses a little extra resonant emphasis so simple oscillator
waves reveal the random cutoff steps more clearly while staying low-pass.
The slowest internal S&H clock is deliberately moderate rather than extremely
slow, so the gesture remains playable.

## Patch Points

`Audio In 1`

Signal to process.

`Audio In 2`

Secondary audio-rate control source used to push parameters around.

`CV In 1`

Consistent modulation for filter range and envelope sensitivity.

`CV In 2`

Consistent modulation for depth.

`Pulse In 1`

External clock / retrigger.
In sample-and-hold mode it forces a new random step.
In filter modes it can kick the envelope open.

`Pulse In 2`

S&H gate input. After seeing a real low-to-high gate, it forces the
sample-and-hold gesture high while held; this avoids a floating input stealing
the envelope-filter audio path.

`Audio Out 1`

Fully wet envelope-up low-pass output. Plucks and transients open the filter upward.

`Audio Out 2`

Fully wet envelope-down low-pass output. Plucks and transients close the filter from the
same stored range, giving the opposite direction. During S&H gesture both audio
outs carry the same pitch-like S&H low-pass voice.

`CV Out 1`

Stepped sample-and-hold CV, always available.

`CV Out 2`

Envelope follower CV, always available.

`Pulse Out 1`

Mirrors Pulse In 1.

`Pulse Out 2`

Divided sample trigger pulse, fired once for every four new S&H values.

## LEDs

- LEDs `1`, `3`, `5` follow Main, X, and Y.
- LEDs `0` and `4` show alternate page vs main page.
- LED `2` shows when the sample-and-hold gesture is active.

## Current Knob Layout

`Middle` main page

- `Main`: Filter Range
- `X`: Depth
- `Y`: Resonance

`Up` alternate page

- `Main`: Output Trim
- `X`: Envelope Sensitivity
- `Y`: Envelope Decay

`Down` momentary gesture

- `Main`: S&H Speed

## Build

This folder is self-contained in the usual Workshop release style:

- `main.cpp`
- `ComputerCard.h`
- `pico_sdk_import.cmake`
- `CMakeLists.txt`
- `info.yaml`

Typical Pico SDK build flow:

```bash
mkdir -p build
cd build
cmake ..
make -j4
```

The build uses:

- `PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64`
- `pico_set_binary_type(... copy_to_ram)`
- USB stdio disabled
- 192 MHz system clock in `main()`

## Notes

- F1shnet is a Workshop Computer interpretation of the FSH-1 idea, not a verified
  schematic clone.
- The current engine is a simple resonant low-pass structure with envelope or
  random stepped control, tuned more for synth use than guitar picking nuance.
- `UP` now exposes an alternate parameter layer rather than changing to a
  separate opposite-polarity voice.
