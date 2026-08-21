/* Mellish 16-Channel Haptic Controller - experimental custom BLE firmware. */
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

const char* BLE_DEVICE_NAME = "MELLISH-16CH";
const char* DEVICE_ADDRESS = "MELLISH-HAPTICS-001";
uint8_t HAPTICS_BASE_MAC[6] = {0x32,0x4D,0x56,0x48,0x16,0x01};
#define MELLISH_SERVICE_UUID "8f780001-6e8c-4f2d-a4b8-51c8e3d21601"
#define MELLISH_TX_UUID      "8f780002-6e8c-4f2d-a4b8-51c8e3d21601"
#define MELLISH_RX_UUID      "8f780003-6e8c-4f2d-a4b8-51c8e3d21601"

constexpr uint8_t OLED_SDA=21, OLED_SCL=22, OLED_ADDRESS=0x3C, PCA9685_ADDRESS=0x40;
constexpr uint16_t PCA9685_FREQUENCY_HZ=1000, PWM_MAX=4095;
constexpr uint8_t MOTOR_COUNT=16, LEVEL_MAX=20;
constexpr size_t COMMAND_BUFFER_MAX=384, BYTE_QUEUE_MAX=512;
constexpr uint8_t TEST_LEVEL=10;
constexpr uint32_t TEST_ON_MS=350, TEST_GAP_MS=150;

const uint8_t MOTOR_TO_PCA_CHANNEL[MOTOR_COUNT]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t motorMinimumPWM8[MOTOR_COUNT]={70,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75};
uint8_t motorLevels[MOTOR_COUNT]={0};

Adafruit_SSD1306 display(128,32,&Wire,-1);
Adafruit_PWMServoDriver pwm(PCA9685_ADDRESS);
BLECharacteristic* responseCharacteristic=nullptr;
volatile bool bleConnected=false, restartAdvertising=false, disconnectStopRequested=false, queueOverflow=false;
portMUX_TYPE queueMux=portMUX_INITIALIZER_UNLOCKED;
char byteQueue[BYTE_QUEUE_MAX];
volatile size_t queueHead=0, queueTail=0;
bool oledAvailable=false, pcaAvailable=false, displayDirty=true;
uint32_t lastDisplayUpdate=0;
String receiveBuffer, serialLine;
enum TestPhase:uint8_t {TEST_IDLE,TEST_ON,TEST_GAP};
TestPhase testPhase=TEST_IDLE;
uint8_t testChannel=0;
uint32_t testDeadline=0;

