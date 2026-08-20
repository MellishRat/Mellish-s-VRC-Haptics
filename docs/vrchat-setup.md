# VRChat integration

VRChat integration comes after the two Intiface outputs pass their independent
bench test.

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

## Current status

- ESP32 BLE discovery and connection: verified
- Edge identification: verified
- Two outputs visible in Intiface: verified
- Independent physical motor response from Intiface controls: pending
- OGB feature binding: pending
- VRChat SPS intensity path: pending
