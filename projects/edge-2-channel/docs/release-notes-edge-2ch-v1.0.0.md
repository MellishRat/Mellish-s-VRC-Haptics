# Proposed release notes: `edge-2ch-v1.0.0`

> Proposed only. No release or Git tag has been created.

## Mellish Edge-Compatible Two-Motor Emulator v1.0.0

First stable, hardware-verified release of the two-channel ESP32 haptic controller.

### Highlights

- Lovense Edge BLE emulation using model identifier `P`
- two independent Intiface `0–20` vibration outputs
- GPIO25 Motor 1 and GPIO26 Motor 2 PWM control
- independent calibrated minimum PWM values of `70` and `75`
- SSD1306 128×32 OLED status and motor-level bars on GPIO21/GPIO22 at `0x3C`
- automatic advertising restart after BLE disconnect
- immediate all-motors-off disconnect failsafe
- Arduino ESP32 Core 2.x/3.x PWM compatibility
- documented Intiface-on-phone and OscGoesBrrr-on-PC workflow

### Hardware safety

Motors must be driven through suitable motor-driver modules, MOSFET circuits,
or controllers. Never drive a motor directly from an ESP32 GPIO. Use a
correctly rated supply, appropriate protection, and a common ground.

### Release approval

Create the `edge-2ch-v1.0.0` tag and GitHub release only after explicit owner approval.
