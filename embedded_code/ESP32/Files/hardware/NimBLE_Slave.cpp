// NimBLE_Slave.cpp

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <cstdio>

#include <SPI.h>
#include <TFT_eSPI.h>

const char *DEVICE_NAME = "CroQuest_ESP";
const std::string ACCESS_CODE = "code:826491";

void printMessage(const String &text);

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);

  printMessage("Starting BLE advertiser " + String(DEVICE_NAME) + " ...");
  Serial.println("Starting BLE advertiser " + String(DEVICE_NAME) + " ...");
  delay(3000);

  // Initialize BLE device
  NimBLEDevice::init(DEVICE_NAME);

  // Create BLE server
  NimBLEServer *pServer = NimBLEDevice::createServer();

  // Set advertising payload
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData adData;
  adData.setName(DEVICE_NAME);

  // Set manufacturer data with access code
  adData.setManufacturerData(std::string("\x01\x02") + ACCESS_CODE);
  pAdvertising->setAdvertisementData(adData);

  // Start advertising
  pAdvertising->start();

  printMessage("Advertising with code:\n" + String(ACCESS_CODE.c_str()));
  Serial.println("Advertising with code:\n" + String(ACCESS_CODE.c_str()));
}

void loop() {
  // idle
  delay(1000);
}

// ###################### Print Message ######################
void printMessage(const String &text) {
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);

  int y = 10;
  int lineHeight = 20;
  int idx = 0;

  while (idx < text.length()) {
    int lineEnd = text.indexOf('\n', idx);
    if (lineEnd == -1)
      lineEnd = text.length();

    String line = text.substring(idx, lineEnd);
    tft.drawString(line, 10, y);
    y += lineHeight;
    idx = lineEnd + 1;
  }
}
