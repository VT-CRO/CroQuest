#include "BluetoothHost.hpp"
#include "ConnectionScreen.hpp"
#include "UUIDs.hpp"

// ===== Constructor =====
HostBLEServer::HostBLEServer(TFT_eSPI &display) : tft(display) {}

// ===== Start BLE & Scan =====
void HostBLEServer::begin() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(3);
  ConnectionScreen::showMessage("Scanning for device...");

  NimBLEDevice::init("ESP32_Master");
  NimBLEScan *scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new MyScanCallbacks(this));
  scanner->setActiveScan(true);
  scanner->start(0);

  Serial.println("BLE scan started.");
  delay(1000);
}

// ===== Scan Callback Impl =====
void HostBLEServer::MyScanCallbacks::onResult(
    const NimBLEAdvertisedDevice *advertisedDevice) {

  std::string name = advertisedDevice->getName();
  std::string manuData = advertisedDevice->getManufacturerData();

  Serial.print("Found ");
  Serial.print(name.c_str());
  Serial.print(", data: ");
  Serial.println(manuData.c_str());

  if (manuData.find(parent->targetCode) != std::string::npos) {
    Serial.println("✅ MATCH FOUND!");
    ConnectionScreen::showMessage("✅ MATCH FOUND!");

    NimBLEDevice::getScan()->stop();
    parent->matchedDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
    parent->foundDevice = true;
  }
}

// ===== Update Loop =====
void HostBLEServer::update() {
  if (!foundDevice) {
    Serial.println("🔍 Still scanning...");
    delay(1000);
    return;
  }

  ConnectionScreen::showMessage("Connecting...");
  Serial.println("🔗 Connecting to device...");
  delay(1000);

  NimBLEClient *client = NimBLEDevice::createClient();
  if (client->connect(matchedDevice)) {
    Serial.println("✅ Connected to device");
    delay(1000);

    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (service) {
      NimBLERemoteCharacteristic *charac =
          service->getCharacteristic(CHARACTERISTIC_UUID);
      if (charac && charac->canRead()) {
        std::string response = charac->readValue();
        Serial.print("📥 Received: ");
        Serial.println(response.c_str());

        if (response.find("ACK") != std::string::npos) {
          ConnectionScreen::showMessage("Connected!\nACK received.");
        } else {
          ConnectionScreen::showMessage("Slave sent: " +
                                        String(response.c_str()));
        }
      } else {
        ConnectionScreen::showMessage("No readable charac.");
      }
    } else {
      ConnectionScreen::showMessage("No service.");
    }

    delay(3000);
    client->disconnect();
  } else {
    ConnectionScreen::showMessage("Connect failed.");
    Serial.println("❌ Failed to connect");
  }

  while (true)
    delay(1000);
}
