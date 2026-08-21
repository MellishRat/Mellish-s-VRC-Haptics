/*
  PCA9685 Motor Bench Test

  Standalone wiring diagnostic for ESP32 + PCA9685 + driven motor modules.
  No BLE, OLED, Wi-Fi, or Intiface code is included.

  Serial Monitor: 115200 baud, Newline or Both NL & CR.
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr uint8_t CHANNEL_COUNT = 16;
constexpr uint16_t PWM_FREQUENCY_HZ = 1000;
constexpr uint16_t PWM_MAX = 4095;
constexpr uint32_t TEST_ON_MS = 1000;
constexpr uint32_t TEST_GAP_MS = 500;

Adafruit_PWMServoDriver pca(PCA9685_ADDRESS);
uint16_t channelPWM[CHANNEL_COUNT] = {0};
String serialLine;

enum TestState : uint8_t { TEST_IDLE, TEST_ON, TEST_GAP };
TestState testState = TEST_IDLE;
uint8_t testChannel = 0;
uint32_t testDeadline = 0;

bool detectPCA9685() {
  Wire.beginTransmission(PCA9685_ADDRESS);
  return Wire.endTransmission() == 0;
}

void setChannel(uint8_t channel, uint16_t value) {
  if (channel >= CHANNEL_COUNT) return;
  value = constrain(value, 0, PWM_MAX);

  // setPin handles the PCA9685 special fully-off and fully-on bits, making
  // this sketch useful for measuring a definite 0 V / logic-high condition.
  pca.setPin(channel, value, false);
  channelPWM[channel] = value;
  Serial.printf("CHANNEL motor=%u pca=%u pwm=%u\n", channel + 1, channel, value);
}

void allOff(const char* reason) {
  for (uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel) {
    pca.setPin(channel, 0, false);
    channelPWM[channel] = 0;
  }
  Serial.print("ALL OFF: ");
  Serial.println(reason);
}

void stopTest(const char* reason) {
  testState = TEST_IDLE;
  allOff(reason);
}

void startTest() {
  allOff("TEST start");
  testChannel = 0;
  testState = TEST_ON;
  setChannel(testChannel, PWM_MAX);
  testDeadline = millis() + TEST_ON_MS;
  Serial.println("TEST: each channel is fully ON for 1 second; only one channel is active");
}

void updateTest() {
  if (testState == TEST_IDLE || (int32_t)(millis() - testDeadline) < 0) return;

  if (testState == TEST_ON) {
    setChannel(testChannel, 0);
    testState = TEST_GAP;
    testDeadline = millis() + TEST_GAP_MS;
    return;
  }

  ++testChannel;
  if (testChannel >= CHANNEL_COUNT) {
    stopTest("TEST complete");
    return;
  }

  testState = TEST_ON;
  setChannel(testChannel, PWM_MAX);
  testDeadline = millis() + TEST_ON_MS;
}

void printStatus() {
  Serial.printf("PCA9685 0x40: %s\n", detectPCA9685() ? "DETECTED" : "NOT DETECTED");
  Serial.printf("I2C SDA=GPIO%u SCL=GPIO%u PWM=%u Hz\n", SDA_PIN, SCL_PIN, PWM_FREQUENCY_HZ);
  for (uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel) {
    Serial.printf("  motor=%u pca=%u pwm=%u\n", channel + 1, channel, channelPWM[channel]);
  }
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  CH <1-16> <0-4095>  Set one raw PCA9685 channel");
  Serial.println("  TEST                 Fully test channels 1-16 sequentially");
  Serial.println("  OFF                  Stop test and force all channels off");
  Serial.println("  STATUS                Detect controller and print channel states");
  Serial.println("Examples: CH 1 4095   |   CH 1 2048   |   OFF");
}

void processCommand(String command) {
  command.trim();
  command.toUpperCase();
  if (command.isEmpty()) return;

  if (command == "TEST") {
    startTest();
    return;
  }
  if (command == "OFF" || command == "STOP") {
    stopTest("Serial command");
    return;
  }
  if (command == "STATUS") {
    printStatus();
    return;
  }
  if (command == "HELP") {
    printHelp();
    return;
  }

  int motor = 0;
  int value = 0;
  char extra = 0;
  if (sscanf(command.c_str(), "CH %d %d %c", &motor, &value, &extra) == 2 &&
      motor >= 1 && motor <= CHANNEL_COUNT && value >= 0 && value <= PWM_MAX) {
    if (testState != TEST_IDLE) stopTest("manual channel command");
    setChannel((uint8_t)(motor - 1), (uint16_t)value);
    return;
  }

  Serial.println("REJECTED. Use CH <1-16> <0-4095>, TEST, OFF, STATUS, or HELP.");
}

void handleSerial() {
  while (Serial.available()) {
    const char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      if (!serialLine.isEmpty()) processCommand(serialLine);
      serialLine = "";
    } else if (serialLine.length() < 80) {
      serialLine += c;
    } else {
      serialLine = "";
      Serial.println("REJECTED: serial line too long");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nPCA9685 Motor Bench Test");
  Serial.println("WARNING: test one motor at a time; use common ground and a suitable 5 V motor supply.");
  Serial.println("Wiring: PCA VCC=3.3 V, V+=5 V, OE=GND, SDA=21, SCL=22.");

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.printf("PCA9685 at 0x40: %s\n", detectPCA9685() ? "DETECTED" : "NOT DETECTED");
  pca.begin();
  pca.setPWMFreq(PWM_FREQUENCY_HZ);
  allOff("startup safety");
  printHelp();
}

void loop() {
  handleSerial();
  updateTest();
  delay(2);
}
