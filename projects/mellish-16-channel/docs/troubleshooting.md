# Troubleshooting

| Symptom | Check |
|---|---|
| OLED/PCA missing | GPIO21/22, common ground, `0x3C`/`0x40`, 3.3 V logic VCC. |
| Resets when motors start | Supply capacity, distribution, decoupling, common-ground topology. |
| Motor will not start at 1 | Calibrate only that channel's `motorMinimumPWM8`. |
| Motor runs at 0 | Disconnect motor power; inspect driver polarity/wiring. Firmware writes zero. |
| Intiface cannot find it | Expected until the custom handler/config ships; do not rename it `LVS-Edge`. |
| BLE command rejected | Use `Vibrate1:10;`…`Vibrate16:10;`, 0–20, with semicolon. |
| TEST interruption | Send `STOP`; it forces every output off. |
| Sag/chatter | Stop `ALL`; return to sequential tests and use a regulated ~5 V / 3 A supply. |
