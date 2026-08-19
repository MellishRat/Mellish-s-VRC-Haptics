# OscGoesBrrr and Buttplug architecture

This project will use **OscGoesBrrr (OGB)** as the VRChat integration layer and
**Intiface Central** as the Buttplug server/hardware manager.

```text
VRChat Contact Receivers
          |
          | OSC avatar parameters
          v
   OscGoesBrrr (PC)
          |
          | Buttplug client connection
          v
   Intiface Central
          |
          | DIY Device WebSocket connection over Wi-Fi
          v
 ESP32 device: [Left vibrator] [Right vibrator]
```

## Responsibilities

- **VRChat** produces contact values from the avatar.
- **OscGoesBrrr** reads those values and maps contact sources to haptic device
  outputs.
- **Intiface Central** is the Buttplug server. OGB connects to it as a client.
- **ESP32** connects to Intiface's Device WebSocket Server as hardware. It does
  not need to parse VRChat OSC and it is not itself the main Buttplug server.
- The ESP32 should appear as one device with two independently addressable
  vibration features, if the pinned Intiface device configuration supports that
  cleanly. If OGB cannot bind features separately, the tested fallback is two
  logical one-vibrator devices backed by the same ESP32 connection/firmware.

## Why the integration is a separate stage

Buttplug's current documentation describes the WebSocket Device Manager as a
reference-implementation feature rather than part of the stable Buttplug
protocol. The v4 documentation currently has no complete device example. It
also requires a User Device Configuration File (UDCF) and a supported emulated
device protocol after the initial WebSocket handshake.

For that reason, do not hard-code an assumed current protocol into Stage 1. In
the integration stage we will:

1. Record the exact Intiface Central, Buttplug, and OGB versions used for the
   prototype.
2. Enable Intiface's Device WebSocket Server and create the matching UDCF.
3. Select the simplest supported two-vibrator protocol representation.
4. Prove discovery, initialization, one output, stop, disconnect, and timeout
   behavior before adding the second output.
5. Capture the actual binary WebSocket command traffic in automated parser
   tests so later updates cannot silently change motor behavior.

## Required firmware safety behavior

- Both outputs remain off during boot, Wi-Fi connection, and device discovery.
- A `Stop` command stops both outputs.
- Disconnect, protocol error, or command timeout stops both outputs.
- Values are clamped to `0.0–1.0` before PWM conversion.
- Left and right commands update independently; changing one must not reset the
  other.
- Network services are intended for a trusted local network and must not be
  exposed to the public internet.

References:

- [OscGoesBrrr getting started](https://github.com/OscToys/osc.toys/blob/main/docs/20-getting-started.mdx)
- [Buttplug WebSocket Device Manager](https://buttplug.io/docs/dev-guide/inflating-buttplug/devices/websocket-device-manager/)
- [Buttplug device control](https://buttplug.io/docs/dev-guide/writing-buttplug-applications/device-control/)
