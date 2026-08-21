# Experimental architecture

```text
Intiface output 0 → Lovense Vibrate1 → PCA9685 channel 0
Intiface output 1 → Lovense Vibrate2 → PCA9685 channel 1
Serial TEST command                 → channels 0–15 in sequence
```

The ESP32 currently retains the Lovense Edge BLE identity and model identifier
`P`. This deliberately preserves the established two-output Intiface path while
PCA9685 output hardware is developed. It does **not** make 16 outputs available
to Intiface. A future experimental protocol/device definition is required for
custom 4–16-output support.

The PCA9685 handles PWM for all motor-control channels. The stable project's
direct GPIO25/GPIO26 motor PWM implementation is not used in this sketch.

`STOP`, Lovense `PowerOff;`, and Bluetooth disconnect call the all-channel-off
path. BLE advertising restarts after disconnect.
