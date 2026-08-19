# Stage 1 wiring and electrical checks

## Connections

| ESP32 | Connects to | Notes |
|---|---|---|
| 3.3 V | OLED VCC | Use 3.3 V for predictable I²C logic levels. |
| GND | OLED GND | Common ground. |
| GPIO 21 | OLED SDA | I²C data. |
| GPIO 22 | OLED SCL | I²C clock. |
| GPIO 25 | Left module IN | Control signal only. |
| GPIO 26 | Right module IN | Control signal only. |
| GND | Both module GND pins | The ESP32 and motor supply must share ground. |
| Regulated motor supply + | Both module VCC pins | Use the voltage specified for the exact modules. |

Do **not** connect a motor or module VCC pin to GPIO 25 or GPIO 26. Those pins
only provide control signals.

## Checks required before connecting motor power

The description “DC 5V 9000 RPM vibration motor module” is not enough to identify
the input circuit, regulator, flyback protection, current draw, or logic
threshold. A product schematic, a clear board/component identification, or
measurements are needed before calling the arrangement electrically verified.

1. Find the exact listing/datasheet or identify the driver component on the
   board. Confirm the module supply range includes the intended motor voltage.
2. Confirm that a 3.3 V signal on `IN` is specified as HIGH. If it is not
   specified, test one module on the bench with a current-limited supply before
   attaching it to the ESP32. A logic-level buffer or transistor stage may be
   required if 3.3 V is not a valid HIGH.
3. Measure one module's running and startup/stall current at the intended supply
   voltage. Size the supply for both motors starting simultaneously, with margin.
4. With motor power disconnected, upload the sketch and measure GPIO 25 and 26.
   They should alternate between approximately 0 V and 3.3 V in the documented
   sequence.
5. Initially test one motor module, then the other, then both. Stop immediately
   if the ESP32 resets, the OLED glitches, or anything heats up.

## Power guidance

The development board's `5V`/`VIN` header is often tied to USB 5 V, but its safe
output current depends on the exact board layout, USB source, protection parts,
and cable. Do not treat it as an automatically approved two-motor supply.

A separate regulated motor supply is the conservative bench-test choice. Join
its negative/ground to ESP32 GND. Do not join its positive rail to USB 5 V unless
the development board documentation explicitly permits that power topology;
otherwise two supplies can back-feed one another.

Add local supply decoupling near the modules if their documentation recommends
it. Motor noise or brownouts should be corrected in the power/wiring design, not
hidden in software.

## Active level

The firmware assumes `IN = HIGH` means motor on. If the exact module is verified
as active LOW, change `kMotorOn` to `LOW` and `kMotorOff` to `HIGH` before
connecting motor power.
