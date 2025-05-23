// BluetoothCommon.cpp

#include "BluetoothCommon.hpp"

// ###################### Global Declarations #####################
std::string BLE_NAME_PREFIX = "CroQuest_ESP";
NimBLEUUID SERVICE_UUID("12345678-1234-1234-1234-123456789abc");
NimBLEUUID CHARACTERISTIC_UUID("abcdefab-1234-1234-1234-abcdefabcdef");

// ###################### Makes Unique Identifier(UUID) ######################
void initializeBluetoothIdentifiers() {
  // Get MAC address
  uint64_t chipId = ESP.getEfuseMac();
  char suffix[13];
  snprintf(suffix, sizeof(suffix), "%012llX", chipId);

  // Create unique BLE name and UUIDs
  BLE_NAME_PREFIX = "CroQuest_ESP_" + std::string(suffix);

  SERVICE_UUID =
      NimBLEUUID("12345678-1234-1234-" + std::string(suffix).substr(0, 4) +
                 "-" + std::string(suffix).substr(4, 12));

  CHARACTERISTIC_UUID =
      NimBLEUUID("abcdefab-1234-1234-" + std::string(suffix).substr(0, 4) +
                 "-" + std::string(suffix).substr(4, 12));
}

std::string generate6DigitCode() {
  int code = random(0, 1000000);                  // range: 0 to 999999
  char buffer[7];                                 // 6 digits + null terminator
  snprintf(buffer, sizeof(buffer), "%06d", code); // pad with leading zeros
  return std::string(buffer);
}
