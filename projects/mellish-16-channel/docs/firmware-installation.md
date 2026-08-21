# Firmware installation

1. Install Arduino IDE 2.x and Espressif's **esp32** board package.
2. Install **Adafruit GFX Library**, **Adafruit SSD1306**, and **Adafruit PWM
   Servo Driver Library**. Adafruit BusIO is a transitive dependency.
3. Open `Mellish_16_Channel_Experimental.ino` in its matching sketch folder.
4. Select **ESP32 Dev Module**, select the port, and upload.
5. At 115200 baud, with motor power disconnected, verify OLED `0x3C`, PCA9685
   `0x40`, and the startup all-off log.

BLE comes with the ESP32 core. There are no Wi-Fi credentials or secrets.
Verified library versions were Adafruit GFX 1.12.6, SSD1306 2.5.17, PWM Servo
Driver 3.0.3, and BusIO 1.17.4. Other compatible releases may work. Exact core
2.x/3.x compile results are recorded in the testing document.
