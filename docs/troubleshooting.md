# Stage 1 troubleshooting

## OLED stays blank

- Confirm VCC is 3.3 V, ground is common, SDA is GPIO 21, and SCL is GPIO 22.
- Check Serial Monitor at 115200 baud. The motor test continues even when OLED
  initialization fails.
- Run an I²C scanner. Some look-alike displays use a different address; change
  `kOledAddress` only after confirming the detected address.
- Confirm the display really is a 128×32 SSD1306 rather than a look-alike using a
  different controller.

## Motor never runs

- Disconnect power before changing wiring.
- Confirm module VCC with a meter and confirm ESP32 ground is connected to module
  ground.
- Measure the module `IN` pin. It should switch between about 0 V and 3.3 V.
- Verify whether the module is active HIGH or active LOW and whether 3.3 V meets
  its documented HIGH threshold.
- Test one module at a time with a suitable current-limited supply.

## Motor is always on

- Disconnect motor power immediately.
- Verify `IN`, `VCC`, and `GND` are not swapped.
- Check whether the module is active LOW. Update `kMotorOn`/`kMotorOff` only after
  identifying the module behavior.

## ESP32 resets or OLED flickers when a motor starts

This indicates a power integrity or motor-noise problem. Do not work around it by
adding delays. Use a correctly sized motor supply, short ground wiring, suitable
decoupling, and the suppression recommended for the exact module. Test motors
off-body until the hardware is stable.

## Left and right are reversed

Move the module input leads to their documented pins: left to GPIO 25 and right
to GPIO 26. Do not swap powered connections.
