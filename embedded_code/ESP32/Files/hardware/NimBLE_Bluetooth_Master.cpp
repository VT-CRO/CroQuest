#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ==================== Config ====================
const std::string TARGET_CODE = "code:826491";
bool foundDevice = false;
bool scanStarted = false;

void printMessage(const String &text);

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ==================== Scan Callback ====================
class MyScanCallbacks : public NimBLEScanCallbacks {

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
    std::string name = advertisedDevice->getName();
    std::string manuData = advertisedDevice->getManufacturerData();

    printMessage("Found ");
    printMessage(name.c_str());
    printMessage(", data: ");
    printMessage(manuData.c_str());

    Serial.print("Found ");
    Serial.print(name.c_str());
    Serial.print(", data: ");
    Serial.println(manuData.c_str());

    if (manuData.find(TARGET_CODE) != std::string::npos) {
      printMessage("✅ MATCH FOUND!");
      Serial.println("✅ MATCH FOUND!");
      foundDevice = true;

      NimBLEDevice::getScan()->stop();
    }
  }
};

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);

  NimBLEDevice::init("ESP32_Master");

  NimBLEScan *scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new MyScanCallbacks());
  scanner->setActiveScan(true);
  scanner->start(0); // Scan continuously in background

  if (scanner->isScanning()) {
    Serial.println("✅ Scanner is running.");
  } else {
    Serial.println("❌ Scanner failed to start.");
  }

  scanStarted = true;
  Serial.println("BLE scan started.");
  printMessage("Scanning for device...");
}

// ==================== Loop ====================
void loop() {
  if (!foundDevice && NimBLEDevice::getScan()->isScanning()) {
    Serial.println("Still scanning...");
    delay(1000);
    return;
  }

  if (foundDevice) {
    printMessage("Target found.\nReady to connect.");
    Serial.println("Target found. Ready to connect.");

    foundDevice = false;
    scanStarted = false;

    delay(5000); // Optional: freeze screen
  }
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
