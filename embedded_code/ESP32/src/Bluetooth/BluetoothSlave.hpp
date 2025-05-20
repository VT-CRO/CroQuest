#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// BLE Configuration
extern const char *DEVICE_NAME;
extern const std::string ACCESS_CODE;
extern TFT_eSPI tft;
extern NimBLECharacteristic *characteristic;
extern NimBLEAdvertising *pAdvertising;
