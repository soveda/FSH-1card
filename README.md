# F1shnet

An FSH-1-inspired modulation effect card for the Music Thing Modular Workshop
Computer.

This version is now aimed much more directly at the Maestro FSH-1 / modern
F1shy style idea: a voltage-controlled low-pass filter that can be driven either
by synth dynamics or by stepped sample-and-hold modulation.

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
In the alternate page this is output gain / drive.

`X`

In the main page this is depth.
In the alternate page this is envelope sensitivity.

`Y`

In the main page this is resonance.
In the alternate page this is envelope decay, and it also sets S&H speed during the gesture.
At minimum, the alternate-page `Y` setting is now the slowest internal S&H clock.

`Switch`

- `UP`: alternate control page
- `MIDDLE`: main control page
- `DOWN`: held sample and hold

The `UP` and `MIDDLE` pages now use soft pickup:
when you switch pages, a knob keeps the stored value for that page until the
physical knob crosses near it, preventing sudden jumps.

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

S&H gate input. Forces the sample-and-hold gesture high while held.

`Audio Out 1/2`

Duplicate processed output.

`CV Out 1`

Stepped sample-and-hold CV, always available.

`CV Out 2`

Envelope follower CV, always available.

`Pulse Out 1`

Mirrors Pulse In 1.

`Pulse Out 2`

Sample trigger pulse whenever a new S&H value is taken.

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

- `Main`: Output Gain
- `X`: Envelope Sensitivity
- `Y`: Decay / S&H Speed

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
