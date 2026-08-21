# Experimental test procedure

1. Disconnect motor power and verify the wiring against the
   [PCA9685 wiring guide](../wiring/README.md).
2. Upload the experimental sketch and open Serial Monitor at `115200` baud.
3. Confirm the OLED is found at `0x3C` and the PCA9685 at `0x40`.
4. With a current-limited, correctly rated motor supply connected to `V+` and
   `GND`, enter `TEST` in Serial Monitor.
5. Confirm channels 0 through 15 pulse one at a time and return off.
6. Enter `STOP` at any time to switch every channel off.
7. In Intiface, confirm output 0 affects only PCA9685 channel 0 and output 1
   affects only channel 1.
8. Disconnect Bluetooth and confirm all channels switch off immediately.

The `TEST` command is a bench diagnostic, not an Intiface 16-output interface.
