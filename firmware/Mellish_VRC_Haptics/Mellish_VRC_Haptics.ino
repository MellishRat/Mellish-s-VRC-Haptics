/*
  Mellish's VRC Haptics - Stage 1 hardware test

  Test sequence: Left -> Right -> Both -> Off
  This sketch intentionally contains no Wi-Fi or OSC code.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace Hardware {
constexpr uint8_t kOledSda = 21;
constexpr uint8_t kOledScl = 22;
constexpr uint8_t kLeftMotor = 25;
constexpr uint8_t kRightMotor = 26;
constexpr uint8_t kOledAddress = 0x3C;
constexpr int8_t kOledReset = -1;
constexpr uint16_t kScreenWidth = 128;
constexpr uint16_t kScreenHeight = 32;

// Most 3-pin driver modules are active HIGH. Change both values if testing the
// exact module proves it is active LOW.
constexpr uint8_t kMotorOn = HIGH;
constexpr uint8_t kMotorOff = LOW;
}  // namespace Hardware

namespace Timing {
constexpr uint32_t kStepDurationMs = 1500;
}  // namespace Timing

Adafruit_SSD1306 display(Hardware::kScreenWidth, Hardware::kScreenHeight,
                         &Wire, Hardware::kOledReset);
bool displayAvailable = false;

struct TestStep {
  const char* label;
  bool leftOn;
  bool rightOn;
};

constexpr TestStep kTestSteps[] = {
    {"LEFT", true, false},
    {"RIGHT", false, true},
    {"BOTH", true, true},
    {"OFF", false, false},
};

constexpr size_t kStepCount = sizeof(kTestSteps) / sizeof(kTestSteps[0]);
size_t currentStep = 0;
uint32_t stepStartedAt = 0;

void setMotors(bool leftOn, bool rightOn) {
  digitalWrite(Hardware::kLeftMotor,
               leftOn ? Hardware::kMotorOn : Hardware::kMotorOff);
  digitalWrite(Hardware::kRightMotor,
               rightOn ? Hardware::kMotorOn : Hardware::kMotorOff);
}

void drawBar(int16_t x, int16_t y, bool active) {
  constexpr int16_t kWidth = 38;
  constexpr int16_t kHeight = 8;
  display.drawRect(x, y, kWidth, kHeight, SSD1306_WHITE);
  if (active) {
    display.fillRect(x + 2, y + 2, kWidth - 4, kHeight - 4, SSD1306_WHITE);
  }
}

void updateDisplay(const TestStep& step) {
  if (!displayAvailable) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("VRC HAPTICS  TEST:"));
  display.println(step.label);
  display.setCursor(0, 16);
  display.print(F("L"));
  drawBar(10, 15, step.leftOn);
  display.setCursor(65, 16);
  display.print(F("R"));
  drawBar(75, 15, step.rightOn);
  display.display();
}

void applyStep(size_t index) {
  const TestStep& step = kTestSteps[index];
  setMotors(step.leftOn, step.rightOn);
  updateDisplay(step);

  Serial.print(F("Test step: "));
  Serial.print(step.label);
  Serial.print(F(" | left="));
  Serial.print(step.leftOn ? F("ON") : F("OFF"));
  Serial.print(F(" right="));
  Serial.println(step.rightOn ? F("ON") : F("OFF"));
}

void setup() {
  Serial.begin(115200);

  // Establish the inactive level before making either pin an output. This
  // minimizes an unwanted motor pulse during startup.
  digitalWrite(Hardware::kLeftMotor, Hardware::kMotorOff);
  digitalWrite(Hardware::kRightMotor, Hardware::kMotorOff);
  pinMode(Hardware::kLeftMotor, OUTPUT);
  pinMode(Hardware::kRightMotor, OUTPUT);
  setMotors(false, false);

  Wire.begin(Hardware::kOledSda, Hardware::kOledScl);
  displayAvailable = display.begin(SSD1306_SWITCHCAPVCC,
                                   Hardware::kOledAddress);
  if (!displayAvailable) {
    Serial.println(F("OLED not found at 0x3C; motor test will continue."));
  }

  Serial.println(F("Mellish's VRC Haptics - Stage 1"));
  Serial.println(F("Sequence: LEFT -> RIGHT -> BOTH -> OFF"));
  currentStep = 0;
  applyStep(currentStep);
  stepStartedAt = millis();
}

void loop() {
  const uint32_t now = millis();
  if (now - stepStartedAt >= Timing::kStepDurationMs) {
    currentStep = (currentStep + 1) % kStepCount;
    applyStep(currentStep);
    stepStartedAt = now;
  }
}
