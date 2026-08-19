# Mellish's VRC Haptics

DIY ESP32 Wi-Fi haptics for VRChat using OscGoesBrrr, Buttplug/Intiface,
avatar contacts, and vibration motors.

This repository is being developed in deliberately small, testable stages. The
current firmware implements **Stage 1 only**: it verifies the OLED and two
independent motor outputs with a repeating Left → Right → Both → Off sequence.
It does not connect to Wi-Fi or receive OSC yet.

## Stage 1 hardware

- ESP32-WROOM-32 development board
- SSD1306 128×32 I²C OLED (expected address `0x3C`)
- Two 3-pin vibration motor modules with onboard drivers
- A suitable regulated motor supply

See [docs/wiring.md](docs/wiring.md) before applying power. In particular, do
not power a bare motor from a GPIO, and do not assume that every module sold
under the same generic description accepts 3.3 V logic.

## Arduino setup

1. Install ESP32 board support in Arduino IDE and select the board matching your
   ESP32-WROOM-32 development board (typically **ESP32 Dev Module**).
2. Install these libraries using Library Manager:
   - **Adafruit GFX Library**
   - **Adafruit SSD1306**
   - `Wire` is supplied by the ESP32 Arduino core.
3. Open
   `firmware/Mellish_VRC_Haptics/Mellish_VRC_Haptics.ino`.
4. Disconnect motor power for the first upload. Upload the sketch and open the
   Serial Monitor at 115200 baud.
5. Confirm the OLED starts and GPIO 25/26 change in the expected sequence.
6. After completing the checks in [docs/wiring.md](docs/wiring.md), connect motor
   power and repeat the test.

The display shows the current test step and simple left/right activity bars.
The test repeats continuously so wiring faults are easy to isolate.

## Planned stages

1. **Hardware test (current):** OLED and Left → Right → Both → Off motor test.
2. Wi-Fi connection and IP status.
3. Intiface DIY-device proof: pin compatible Intiface/Buttplug versions, connect
   over the Device WebSocket Server, and prove one safe output command.
4. Expose two independent vibration features and map them to left/right PWM.
5. Connect OscGoesBrrr to Intiface and bind the two avatar contact sources to the
   corresponding device features.
6. Optional tuning, provisioning, and expansion only after the two-channel
   prototype is reliable.

See [docs/architecture.md](docs/architecture.md) and
[docs/vrchat-setup.md](docs/vrchat-setup.md) for the intended integration.

## Safety and scope

This is hobby hardware documentation, not a certified wearable design. During
bench testing, keep motors off the body, use current limiting where available,
and stop if a module, wire, regulator, or ESP32 becomes warm.

## License

Licensed under the [MIT License](LICENSE).
