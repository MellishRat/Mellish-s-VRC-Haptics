# Mellish 16-Channel Haptic Controller — Experimental

**Status: experimental; not a replacement for the verified two-channel build.**

This development area moves motor PWM generation to a PCA9685 at I²C address
`0x40`. The PCA9685 shares the ESP32's GPIO21/GPIO22 I²C bus with the SSD1306
OLED at `0x3C`.

The current firmware still identifies as a Lovense Edge. Intiface therefore
exposes only two vibration outputs: output 0 controls PCA9685 channel 0 and
output 1 controls PCA9685 channel 1. Custom 4–16-output Intiface support is
future experimental work; channels 2–15 are not independently addressable from
Intiface yet.

## Current experimental features

- PCA9685 PWM controller at I²C address `0x40`
- SSD1306 OLED and PCA9685 sharing SDA GPIO21 and SCL GPIO22
- PCA9685 logic VCC powered from ESP32 3.3 V
- separate motor power entering at PCA9685 `V+` and `GND`
- all grounds connected in common
- Intiface outputs 0 and 1 mapped to PCA9685 channels 0 and 1
- Serial Monitor `TEST` command pulses channels 0–15 sequentially
- Serial Monitor `STOP`, Lovense `PowerOff;`, and BLE disconnect switch every channel off

## Firmware

Open
[`firmware/Mellish_16_Channel_Experimental/Mellish_16_Channel_Experimental.ino`](firmware/Mellish_16_Channel_Experimental/Mellish_16_Channel_Experimental.ino).
The sketch folder and `.ino` filename match.

Required Arduino libraries:

- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit PWM Servo Driver Library
- BLE support supplied by the Espressif ESP32 Arduino core

## Documentation

- [Experimental scope and protocol](docs/architecture.md)
- [PCA9685 wiring and power guidance](wiring/README.md)
- [Development and test procedure](docs/testing.md)

For a known-good build, use the
[Mellish Edge-Compatible Two-Motor Emulator](../edge-2-channel/README.md).
