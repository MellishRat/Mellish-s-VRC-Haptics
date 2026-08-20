/*
  Mellish VRC Haptics - ESP32 Lovense Edge BLE test firmware

  Intiface on the phone discovers this ESP32 during a normal Bluetooth scan.
  No ESP32 Wi-Fi settings or custom WebSocket-device entry are required.

  Hardware: SSD1306 128x32 (SDA 21, SCL 22, 0x3C), Motor 1 GPIO25,
  Motor 2 GPIO26. Drive motors through suitable driver modules with common GND.

  Libraries: Adafruit SSD1306 and Adafruit GFX. BLE support is included in the
  Arduino ESP32 board package. Board profile: ESP32 Dev Module.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
#if __has_include(<esp_arduino_version.h>)
  #include <esp_arduino_version.h>
#endif

// Keep the Lovense-compatible name in the primary advertisement. The service
// UUID is placed in the scan response, matching the verified BLE radio test.
const char* BLE_DEVICE_NAME = "LVS-Edge";
const char* DEVICE_ADDRESS  = "MELLISH-HAPTICS-001";

// Use a stable, locally administered address that differs from the address
// used by the earlier BLE radio-test sketch. This prevents Android from
// reusing that sketch's bonded GATT service cache.
uint8_t HAPTICS_BASE_MAC[6] = {0x32, 0x4D, 0x56, 0x48, 0x01, 0x01};

// Lovense P/Edge-family BLE UART service. Intiface writes commands to 0002
// and subscribes to responses from 0003.
#define LOVENSE_SERVICE_UUID "50300001-0023-4bd4-bbd5-a6920e4c5653"
#define LOVENSE_TX_UUID      "50300002-0023-4bd4-bbd5-a6920e4c5653"
#define LOVENSE_RX_UUID      "50300003-0023-4bd4-bbd5-a6920e4c5653"

constexpr uint8_t OLED_SDA = 21, OLED_SCL = 22, OLED_ADDRESS = 0x3C;
constexpr uint8_t MOTOR1_PIN = 25, MOTOR2_PIN = 26;
const uint8_t MOTOR1_MIN_PWM = 70;
const uint8_t MOTOR2_MIN_PWM = 75;
constexpr uint32_t PWM_FREQUENCY = 20000;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint8_t MOTOR1_CHANNEL = 0, MOTOR2_CHANNEL = 1; // Core 2.x

Adafruit_SSD1306 display(128, 32, &Wire, -1);
BLECharacteristic* responseCharacteristic = nullptr;
volatile bool bleConnected = false, restartAdvertising = false;
volatile bool displayDirty = true;
volatile uint8_t motor1Level = 0, motor2Level = 0;
bool oledAvailable = false;
unsigned long lastDisplayUpdate = 0;

void writeMotor1(uint8_t pwm) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(MOTOR1_PIN, pwm);
#else
  ledcWrite(MOTOR1_CHANNEL, pwm);
#endif
}

void writeMotor2(uint8_t pwm) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(MOTOR2_PIN, pwm);
#else
  ledcWrite(MOTOR2_CHANNEL, pwm);
#endif
}

uint8_t lovenseToPWM(int level, uint8_t minimumPWM) {
  level = constrain(level, 0, 20);
  return level == 0 ? 0 : (uint8_t)map(level, 1, 20, minimumPWM, 255);
}

void setMotor1Level(int level) {
  level = constrain(level, 0, 20);
  motor1Level = (uint8_t)level;
  uint8_t pwm = lovenseToPWM(level, MOTOR1_MIN_PWM);
  writeMotor1(pwm);
  displayDirty = true;
  Serial.printf("Motor 1: Level=%d PWM=%u Min=%u\n", level, pwm, MOTOR1_MIN_PWM);
}

void setMotor2Level(int level) {
  level = constrain(level, 0, 20);
  motor2Level = (uint8_t)level;
  uint8_t pwm = lovenseToPWM(level, MOTOR2_MIN_PWM);
  writeMotor2(pwm);
  displayDirty = true;
  Serial.printf("Motor 2: Level=%d PWM=%u Min=%u\n", level, pwm, MOTOR2_MIN_PWM);
}

void stopAllMotors() { setMotor1Level(0); setMotor2Level(0); }

void setupPWM() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  if (!ledcAttach(MOTOR1_PIN, PWM_FREQUENCY, PWM_RESOLUTION))
    Serial.println("ERROR: PWM attach failed on GPIO25");
  if (!ledcAttach(MOTOR2_PIN, PWM_FREQUENCY, PWM_RESOLUTION))
    Serial.println("ERROR: PWM attach failed on GPIO26");
#else
  ledcSetup(MOTOR1_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(MOTOR2_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(MOTOR1_PIN, MOTOR1_CHANNEL);
  ledcAttachPin(MOTOR2_PIN, MOTOR2_CHANNEL);
#endif
  writeMotor1(0); writeMotor2(0);
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
  display.setCursor(0, 0);
  display.print(bleConnected ? "Lovense Edge: READY" : "BLE: WAITING/SCAN");
  display.setCursor(0, 10); display.print("M1"); drawBar(18, 10, m1);
  display.setCursor(99, 10); if (m1 < 10) display.print(' '); display.print(m1);
  display.setCursor(0, 22); display.print("M2"); drawBar(18, 22, m2);
  display.setCursor(99, 22); if (m2 < 10) display.print(' '); display.print(m2);
  display.display();
}

void sendResponse(const String& response) {
  if (!responseCharacteristic || !bleConnected || !response.length()) return;
  responseCharacteristic->setValue((uint8_t*)response.c_str(), response.length());
  responseCharacteristic->notify();
  Serial.print("BLE TX: "); Serial.println(response);
}

bool extractLevel(const String& command, int& level) {
  int colon = command.indexOf(':'), semi = command.indexOf(';', colon + 1);
  if (colon < 0 || semi <= colon + 1) return false;
  level = constrain(command.substring(colon + 1, semi).toInt(), 0, 20);
  return true;
}

void processCommand(String command) {
  command.trim(); if (!command.length()) return;
  Serial.print("BLE RX: "); Serial.println(command);
  if (command == "DeviceType;") {
    String response = "P:1:"; response += DEVICE_ADDRESS; response += ';';
    sendResponse(response); return;
  }
  if (command == "Battery;") { sendResponse("100;"); return; }
  if (command == "Status:1;") { sendResponse("2;"); return; }
  int level = 0;
  if (command.startsWith("Vibrate1:") && extractLevel(command, level)) {
    setMotor1Level(level); return;
  }
  if (command.startsWith("Vibrate2:") && extractLevel(command, level)) {
    setMotor2Level(level); return;
  }
  if (command.startsWith("Vibrate:") && extractLevel(command, level)) {
    setMotor1Level(level); setMotor2Level(level); return;
  }
  if (command == "PowerOff;") { stopAllMotors(); sendResponse("OK;"); return; }
  Serial.print("Unhandled Lovense command: "); Serial.println(command);
}

void processPayload(const uint8_t* data, size_t length) {
  if (!data || !length) return;
  String payload; payload.reserve(length + 1);
  for (size_t i = 0; i < length; ++i) payload += (char)data[i];
  int start = 0;
  while (start < payload.length()) {
    int semi = payload.indexOf(';', start);
    if (semi < 0) {
      String remainder = payload.substring(start); remainder.trim();
      if (remainder.length()) { Serial.print("Incomplete BLE command: "); Serial.println(remainder); }
      break;
    }
    processCommand(payload.substring(start, semi + 1)); start = semi + 1;
  }
}

class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    processPayload(characteristic->getData(), characteristic->getLength());
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    bleConnected = true; displayDirty = true;
    Serial.println("BLE CONNECTED to Intiface");
  }
  void onDisconnect(BLEServer*) override {
    bleConnected = false; restartAdvertising = true; displayDirty = true;
    stopAllMotors(); Serial.println("BLE DISCONNECTED - motors stopped");
  }
};

void setupBLE() {
  esp_base_mac_addr_set(HAPTICS_BASE_MAC);
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService* service = server->createService(LOVENSE_SERVICE_UUID);
  BLECharacteristic* commands = service->createCharacteristic(
    LOVENSE_TX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  commands->setCallbacks(new WriteCallbacks());
  responseCharacteristic = service->createCharacteristic(
    LOVENSE_RX_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  responseCharacteristic->addDescriptor(new BLE2902());
  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x06); // General discoverable, no classic BT.
  advertisementData.setName(BLE_DEVICE_NAME);
  advertising->setAdvertisementData(advertisementData);

  BLEAdvertisementData scanResponse;
  scanResponse.setCompleteServices(BLEUUID(LOVENSE_SERVICE_UUID));
  advertising->setScanResponseData(scanResponse);
  BLEDevice::startAdvertising();
  Serial.print("BLE advertising as: "); Serial.println(BLE_DEVICE_NAME);
  Serial.println("Start a normal device scan in Intiface on the phone.");
}

void setup() {
  Serial.begin(115200); delay(300);
  Serial.println("\nMellish VRC Haptics - Lovense Edge BLE Test");
  pinMode(MOTOR1_PIN, OUTPUT); pinMode(MOTOR2_PIN, OUTPUT);
  setupPWM(); stopAllMotors();
  Wire.begin(OLED_SDA, OLED_SCL);
  oledAvailable = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!oledAvailable) Serial.println("WARNING: SSD1306 OLED not detected at 0x3C");
  else {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0); display.println("Mellish VRC Haptics");
    display.println("Starting BLE..."); display.display();
  }
  setupBLE(); updateDisplay(true);
}

void loop() {
  if (restartAdvertising) {
    restartAdvertising = false; delay(250); BLEDevice::startAdvertising();
    Serial.println("BLE advertising restarted");
  }
  updateDisplay(); delay(5);
}
