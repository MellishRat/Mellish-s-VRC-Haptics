# PCA9685 experimental wiring

## I²C and logic connections

| ESP32 | OLED | PCA9685 | Purpose |
|---|---|---|---|
| 3.3 V | VCC | VCC | Logic power |
| GND | GND | GND | Common reference |
| GPIO21 | SDA | SDA | Shared I²C data |
| GPIO22 | SCL | SCL | Shared I²C clock |

- OLED address: `0x3C`
- PCA9685 address: `0x40`
- PCA9685 `VCC` must be powered from ESP32 3.3 V.
- Motor power enters through PCA9685 `V+` and `GND`, not through `VCC`.
- ESP32, OLED, PCA9685, motor supply, and motor drivers must share a common ground.

```text
ESP32 3V3 ──┬── OLED VCC
            └── PCA9685 VCC (logic)
ESP32 GND ──┬── OLED GND
            ├── PCA9685 GND
            └── regulated motor-supply negative
GPIO21 ─────┬── OLED SDA
            └── PCA9685 SDA
GPIO22 ─────┬── OLED SCL
            └── PCA9685 SCL

regulated motor-supply positive ── PCA9685 V+
PCA9685 channels 0–15 ──────────── suitable motor modules/controllers
```

## Motor modules and supply sizing

The small motor modules are specified for `3.0–5.3 V`, `60 mA` running current,
and `90 mA` starting current. Sixteen motors may demand about `1.44 A` at startup
before allowing for margin, controller losses, wiring losses, or supply
tolerance. Use a properly rated regulated supply selected for the actual motors
and simultaneous startup load.

Do not route the full 16-motor current through solderless breadboard power rails.
Use appropriately rated distribution wiring, connectors, protection, motor
drivers, and decoupling. Never drive a motor from an ESP32 GPIO or assume a bare
PCA9685 output can supply motor current; each PWM channel must feed a suitable
motor-driver or MOSFET input.
