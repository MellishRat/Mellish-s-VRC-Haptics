# Intiface and OscGoesBrrr setup

## Intiface on the phone

1. Start the Intiface engine.
2. Start a normal Bluetooth device scan in Intiface.
3. Wait for `LVS-Edge` to be discovered and initialized.
4. Confirm the Devices page shows a Lovense Edge with two vibration outputs.

Expected ESP32 Serial output:

```text
BLE advertising as: LVS-Edge
BLE CONNECTED to Intiface
BLE RX: DeviceType;
BLE TX: P:1:MELLISH-HAPTICS-001;
```

Android's normal Bluetooth settings may not list this BLE-only peripheral. The
Intiface scan is authoritative. Do not manually bond the ESP32.

The ESP32 connects only by BLE. It does not need the phone's IP address, Wi-Fi
credentials, or any custom WebSocket-device entry.

## OscGoesBrrr on the PC

With Intiface running on the phone, configure OGB to connect to the phone's LAN
address, for example:

```text
ws://192.168.0.251:12345
```

The actual phone IP can change with DHCP. The ESP32 does not use this address;
only OGB needs it. The verified test used OscGoesBrrr v2.1.28.

## First output test

Before involving VRChat:

1. Set both outputs to 0.
2. Exercise output 0 through `0 → 1 → 10 → 20 → 0`; Motor 2 must remain still.
3. Exercise output 1 through the same sequence; Motor 1 must remain still.
4. Disconnect Intiface and confirm both motors stop immediately.

Expected PWM values are documented in [architecture.md](architecture.md).
