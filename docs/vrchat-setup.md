# VRChat, OscGoesBrrr, and avatar setup for later stages

Stage 1 does not use VRChat, OGB, Intiface, or Wi-Fi. These notes record the
validated design constraints for later stages.

The selected route is described in [architecture.md](architecture.md). OGB, not
the ESP32, receives VRChat OSC. OGB then controls the ESP32 through
Buttplug/Intiface.

- Avatar OSC addresses use `/avatar/parameters/<name>`, so the proposed paths
  are `/avatar/parameters/Haptic_LeftEar` and
  `/avatar/parameters/Haptic_RightEar`.
- VRChat avatar OSC values can be Bool, Int, or Float. Bool is sufficient for the
  first on/off prototype; Float is a sensible later choice for `0.0–1.0`
  intensity.
- VRChat normally receives OSC on UDP 9000 and sends on UDP 9001.
- The default outgoing target is localhost (`127.0.0.1`). That suits OGB running
  on the same PC and means the earlier idea of sending VRChat OSC directly to
  the ESP32 is no longer required.
- OSC uses UDP and has no handshake or delivery guarantee. Later firmware should
  implement a short safety timeout that turns motors off if expected updates
  stop, rather than leaving the last received ON value active indefinitely.
- Avatar parameter names and OSC addresses are case-sensitive in practice; keep
  spelling and capitalization identical throughout the avatar and firmware.

OGB's normal setup is based around SPS-compatible avatar contacts and then links
detected haptic sources to devices. Custom left/right ear Contact Receivers can
still be used, but their exact parameter naming, OGB source visibility, and
per-feature binding must be verified in OGB rather than assuming that arbitrary
`Haptic_LeftEar`/`Haptic_RightEar` values are automatically recognized.

Contact Receiver configuration, expression-parameter budgeting, and left/right
binding will be added only after the ESP32 appears in Intiface and both features
can be controlled independently from an Intiface test.

Official references:

- [VRChat OSC Overview](https://docs.vrchat.com/docs/osc-overview)
- [VRChat OSC Avatar Parameters](https://docs.vrchat.com/docs/osc-avatar-parameters)
- [VRChat OSC DIY](https://docs.vrchat.com/docs/osc-diy)
- [OscGoesBrrr getting started](https://github.com/OscToys/osc.toys/blob/main/docs/20-getting-started.mdx)
