# Standalone PCA9685 motor bench test

Use this sketch to isolate the ESP32, PCA9685, motor supply, and driven motor
modules from BLE, Intiface, and the OLED:

[`../firmware/PCA9685_Motor_Bench_Test/PCA9685_Motor_Bench_Test.ino`](../firmware/PCA9685_Motor_Bench_Test/PCA9685_Motor_Bench_Test.ino)

## Minimum wiring

| Connection | Destination |
|---|---|
| ESP32 3.3 V | PCA9685 `VCC` |
| ESP32 GND | PCA9685 GND and external-supply negative |
| ESP32 GPIO21 | PCA9685 SDA |
| ESP32 GPIO22 | PCA9685 SCL |
| PCA9685 `OE` | GND |
| External regulated 5 V | PCA9685 `V+` |
| Module `IN` | Selected PCA9685 `S`/PWM pin |
| Module `VCC` | 5 V motor rail |
| Module `GND` | Common ground |

Upload as **ESP32 Dev Module**, then open Serial Monitor at 115200 baud with
**Newline** or **Both NL & CR** selected. Disconnect motor power while uploading.

## Commands

- `STATUS` confirms the controller is still visible at `0x40`.
- `CH 1 4095` holds channel 0 fully on for voltage measurements.
- `CH 1 2048` holds channel 0 at approximately half duty cycle.
- `OFF` forces every channel off.
- `TEST` drives channels 0–15 fully on, one at a time for one second, with a
  half-second off gap. It never activates multiple motors simultaneously.

During `CH 1 4095`, expect about 3.3 V from channel 0 signal to common ground.
The motor module must independently measure about 5 V across its `VCC`/`GND`.
If those readings are correct but the motor does not run, disconnect the module
`IN` from the PCA9685 and briefly apply ESP32 3.3 V directly to `IN` to isolate
the motor module from the controller.
