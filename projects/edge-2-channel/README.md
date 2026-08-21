# Mellish Edge-Compatible Two-Motor Emulator

**Status: completed and hardware verified. Recommended starting point.**

This ESP32-WROOM-32 project emulates a Lovense Edge over Bluetooth Low Energy.
Its verified firmware identifies with Lovense model identifier `P`, and Intiface
exposes it as two independent vibration outputs with levels from `0` to `20`.
It is an Edge-compatible two-motor emulator; it is not described as a verified
Gemini emulator.

## Verified hardware and behaviour

- ESP32-WROOM-32 development board (`ESP32 Dev Module`)
- SSD1306 128×32 OLED at I²C address `0x3C`
- OLED SDA on GPIO21 and SCL on GPIO22
- Motor 1 control signal on GPIO25
- Motor 2 control signal on GPIO26
- `MOTOR1_MIN_PWM` = `70`
- `MOTOR2_MIN_PWM` = `75`
- OLED BLE connection status and two motor-level bars
- automatic BLE advertising restart after disconnect
- immediate motor-off disconnect failsafe
- Arduino ESP32 Core 2.x and 3.x PWM API compatibility
- two independent Intiface vibration outputs

The tested signal path is:

```text
VRChat on PC → OscGoesBrrr on PC → Intiface on phone → BLE → ESP32
```

The phone and PC communicate over the local network. The ESP32 itself uses BLE
only: it needs no Wi-Fi credentials and runs no WebSocket server.

## Build and upload

1. Install the Espressif ESP32 Arduino board package.
2. Select **ESP32 Dev Module**.
3. Install **Adafruit GFX Library** and **Adafruit SSD1306**.
4. Open [`firmware/Mellish_Edge_2_Channel/Mellish_Edge_2_Channel.ino`](firmware/Mellish_Edge_2_Channel/Mellish_Edge_2_Channel.ino).
5. Upload, then open Serial Monitor at `115200` baud.
6. Follow the [Intiface and OscGoesBrrr instructions](docs/intiface-setup.md).

The sketch folder and `.ino` filename match, as required by Arduino.

## Documentation

- [Architecture, BLE identity, and protocol](docs/architecture.md)
- [Wiring, pinout, and external motor drivers](wiring/README.md)
- [Shared editable Fritzing layouts](../../images/layout.fzz)
- [Intiface on phone and OscGoesBrrr on PC](docs/intiface-setup.md)
- [VRChat integration](docs/vrchat-setup.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Hardware-verification checkpoint](docs/verified-checkpoint.md)
- [Proposed `edge-2ch-v1.0.0` release notes](docs/release-notes-edge-2ch-v1.0.0.md)

## Safety

GPIO25 and GPIO26 are control signals only. Never connect a motor directly to
an ESP32 GPIO. See the [wiring guide](wiring/README.md) before applying motor
power.
