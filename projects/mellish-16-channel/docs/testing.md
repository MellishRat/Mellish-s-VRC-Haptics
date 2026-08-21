# Safe staged testing and calibration

1. Power ESP32 with **motor power disconnected**.
2. Confirm OLED `0x3C`, PCA9685 `0x40`, and all outputs off.
3. Connect a current-limited supply and run `TEST`; only one motor may run and
   all must be off after completion or `STOP`.
4. Use four `CH` commands to prove four mappings remain independent.
5. Expand to all 16 only with correctly wired regulated power around 5 V / 3 A.
   Never use `ALL` on a sub-700 mA breadboard supply.
6. During one-channel operation, disconnect BLE and verify immediate stop before
   advertising resumes.

## Calibration

`motorMinimumPWM8[16]` stores each minimum on the original 8-bit scale. Motors 1
and 2 preserve 70/75; unmeasured channels start at 75. Calibrate one motor at a
time, raising its value only until reliable cold starts. Conversion is rounded
`min8 × 4095 / 255`; level 0 is exactly 0 and level 20 exactly 4095.

## Software checks

- combined/fragmented commands, whitespace, and line endings;
- channels 1 and 16 accepted; 0/17, negative/>20, junk, and missing values rejected;
- explicit 16-output startup-off loop and disconnect stop path;
- one-to-one mapping and independent retained levels;
- non-blocking `TEST`, leaving every channel at zero after finish/interruption.

| ESP32 Arduino core | Board | Result |
|---|---|---|
| 2.0.17 | ESP32 Dev Module | PASS — 1,155,525 bytes flash (88%), 40,116 bytes globals (12%) |
| 3.3.11 | ESP32 Dev Module | PASS — 1,140,459 bytes flash (87%), 43,524 bytes globals (13%) |

Compiled 2026-08-21 with Arduino CLI 1.5.1, Adafruit GFX 1.12.6,
SSD1306 2.5.17, PWM Servo Driver 3.0.3, and BusIO 1.17.4. Host contract
tests: 5/5 passing. Hardware observations remain pending.
