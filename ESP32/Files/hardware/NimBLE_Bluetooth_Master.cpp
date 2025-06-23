#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ==================== Config ====================
const std::string TARGET_CODE = "code:060970";
bool foundDevice = false;
NimBLEAdvertisedDevice *matchedDevice = nullptr;

TFT_eSPI tft = TFT_eSPI();

void printMessage(const String &text) {
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

// ==================== Scan Callback ====================
class MyScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
    std::string name = advertisedDevice->getName();
    std::string manuData = advertisedDevice->getManufacturerData();

    Serial.print("Found ");
    Serial.print(name.c_str());
    Serial.print(", data: ");
    Serial.println(manuData.c_str());

    if (manuData.find(TARGET_CODE) != std::string::npos) {
      Serial.println("✅ MATCH FOUND!");
      printMessage("✅ MATCH FOUND!");

      NimBLEDevice::getScan()->stop();

      matchedDevice =
          new NimBLEAdvertisedDevice(*advertisedDevice); // clone device
      foundDevice = true;
    }
  }
};

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(3);
  printMessage("Scanning for device...");

  NimBLEDevice::init("ESP32_Master");

  NimBLEScan *scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new MyScanCallbacks());
  scanner->setActiveScan(true);
  scanner->start(0); // Continuous scan

  Serial.println("BLE scan started.");
  delay(1000);
}

// ==================== Loop ====================
void loop() {
  if (!foundDevice) {
    Serial.println("🔍 Still scanning...");
    delay(1000);
    return;
  }

  // === CONNECT ===
  printMessage("Connecting...");
  Serial.println("🔗 Connecting to device...");
  delay(1000);

  NimBLEClient *client = NimBLEDevice::createClient();
  if (client->connect(matchedDevice)) {
    Serial.println("✅ Connected to device");
    delay(1000);

    NimBLERemoteService *service =
        client->getService("12345678-1234-1234-1234-123456789abc");
    if (service) {
      NimBLERemoteCharacteristic *charac =
          service->getCharacteristic("abcdefab-1234-1234-1234-abcdefabcdef");
      if (charac && charac->canRead()) {
        std::string response = charac->readValue();
        Serial.print("📥 Received: ");
        Serial.println(response.c_str());

        if (response.find("ACK") != std::string::npos) {
          printMessage("Connected!\nACK received.");
        } else {
          printMessage("Wrong response.");
        }
      } else {
        printMessage("No readable charac.");
      }
    } else {
      printMessage("No service.");
    }

    delay(3000);
    client->disconnect();
  } else {
    printMessage("Connect failed.");
    Serial.println("❌ Failed to connect");
  }

  // Freeze and stop further attempts
  while (true)
    delay(1000);
}
