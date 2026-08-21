# VRChat integration

First confirm the completed two-channel build using Intiface's independent
output controls, then configure the VRChat-specific mapping.

## Planned data path

```text
VRCFury/SPS Touch Zones → VRChat OSC → OscGoesBrrr → Intiface → BLE ESP32
```

Use VRCFury SPS Touch Zones with OGB IDs such as `VB01` and `VB02`. OGB should
map those sources to the two features of the single Edge-compatible device:

```text
VB01 → output 0 → GPIO25 → Motor 1
VB02 → output 1 → GPIO26 → Motor 2
```

The firmware intentionally has no knowledge of body-part names. All semantic
mapping belongs in the avatar/OGB configuration.

The BLE device, two Intiface outputs, and independent physical motor response
are hardware verified. OGB IDs and avatar contact placement are configured per
avatar and are not embedded in the firmware.
