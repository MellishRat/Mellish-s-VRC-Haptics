# Troubleshooting

## Intiface sees `LVS-Edge` in logs but it is absent from Devices

Discovery is not the same as initialization. Check Serial Monitor for
`BLE CONNECTED` followed by `DeviceType;`. Leave the scan running long enough
for initialization and ensure no other app is connected to the ESP32.

## nRF Connect shows an old `12345678...` service

That UUID belongs to the temporary BLE radio-test sketch. Android has cached the
old GATT table. The current firmware uses a separate stable base MAC to avoid
that cache. Upload the latest firmware, reset the ESP32, and select the newly
advertised address. Do not bond it.

## `btleplug error: NotConnected`

- Close nRF Connect completely; only one client can hold the BLE connection.
- Remove any old ESP32 test bond.
- Toggle phone Bluetooth off and on, then reset the ESP32.
- Confirm a dedicated BLE scan sees `LVS-Edge` and service `50300001...`.

## OLED remains blank

Confirm address `0x3C`, SDA GPIO21, SCL GPIO22, 3.3 V, and common ground. The
firmware continues safely without the display and reports the failure over
Serial at 115200 baud.

## Motor does not start at level 1

Increase only that channel's minimum in steps of about five:

```cpp
const uint8_t MOTOR1_MIN_PWM = 70;
const uint8_t MOTOR2_MIN_PWM = 75;
```

Repeat `0 → 1 → 0` five to ten times. Do not change the other channel. Confirm
the other motor remains physically still and its OLED/Serial level remains 0.

## Motor is always on

Disconnect motor power. Verify `IN`, `VCC`, and `GND`, and confirm the driver is
active high. Do not compensate for incorrect wiring in software.

## ESP32 resets or OLED flickers when motors start

Treat this as a power-integrity or motor-noise problem. Use a correctly sized
supply, common ground, short wiring, and appropriate decoupling/suppression.
