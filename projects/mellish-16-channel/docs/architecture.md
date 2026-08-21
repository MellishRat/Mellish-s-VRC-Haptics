# ADR-001: honest 16-output Intiface integration

- **Date:** 2026-08-21
- **Decision:** use a custom BLE identity/protocol and add a proper Buttplug
  protocol/device definition for 16 `Vibrate` features.

```text
VRChat PC → OscGoesBrrr → WebSocket → Intiface Central Android
             16 links                   custom Buttplug handler
                                             ↓ BLE
                                    ESP32 → PCA9685 → 16 motors
```

## Options and evidence

### User-device configuration with an existing handler

Not sufficient. User configuration can match hardware to an **implemented**
protocol and customize known properties; it cannot provide new handler code.
Current Intiface Central stores/edits user device settings on mobile, but source
inspection found no general Android import mechanism for a brand-new encoder.

- [Engine user-device-config option](https://github.com/intiface/intiface-engine#command-line-options)
- [Config explanation: users cannot define new protocols](https://github.com/buttplugio/buttplug-device-config/blob/master/buttplug-device-config.yml)
- [Current user-config state](https://github.com/intiface/intiface-central/blob/main/lib/bloc/device_configuration/user_device_configuration_cubit.dart)
- [Current feature-settings UI](https://github.com/intiface/intiface-central/blob/main/lib/page/device_detail_page.dart)

### Reuse Lovense Edge

Rejected. Edge reports model `P`; its current definition/test exposes two motors
and sends only `Vibrate1`/`Vibrate2`. Stock configuration never supplies 16 Edge
features, and claiming Edge would falsely identify this device.

- [Current Edge protocol test](https://github.com/buttplugio/buttplug/blob/master/crates/buttplug_tests/tests/util/device_test/device_test_case/test_lovense_edge.yaml)

### Proper custom Buttplug protocol/device definition

Selected. Add this BLE name/service/characteristics to Buttplug configuration;
implement a handler announcing 16 `Vibrate` features (range 0–20) and writing
`Vibrate1`…`Vibrate16`. Until included in a released engine, Android requires a
custom Intiface Central/engine APK. Stock support must not be claimed before an
upstream release or demonstrated end-to-end custom APK test.

Serial/USB changes the wireless architecture; Wi-Fi/WebSocket introduces
credentials and changes the requested link, so neither is selected.

## OscGoesBrrr

Current OGB enumerates every returned device feature, creates an output ID
`intiface.<deviceIndex>.<featureIndex>`, and sends `OutputCmd` with that exact
feature index. It can independently address 16 outputs once Intiface exposes
them. Map feature 0–15 to motors 1–16 and configure one SPS/OSC/avatar-parameter
link per desired zone. OGB will not invent 16 avatar parameters, so the avatar
and user configuration must provide them. No OGB code change is currently shown
necessary; bulk-mapping UX could still be useful.

- [Feature enumeration/indexed commands](https://github.com/OscToys/OscGoesBrrr/blob/main/src/main/Intiface.ts)
- [Buttplug v4 message shapes](https://github.com/OscToys/OscGoesBrrr/blob/main/src/main/IntifaceProtocol.ts)

## Remaining limitations

- no stock Intiface release demonstrates this custom device;
- Buttplug handler/config and custom Android APK are outside this repository;
- BLE throughput under rapid 16-feature updates needs measurement;
- electrical, calibration, EMI, and disconnect behavior need staged hardware tests.
