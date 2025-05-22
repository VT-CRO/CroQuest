// src/Bluetooth/BluetoothCentral.cpp

#include "BluetoothCentral.hpp"

#include "BluetoothCommon.hpp"
#include "ConnectionScreen.hpp"

BluetoothCentral::BluetoothCentral(TFT_eSPI &display) : tft(display) {}

// ####################################################################################################
//  Scan Callbacks
// ####################################################################################################
void BluetoothCentral::ScanCallbacks::onResult(
    const NimBLEAdvertisedDevice *advertisedDevice) {
  std::string name = advertisedDevice->getName();
  std::string manuData = advertisedDevice->getManufacturerData();

  Serial.printf("🔍 Found %s | Data: %s\n", name.c_str(), manuData.c_str());
  if (manuData.find(parent->targetCode) != std::string::npos) {
    Serial.println("✅ Match found!");
    parent->foundDevices.push_back(*advertisedDevice);
  }
}

// ###################### Start Scanning #####################
void BluetoothCentral::beginScan(const std::string &accessCode) {
  this->targetCode = accessCode;
  this->foundDevices.clear();

  NimBLEDevice::init("ESP32_Host");

  NimBLEScan *scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new ScanCallbacks(this));
  scanner->setActiveScan(true);
  scanner->start(5, false); // Scan for 5 seconds

  ConnectionScreen::showMessage("Scanning...\nAccess Code:\n" +
                                String(accessCode.c_str()));
}

// ###################### Keep Scanning for more Players #####################
void BluetoothCentral::scanAndConnectLoop() {
  Serial.println("🔄 Starting scan-and-connect loop...");

  ConnectionScreen::showMessage("Scanning for players...\nCode: " +
                                String(this->targetCode.c_str()));
  beginScan(this->targetCode);

  delay(5000);                  // wait for scan to complete
  BluetoothManager::stopScan(); // stop scanning explicitly

  if (foundDevices.empty()) {
    Serial.println("⚠️ No matching devices found.");
    ConnectionScreen::showMessage("No devices found.\nTry again.");
  } else {
    Serial.printf("🔍 Scan complete. Found %d device(s).\n",
                  foundDevices.size());
    ConnectionScreen::showMessage(
        "Connecting to " + String(foundDevices.size()) + " player(s)...");
    connectToDevices();
  }

  Serial.println("✅ Scan-and-connect loop complete.");
}

// ###################### Connect to All Devices #####################
void BluetoothCentral::connectToDevices() {
  this->connectedClients.clear();

  for (auto &device : this->foundDevices) {
    NimBLEClient *client = NimBLEDevice::createClient();
    if (client->connect(&device)) {
      Serial.printf("🔗 Connected to %s\n", device.getName().c_str());
      this->connectedClients.push_back(client);
    } else {
      Serial.println("❌ Failed to connect to device.");
      NimBLEDevice::deleteClient(client); // ✅ Safe deletion
    }
  }

  ConnectionScreen::showMessage(
      "Connected to " + String(this->connectedClients.size()) + " player(s)");
}

// ###################### Poll Devices for Incoming Data #####################
void BluetoothCentral::pollDevices() {
  for (auto *client : this->connectedClients) {
    if (!client || !client->isConnected())
      continue;

    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (!service)
      continue;

    NimBLERemoteCharacteristic *charac =
        service->getCharacteristic(CHARACTERISTIC_UUID);
    if (!charac || !charac->canRead())
      continue;

    std::string data = charac->readValue();
    if (!data.empty()) {
      Serial.printf("📥 [%s] %s\n", client->getPeerAddress().toString().c_str(),
                    data.c_str());
      // TODO: hand off to game logic
    }
  }
}

// ###################### Send Message to Specific Device #####################
void BluetoothCentral::sendToDevice(NimBLEClient *client,
                                    const std::string &message) {
  if (!client || !client->isConnected())
    return;

  NimBLERemoteService *service = client->getService(SERVICE_UUID);
  if (!service)
    return;

  NimBLERemoteCharacteristic *charac =
      service->getCharacteristic(CHARACTERISTIC_UUID);
  if (charac && charac->canWrite()) {
    charac->writeValue(message, false);
    Serial.printf("📤 Sent to [%s]: %s\n",
                  client->getPeerAddress().toString().c_str(), message.c_str());
  }
}

// ###################### Disconnect from ALL Devices #####################
void BluetoothCentral::disconnectAll() {
  for (auto *client : this->connectedClients) {
    if (client && client->isConnected())
      client->disconnect();
    NimBLEDevice::deleteClient(client); // Proper deletion
  }
  this->connectedClients.clear();
}