uint16_t minimum8To12(uint8_t value){return (uint16_t)(((uint32_t)value*PWM_MAX+127U)/255U);}
uint16_t levelToPWM(uint8_t channel,int level){
  if(channel>=MOTOR_COUNT||level<=0)return 0;
  if(level>=LEVEL_MAX)return PWM_MAX;
  uint16_t minimum=minimum8To12(motorMinimumPWM8[channel]);
  return minimum+(uint16_t)(((uint32_t)(level-1)*(PWM_MAX-minimum))/(LEVEL_MAX-1));
}
void writeChannel(uint8_t channel,uint16_t value){
  if(channel<MOTOR_COUNT)pwm.setPWM(MOTOR_TO_PCA_CHANNEL[channel],0,constrain(value,0,PWM_MAX));
}
bool setMotorLevel(uint8_t channel,int level,const char* source="CMD"){
  if(channel>=MOTOR_COUNT||level<0||level>LEVEL_MAX){Serial.printf("REJECT %s channel=%u level=%d\n",source,channel+1,level);return false;}
  uint16_t value=levelToPWM(channel,level); motorLevels[channel]=(uint8_t)level; writeChannel(channel,value); displayDirty=true;
  Serial.printf("%s channel=%u pca=%u level=%d pwm=%u min8=%u\n",source,channel+1,MOTOR_TO_PCA_CHANNEL[channel],level,value,motorMinimumPWM8[channel]);
  return true;
}
void stopAllChannels(const char* reason="STOP"){
  for(uint8_t i=0;i<MOTOR_COUNT;++i){motorLevels[i]=0;writeChannel(i,0);} displayDirty=true;
  Serial.print("ALL CHANNELS OFF: ");Serial.println(reason);
}
void cancelTest(const char* reason){if(testPhase==TEST_IDLE)return;Serial.printf("TEST interrupted: %s\n",reason);testPhase=TEST_IDLE;stopAllChannels(reason);}
void startTest(){stopAllChannels("TEST start");testChannel=0;testPhase=TEST_ON;setMotorLevel(0,TEST_LEVEL,"TEST");testDeadline=millis()+TEST_ON_MS;Serial.println("TEST: one motor at a time; STOP interrupts");}
void updateTest(){
  if(testPhase==TEST_IDLE||(int32_t)(millis()-testDeadline)<0)return;
  if(testPhase==TEST_ON){setMotorLevel(testChannel,0,"TEST");testPhase=TEST_GAP;testDeadline=millis()+TEST_GAP_MS;return;}
  if(++testChannel>=MOTOR_COUNT){testPhase=TEST_IDLE;stopAllChannels("TEST complete");return;}
  testPhase=TEST_ON;setMotorLevel(testChannel,TEST_LEVEL,"TEST");testDeadline=millis()+TEST_ON_MS;
}
void sendResponse(const String& value){if(!responseCharacteristic||!bleConnected||value.isEmpty())return;responseCharacteristic->setValue((uint8_t*)value.c_str(),value.length());responseCharacteristic->notify();Serial.print("BLE TX: ");Serial.println(value);}
bool strictInteger(String text,int& value){text.trim();if(text.isEmpty())return false;size_t i=(text[0]=='+'||text[0]=='-')?1:0;if(i==text.length())return false;for(;i<text.length();++i)if(!isDigit(text[i]))return false;value=text.toInt();return true;}
bool levelAfterColon(const String& command,int& level){int colon=command.indexOf(':');return colon>=0&&strictInteger(command.substring(colon+1),level)&&level>=0&&level<=LEVEL_MAX;}
void processBLECommand(String command){
  command.trim();if(command.isEmpty())return;Serial.print("BLE RX: ");Serial.println(command);
  if(command.equalsIgnoreCase("Capabilities")){sendResponse("MELLISH16:16:0-20;");return;}
  if(command.equalsIgnoreCase("DeviceType")){String r="MELLISH16:16:";r+=DEVICE_ADDRESS;r+=';';sendResponse(r);return;}
  if(command.equalsIgnoreCase("PowerOff")||command.equalsIgnoreCase("STOP")){cancelTest("BLE stop");stopAllChannels("BLE stop");sendResponse("OK;");return;}
  int level=0;
  if(command.substring(0,8).equalsIgnoreCase("Vibrate:")){
    if(!levelAfterColon(command,level)){Serial.println("REJECT BLE malformed all-channel level");return;}
    cancelTest("BLE ALL");Serial.println("WARNING: all-channel command requires a suitable ~5 V / 3 A supply");
    for(uint8_t i=0;i<MOTOR_COUNT;++i)setMotorLevel(i,level,"BLE ALL");return;
  }
  if(command.length()>=10&&command.substring(0,7).equalsIgnoreCase("Vibrate")){
    int colon=command.indexOf(':'),channel=0;
    if(colon<=7||!strictInteger(command.substring(7,colon),channel)||!levelAfterColon(command,level)||channel<1||channel>MOTOR_COUNT){Serial.println("REJECT BLE expected Vibrate<1-16>:<0-20>;");return;}
    cancelTest("BLE CH");setMotorLevel((uint8_t)(channel-1),level,"BLE");return;
  }
  Serial.println("REJECT BLE unknown command");
}
void consumeByte(char c){
  if(c==';'){processBLECommand(receiveBuffer);receiveBuffer="";}
  else if(c!='\r'&&c!='\n'){if(receiveBuffer.length()>=COMMAND_BUFFER_MAX){receiveBuffer="";Serial.println("REJECT BLE command buffer overflow");}else receiveBuffer+=c;}
}
void drainQueue(){
  if(queueOverflow){portENTER_CRITICAL(&queueMux);queueTail=queueHead;queueOverflow=false;portEXIT_CRITICAL(&queueMux);receiveBuffer="";Serial.println("REJECT BLE byte queue overflow");}
  while(true){char c=0;bool available=false;portENTER_CRITICAL(&queueMux);if(queueTail!=queueHead){c=byteQueue[queueTail];queueTail=(queueTail+1)%BYTE_QUEUE_MAX;available=true;}portEXIT_CRITICAL(&queueMux);if(!available)break;consumeByte(c);}
}
class WriteCallbacks:public BLECharacteristicCallbacks{void onWrite(BLECharacteristic* characteristic)override{
  const uint8_t* data=characteristic->getData();size_t length=characteristic->getLength();portENTER_CRITICAL(&queueMux);
  for(size_t i=0;i<length;++i){size_t next=(queueHead+1)%BYTE_QUEUE_MAX;if(next==queueTail){queueOverflow=true;break;}byteQueue[queueHead]=(char)data[i];queueHead=next;}portEXIT_CRITICAL(&queueMux);
}};
class ServerCallbacks:public BLEServerCallbacks{
  void onConnect(BLEServer*)override{bleConnected=true;displayDirty=true;Serial.println("BLE CONNECTED");}
  void onDisconnect(BLEServer*)override{bleConnected=false;disconnectStopRequested=true;restartAdvertising=true;displayDirty=true;}
};
void setupBLE(){
  esp_base_mac_addr_set(HAPTICS_BASE_MAC);BLEDevice::init(BLE_DEVICE_NAME);BLEServer* server=BLEDevice::createServer();server->setCallbacks(new ServerCallbacks());
  BLEService* service=server->createService(MELLISH_SERVICE_UUID);BLECharacteristic* commands=service->createCharacteristic(MELLISH_TX_UUID,BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_WRITE_NR);commands->setCallbacks(new WriteCallbacks());
  responseCharacteristic=service->createCharacteristic(MELLISH_RX_UUID,BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);responseCharacteristic->addDescriptor(new BLE2902());service->start();
  BLEAdvertising* advertising=BLEDevice::getAdvertising();BLEAdvertisementData ad;ad.setFlags(0x06);ad.setName(BLE_DEVICE_NAME);advertising->setAdvertisementData(ad);BLEAdvertisementData scan;scan.setCompleteServices(BLEUUID(MELLISH_SERVICE_UUID));advertising->setScanResponseData(scan);BLEDevice::startAdvertising();
  Serial.print("BLE advertising custom device: ");Serial.println(BLE_DEVICE_NAME);
}
bool probeI2C(uint8_t address){Wire.beginTransmission(address);return Wire.endTransmission()==0;}
void updateDisplay(bool force=false){
  if(!oledAvailable)return;if(!force&&!displayDirty&&millis()-lastDisplayUpdate<500)return;if(!force&&millis()-lastDisplayUpdate<100)return;lastDisplayUpdate=millis();displayDirty=false;
  uint8_t active=0,highest=0;for(uint8_t level:motorLevels){if(level)++active;if(level>highest)highest=level;}
  display.clearDisplay();display.setTextSize(1);display.setTextColor(SSD1306_WHITE);display.setCursor(0,0);display.print(bleConnected?"BLE: CONNECTED":"BLE: WAITING");display.setCursor(0,11);display.printf("Active:%u Highest:%u",active,highest);display.setCursor(0,22);if(testPhase==TEST_IDLE)display.print(pcaAvailable?"PCA9685: READY":"PCA9685: MISSING");else display.printf("TEST channel: %u/16",testChannel+1);display.display();
}
void printStatus(){Serial.printf("STATUS BLE=%s PCA=%s TEST=%s\n",bleConnected?"connected":"waiting",pcaAvailable?"ready":"missing",testPhase==TEST_IDLE?"idle":"running");for(uint8_t i=0;i<MOTOR_COUNT;++i)Serial.printf(" motor=%u pca=%u level=%u pwm=%u min8=%u\n",i+1,MOTOR_TO_PCA_CHANNEL[i],motorLevels[i],levelToPWM(i,motorLevels[i]),motorMinimumPWM8[i]);}
void processSerialCommand(String command){
  command.trim();if(command.isEmpty())return;String upper=command;upper.toUpperCase();
  if(upper=="STOP"){cancelTest("Serial STOP");stopAllChannels("Serial STOP");return;}if(upper=="TEST"){startTest();return;}if(upper=="STATUS"){printStatus();return;}
  int channel=0,level=0;
  if(sscanf(upper.c_str(),"CH %d %d",&channel,&level)==2){if(channel<1||channel>MOTOR_COUNT||level<0||level>LEVEL_MAX){Serial.println("REJECT serial: CH <1-16> <0-20>");return;}cancelTest("Serial CH");setMotorLevel(channel-1,level,"SERIAL");return;}
  if(sscanf(upper.c_str(),"ALL %d",&level)==1){if(level<0||level>LEVEL_MAX){Serial.println("REJECT serial: ALL <0-20>");return;}cancelTest("Serial ALL");Serial.println("*** POWER WARNING: use regulated ~5 V / 3 A; never a sub-700 mA breadboard supply ***");for(uint8_t i=0;i<MOTOR_COUNT;++i)setMotorLevel(i,level,"SERIAL ALL");return;}
  Serial.println("Commands: STOP | TEST | CH <1-16> <0-20> | ALL <0-20> | STATUS");
}
void handleSerial(){while(Serial.available()){char c=(char)Serial.read();if(c=='\r'||c=='\n'){if(!serialLine.isEmpty())processSerialCommand(serialLine);serialLine="";}else if(serialLine.length()<80)serialLine+=c;else{serialLine="";Serial.println("REJECT serial line too long");}}}
void setup(){
  Serial.begin(115200);delay(300);Serial.println("\nMellish 16-Channel Haptic Controller - Experimental");Wire.begin(OLED_SDA,OLED_SCL);
  Serial.printf("I2C OLED 0x3C: %s\n",probeI2C(OLED_ADDRESS)?"detected":"not detected");pcaAvailable=probeI2C(PCA9685_ADDRESS);Serial.printf("I2C PCA9685 0x40: %s\n",pcaAvailable?"detected":"not detected");
  pwm.begin();pwm.setPWMFreq(PCA9685_FREQUENCY_HZ);stopAllChannels("startup failsafe");oledAvailable=display.begin(SSD1306_SWITCHCAPVCC,OLED_ADDRESS);if(!oledAvailable)Serial.println("WARNING: OLED initialization failed");setupBLE();updateDisplay(true);Serial.println("Ready: STOP | TEST | CH <1-16> <0-20> | ALL <0-20> | STATUS");
}
void loop(){
  if(disconnectStopRequested){disconnectStopRequested=false;cancelTest("BLE disconnect failsafe");stopAllChannels("BLE disconnect failsafe");receiveBuffer="";Serial.println("BLE DISCONNECTED - all channels stopped");}
  handleSerial();drainQueue();updateTest();if(restartAdvertising){restartAdvertising=false;BLEDevice::startAdvertising();Serial.println("BLE advertising restarted");}updateDisplay();delay(2);
}
