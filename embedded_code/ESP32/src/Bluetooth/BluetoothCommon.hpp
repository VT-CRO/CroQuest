// BluetoothCommon.hpp

#pragma once

#include <Arduino.h>
#include <NimBLEUUID.h>
#include <esp_system.h>
#include <string>

// ###################### Global Declarations #####################
extern std::string BLE_NAME_PREFIX;
extern NimBLEUUID SERVICE_UUID;
extern NimBLEUUID CHARACTERISTIC_UUID;

// ###################### Makes Unique Identifier(UUID) ######################
void initializeBluetoothIdentifiers();
std::string generate6DigitCode();