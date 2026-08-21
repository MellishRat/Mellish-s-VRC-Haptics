# Mellish 16-Channel Haptic Controller — Experimental

> **Status (2026-08-21):** ESP32/PCA9685 firmware is ready for staged bench
> testing. Stock Intiface Central does **not** yet discover this custom protocol
> or expose 16 outputs. A Buttplug protocol/config contribution—and until it is
> released, a custom Intiface build—is required.

This custom ESP32 controller drives 16 independent motor modules. It advertises
as `MELLISH-16CH` and does not impersonate a commercial Lovense product. The
completed [two-channel reference](../edge-2-channel/README.md) remains unchanged.

## Channel mapping

| Motor | PCA | Motor | PCA | Motor | PCA | Motor | PCA |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0 | 5 | 4 | 9 | 8 | 13 | 12 |
| 2 | 1 | 6 | 5 | 10 | 9 | 14 | 13 |
| 3 | 2 | 7 | 6 | 11 | 10 | 15 | 14 |
| 4 | 3 | 8 | 7 | 12 | 11 | 16 | 15 |

OLED `0x3C` and PCA9685 `0x40` share SDA GPIO21/SCL GPIO22. PCA logic is
3.3 V; motors use a separate regulated 5 V supply; all grounds are common.

## Firmware features

- 16 independent levels (0–20), per-channel calibration, and 0/4095 endpoints;
- all-off startup and BLE-disconnect failsafes;
- persistent fragmented/multi-command BLE buffering and strict validation;
- BLE callbacks only queue work; OLED/PCA9685 I²C stays in `loop()`;
- readable OLED connection, active-count, highest-level, and test status;
- non-blocking, one-motor-at-a-time sequential test.

Open the matching [Arduino sketch](firmware/Mellish_16_Channel_Experimental/Mellish_16_Channel_Experimental.ino).
At 115200 baud use `STOP`, `TEST`, `CH <1-16> <0-20>`, `ALL <0-20>`, and
`STATUS`. `ALL` prints a power warning: never run simultaneous motors from a
sub-700 mA breadboard supply; the production target is about 5 V / 3 A.

## Documentation

- [Architecture decision and Intiface/OscGoesBrrr boundary](docs/architecture.md)
- [Custom BLE protocol](docs/protocol.md)
- [Installation and libraries](docs/firmware-installation.md)
- [Safe testing and calibration](docs/testing.md)
- [Wiring and power](wiring/README.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Standalone PCA9685 motor bench test](docs/pca9685-bench-test.md)

Implemented: firmware-side control, parsing, serial tools, display, and safety.
Pending: hardware validation, Buttplug handler/config, custom Android build test,
and end-to-end VRChat → OscGoesBrrr → Android Intiface validation.
