"""Host-side contract checks for the Arduino firmware (no hardware required)."""
import re
import unittest
from pathlib import Path

SKETCH = (Path(__file__).parents[1] / "firmware" /
          "Mellish_16_Channel_Experimental" /
          "Mellish_16_Channel_Experimental.ino")
SOURCE = SKETCH.read_text(encoding="utf-8")


def pwm(level: int, minimum8: int) -> int:
    if level <= 0:
        return 0
    if level >= 20:
        return 4095
    minimum12 = (minimum8 * 4095 + 127) // 255
    return minimum12 + ((level - 1) * (4095 - minimum12)) // 19


def parse_stream(fragments):
    buffer, commands = "", []
    for fragment in fragments:
        for char in fragment:
            if char == ";":
                commands.append(buffer.strip())
                buffer = ""
            elif char not in "\r\n":
                buffer += char
    return commands, buffer


class FirmwareContract(unittest.TestCase):
    def test_pwm_endpoints_and_calibration(self):
        self.assertEqual(pwm(0, 70), 0)
        self.assertEqual(pwm(20, 70), 4095)
        self.assertEqual(pwm(1, 70), (70 * 4095 + 127) // 255)
        self.assertLess(pwm(1, 70), pwm(2, 70))

    def test_fragmented_and_combined_stream(self):
        commands, remainder = parse_stream(
            [" Vibrate1:5;Vib", "rate16:20;\r\n Vibrate", "2:7"])
        self.assertEqual(commands, ["Vibrate1:5", "Vibrate16:20"])
        self.assertEqual(remainder.strip(), "Vibrate2:7")

    def test_mapping_and_arrays_have_16_entries(self):
        mapping = re.search(r"MOTOR_TO_PCA_CHANNEL\[MOTOR_COUNT\]=\{([^}]+)\}", SOURCE)
        levels = re.search(r"motorLevels\[MOTOR_COUNT\]=\{([^}]+)\}", SOURCE)
        self.assertEqual([int(x) for x in mapping.group(1).split(",")], list(range(16)))
        self.assertEqual(len([x for x in levels.group(1).split(",") if x.strip()]), 1)
        self.assertIn("uint8_t motorMinimumPWM8[MOTOR_COUNT]", SOURCE)

    def test_failsafes_and_callback_boundary_exist(self):
        self.assertIn('stopAllChannels("startup failsafe")', SOURCE)
        self.assertIn('stopAllChannels("BLE disconnect failsafe")', SOURCE)
        callback = SOURCE.split("class WriteCallbacks", 1)[1].split("class ServerCallbacks", 1)[0]
        self.assertNotIn("setPWM", callback)
        self.assertNotIn("display.", callback)

    def test_bounds_are_explicit(self):
        self.assertIn("channel<1||channel>MOTOR_COUNT", SOURCE)
        self.assertIn("level<0||level>LEVEL_MAX", SOURCE)
        self.assertIn("COMMAND_BUFFER_MAX", SOURCE)
        self.assertIn("BYTE_QUEUE_MAX", SOURCE)


if __name__ == "__main__":
    unittest.main()
