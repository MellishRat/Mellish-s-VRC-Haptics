# Hardware-verified checkpoint — 20 August 2026

## Verified

- ESP32-WROOM-32 uploads and runs from the Arduino IDE.
- SSD1306 128×32 OLED works on GPIO21/GPIO22 at `0x3C`.
- GPIO25 and GPIO26 independently control the two motor driver modules.
- ESP32 Bluetooth radio advertising and GATT connection work.
- Intiface on Android discovers `LVS-Edge` over BLE.
- Intiface identifies it as a Lovense Edge.
- The device populates in Intiface with two vibration outputs.
- Intiface controls the two physical motors independently across the `0–20` range.
- Motor 1 has a verified minimum PWM of `70`; Motor 2 has a verified minimum PWM of `75`.
- OGB v2.1.28 connects from the PC to Intiface on the phone over the LAN.
- BLE disconnect invokes the firmware motor-off failsafe.

The ESP32/Intiface two-channel build is complete and hardware verified. Avatar
contact placement and OscGoesBrrr source bindings remain user-specific setup,
not unverified firmware behaviour.

## Important diagnostic result

The initial BLE radio test and haptics firmware shared a Bluetooth address,
causing Android to reuse the diagnostic sketch's cached GATT table. The current
firmware assigns a separate stable locally administered base MAC, ensuring that
Android discovers the correct Lovense service and characteristics.
