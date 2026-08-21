# Custom BLE protocol

| Item | Value |
|---|---|
| Name | `MELLISH-16CH` |
| Service | `8f780001-6e8c-4f2d-a4b8-51c8e3d21601` |
| Host write / TX | `8f780002-6e8c-4f2d-a4b8-51c8e3d21601` |
| Device read/notify / RX | `8f780003-6e8c-4f2d-a4b8-51c8e3d21601` |

UTF-8/ASCII commands end with `;`:

- `Vibrate1:<0-20>;` … `Vibrate16:<0-20>;` set one motor.
- `Vibrate:<0-20>;` sets all motors (project-defined; full-rated supply only).
- `STOP;` or `PowerOff;` stops all motors.
- `Capabilities;` replies `MELLISH16:16:0-20;`.
- `DeviceType;` replies `MELLISH16:16:MELLISH-HAPTICS-001;`.

CR/LF and surrounding whitespace are accepted. Writes may contain multiple
commands or command fragments. The bounded persistent buffer rejects malformed
values, out-of-range channels/levels, unknown commands, and overflow without
applying the invalid update. A future Buttplug handler maps feature indices 0–15
to protocol motors 1–16 and emits explicit zero commands on stop.
