/*
  Mellish 16-Channel Haptic Controller - Experimental

  PCA9685 PWM development firmware. Intiface still sees a Lovense Edge with
  two outputs: Vibrate1 controls PCA9685 channel 0 and Vibrate2 channel 1.
  Serial commands: TEST pulses channels 0-15; STOP switches all channels off.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_system.h>
#if __has_include(<esp_mac.h>)
  #include <esp_mac.h>
#else
  #include <esp_err.h>
  extern "C" esp_err_t esp_base_mac_addr_set(const uint8_t* mac);
#endif

const char* BLE_DEVICE_NAME = "LVS-Edge";
const char* DEVICE_ADDRESS  = "MELLISH-HAPTICS-001";
uint8_t HAPTICS_BASE_MAC[6] = {0x32, 0x4D, 0x56, 0x48, 0x01, 0x01};

#define LOVENSE_SERVICE_UUID "50300001-0023-4bd4-bbd5-a6920e4c5653"
#define LOVENSE_TX_UUID      "50300002-0023-4bd4-bbd5-a6920e4c5653"
#define LOVENSE_RX_UUID      "50300003-0023-4bd4-bbd5-a6920e4c5653"

constexpr uint8_t OLED_SDA = 21, OLED_SCL = 22, OLED_ADDRESS = 0x3C;
constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr uint8_t MOTOR1_CHANNEL = 0, MOTOR2_CHANNEL = 1;
constexpr uint8_t MOTOR_CHANNEL_COUNT = 16;
const uint8_t MOTOR1_MIN_PWM = 70;
const uint8_t MOTOR2_MIN_PWM = 75;
constexpr uint16_t PWM_MAX = 4095;
constexpr uint16_t TEST_PWM = 2048;
constexpr uint16_t TEST_PULSE_MS = 300;

Adafruit_SSD1306 display(128, 32, &Wire, -1);
Adafruit_PWMServoDriver pwm(PCA9685_ADDRESS);
BLECharacteristic* responseCharacteristic = nullptr;
volatile bool bleConnected = false, restartAdvertising = false;
volatile bool displayDirty = true;
volatile uint8_t motor1Level = 0, motor2Level = 0;
bool oledAvailable = false;
unsigned long lastDisplayUpdate = 0;
String serialCommand;

uint16_t lovenseToPWM(int level, uint8_t minimum8Bit) {
  level = constrain(level, 0, 20);
  if (level == 0) return 0;
  uint16_t minimum12Bit = map(minimum8Bit, 0, 255, 0, PWM_MAX);
  return (uint16_t)map(level, 1, 20, minimum12Bit, PWM_MAX);
}

void writeChannel(uint8_t channel, uint16_t level) {
  pwm.setPWM(channel, 0, constrain(level, 0, PWM_MAX));
}

void stopAllChannels() {
  for (uint8_t channel = 0; channel < MOTOR_CHANNEL_COUNT; ++channel)
    pwm.setPWM(channel, 0, 0);
  motor1Level = 0; motor2Level = 0; displayDirty = true;
  Serial.println("All PCA9685 channels OFF");
}

void setMotor1Level(int level) {
  level = constrain(level, 0, 20); motor1Level = (uint8_t)level;
  writeChannel(MOTOR1_CHANNEL, lovenseToPWM(level, MOTOR1_MIN_PWM));
  displayDirty = true;
}

void setMotor2Level(int level) {
  level = constrain(level, 0, 20); motor2Level = (uint8_t)level;
  writeChannel(MOTOR2_CHANNEL, lovenseToPWM(level, MOTOR2_MIN_PWM));
  displayDirty = true;
}

void runChannelTest() {
  stopAllChannels();
  Serial.println("TEST: pulsing PCA9685 channels 0-15");
  for (uint8_t channel = 0; channel < MOTOR_CHANNEL_COUNT; ++channel) {
    Serial.printf("TEST channel %u\n", channel);
    writeChannel(channel, TEST_PWM); delay(TEST_PULSE_MS); writeChannel(channel, 0);
    delay(100);
  }
  stopAllChannels(); Serial.println("TEST complete");
}

void drawBar(int x, int y, uint8_t level) {
  display.drawRect(x, y, 76, 8, SSD1306_WHITE);
  int fill = map(constrain(level, 0, 20), 0, 20, 0, 74);
  if (fill) display.fillRect(x + 1, y + 1, fill, 6, SSD1306_WHITE);
}

void updateDisplay(bool force = false) {
  if (!oledAvailable) return;
  if (!force && !displayDirty && millis() - lastDisplayUpdate < 500) return;
  if (!force && millis() - lastDisplayUpdate < 100) return;
  lastDisplayUpdate = millis(); displayDirty = false;
  uint8_t m1 = motor1Level, m2 = motor2Level;
  display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.print(bleConnected ? "PCA9685 BLE READY" : "PCA9685 WAITING");
  display.setCursor(0, 10); display.print("C0"); drawBar(18, 10, m1);
  display.setCursor(99, 10); if (m1 < 10) display.print(' '); display.print(m1);
  display.setCursor(0, 22); display.print("C1"); drawBar(18, 22, m2);
  display.setCursor(99, 22); if (m2 < 10) display.print(' '); display.print(m2);
  display.display();
}

void sendResponse(const String& response) {
  if (!responseCharacteristic || !bleConnected || !response.length()) return;
  responseCharacteristic->setValue((uint8_t*)response.c_str(), response.length());
  responseCharacteristic->notify();
}

bool extractLevel(const String& command, int& level) {
  int colon = command.indexOf(':'), semi = command.indexOf(';', colon + 1);
  if (colon < 0 || semi <= colon + 1) return false;
  level = constrain(command.substring(colon + 1, semi).toInt(), 0, 20); return true;
}

void processCommand(String command) {
  command.trim(); if (!command.length()) return;
  if (command == "DeviceType;") {
    String response = "P:1:"; response += DEVICE_ADDRESS; response += ';';
    sendResponse(response); return;
  }
  if (command == "Battery;") { sendResponse("100;"); return; }
  if (command == "Status:1;") { sendResponse("2;"); return; }
  int level = 0;
  if (command.startsWith("Vibrate1:") && extractLevel(command, level)) { setMotor1Level(level); return; }
  if (command.startsWith("Vibrate2:") && extractLevel(command, level)) { setMotor2Level(level); return; }
  if (command.startsWith("Vibrate:") && extractLevel(command, level)) {
    setMotor1Level(level); setMotor2Level(level); return;
  }
  if (command == "PowerOff;" || command == "STOP;") {
    stopAllChannels(); sendResponse("OK;"); return;
  }
}

void processPayload(const uint8_t* data, size_t length) {
  if (!data || !length) return;
  String payload; payload.reserve(length + 1);
  for (size_t i = 0; i < length; ++i) payload += (char)data[i];
  int start = 0;
  while (start < payload.length()) {
    int semi = payload.indexOf(';', start); if (semi < 0) break;
    processCommand(payload.substring(start, semi + 1)); start = semi + 1;
  }
}

class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    processPayload(characteristic->getData(), characteristic->getLength());
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override { bleConnected = true; displayDirty = true; }
  void onDisconnect(BLEServer*) override {
    bleConnected = false; restartAdvertising = true; displayDirty = true;
    stopAllChannels(); Serial.println("BLE DISCONNECTED - all channels stopped");
  }
};

void setupBLE() {
  esp_base_mac_addr_set(HAPTICS_BASE_MAC); BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer(); server->setCallbacks(new ServerCallbacks());
  BLEService* service = server->createService(LOVENSE_SERVICE_UUID);
  BLECharacteristic* commands = service->createCharacteristic(
    LOVENSE_TX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  commands->setCallbacks(new WriteCallbacks());
  responseCharacteristic = service->createCharacteristic(
    LOVENSE_RX_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  responseCharacteristic->addDescriptor(new BLE2902()); service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advertisementData; advertisementData.setFlags(0x06);
  advertisementData.setName(BLE_DEVICE_NAME); advertising->setAdvertisementData(advertisementData);
  BLEAdvertisementData scanResponse;
  scanResponse.setCompleteServices(BLEUUID(LOVENSE_SERVICE_UUID));
  advertising->setScanResponseData(scanResponse); BLEDevice::startAdvertising();
}

void handleSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      serialCommand.trim(); serialCommand.toUpperCase();
      if (serialCommand == "TEST") runChannelTest();
      else if (serialCommand == "STOP") stopAllChannels();
      else if (serialCommand.length()) Serial.println("Commands: TEST, STOP");
      serialCommand = "";
    } else serialCommand += c;
  }
}

void setup() {
  Serial.begin(115200); delay(300); Wire.begin(OLED_SDA, OLED_SCL);
  oledAvailable = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  pwm.begin(); pwm.setPWMFreq(1600); stopAllChannels();
  setupBLE(); updateDisplay(true);
  Serial.println("Experimental PCA9685 controller ready. Commands: TEST, STOP");
}

void loop() {
  handleSerial();
  if (restartAdvertising) {
    restartAdvertising = false; delay(250); BLEDevice::startAdvertising();
    Serial.println("BLE advertising restarted");
  }
  updateDisplay(); delay(5);
}
