# F1shnet Stable Fallback

This is the current tested fallback build for F1shnet.

## Hardware-Tested Behaviour

- Euro-level envelope scaling works with Workshop/euro oscillators.
- `Up Main` output trim is appropriate and starts at minimum on boot.
- `Down Main` controls S&H clock speed with soft pickup.
- `CV In 2` has an audible depth effect.
- S&H resonance is damped to avoid harsh ringing spikes.

## Flashable UF2

- Current UF2: `UF2/F1shnet.0.1.0.uf2`
- Fallback copy: `UF2/stable-fallback/F1shnet.0.1.0.stable-fallback.uf2`
- SHA256: `3555612944e22b2fd5f7e43f47b37cf9cb009db79590191a3bb5dd2187bd59d7`

Use this version as the return point before further filter, envelope, or S&H tuning.
