# Mellish's VRC Haptics

ESP32 haptic-controller projects for Intiface and OscGoesBrrr. Start with the
completed two-channel build unless you specifically want to develop and test
the experimental PCA9685 design.

| Project | Status | Description |
|---|---|---|
| [Mellish Edge-Compatible Two-Motor Emulator](projects/edge-2-channel/README.md) | **Stable and verified** | Hardware-verified ESP32 build with two independent `0–20` Intiface vibration outputs on GPIO25 and GPIO26. |
| [Mellish 16-Channel Haptic Controller — Experimental](projects/mellish-16-channel/README.md) | **Experimental** | PCA9685 development area. Channels 0 and 1 map to the two currently exposed Edge-compatible outputs; custom 4–16-output Intiface support is future work. |

## Recommended starting point

Build the [stable two-channel project](projects/edge-2-channel/README.md) first.
It contains the verified firmware, wiring guidance, phone/PC connection steps,
troubleshooting notes, and the proposed `edge-2ch-v1.0.0` release notes.

The [16-channel project](projects/mellish-16-channel/README.md) is intentionally
kept separate so experimental PCA9685 work cannot be confused with the verified
direct-GPIO firmware.

## Shared images

Component reference photographs are retained in [`images/`](images/README.md).
Project-specific wiring diagrams live with each project.

## Safety

Never drive a motor directly from an ESP32 GPIO. Use a suitably rated driver or
MOSFET, a correctly rated regulated motor supply, appropriate protection, and a
common ground. Disconnect motor power before changing wiring.

## License

Licensed under the [MIT License](LICENSE).
