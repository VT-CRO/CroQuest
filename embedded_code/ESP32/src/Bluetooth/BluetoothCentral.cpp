// src/Bluetooth/BluetoothCentral.cpp

#include "BluetoothCentral.hpp"
#include "ConnectionScreen.hpp"

BluetoothCentral::BluetoothCentral(TFT_eSPI &display) : tft(display) {}

// ####################################################################################################
//  Scan Callbacks
// ####################################################################################################
void BluetoothCentral::ScanCallbacks::onResult(
    const NimBLEAdvertisedDevice *advertisedDevice) {

  std::string name = advertisedDevice->getName();
  std::string manuData = advertisedDevice->getManufacturerData();

  if (manuData.find("code:") == 0 && manuData.length() >= 11) {
    std::string codePart = manuData.substr(5, 6);
    Serial.printf("🔍 Found %s | Code: %s\n", name.c_str(), codePart.c_str());

    if (codePart == parent->targetCode) {
      Serial.printf("✅ Match found: %s\n", name.c_str());

      // ✅ Add match and stop scanning immediately
      parent->foundDevices.push_back(*advertisedDevice);
      NimBLEDevice::getScan()->stop(); // ✅ prevents repeat matches
    } else {
      Serial.println("🚫 Code did not match");
    }
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
  scanner->start(0, false); // Scan continuously

  ConnectionScreen::showMessage("Scanning...\nAccess Code:\n" +
                                String(accessCode.c_str()));
}

// ###################### Keep Scanning for more Players #####################
void BluetoothCentral::scanAndConnectLoop(const std::string &accessCode) {
  this->targetCode = accessCode;
  this->foundDevices.clear();
  Serial.println("🔄 Starting scan-and-connect loop...");

  ConnectionScreen::showMessage("Scanning for players...\nCode: " +
                                String(this->targetCode.c_str()));
  beginScan(this->targetCode);

  // ✅ Wait until scan stops due to onResult()
  while (NimBLEDevice::getScan()->isScanning()) {
    delay(100);
  }

  Serial.println("⏹ Scan stopped.");

  if (!foundDevices.empty()) {
    connectToDevices();
  } else {
    Serial.println("⚠️ No matching devices found.");
    ConnectionScreen::showMessage("No devices found.\nTry again.");
  }

  Serial.println("✅ Scan-and-connect loop complete.");
}

// ###################### Connect to All Devices #####################
void BluetoothCentral::connectToDevices() {
  this->connectedClients.clear();

  if (foundDevices.empty()) {
    Serial.println("⚠️ No devices to connect to.");
    return;
  }

  NimBLEAdvertisedDevice &device = foundDevices[0];

  Serial.printf("🔗 Attempting to connect to %s (%s)...\n",
                device.getName().c_str(),
                device.getAddress().toString().c_str());

  NimBLEClient *client = NimBLEDevice::createClient();
  if (client->connect(&device)) {
    Serial.printf("✅ Connected to %s\n", device.getName().c_str());
    this->connectedClients.push_back(client);

    ConnectionScreen::showMessage("Connected to 1 player!");

    sendToDevice(client, "Hello Slave");
  }

  else {
    Serial.println("❌ Connection failed.");
    NimBLEDevice::deleteClient(client);
    ConnectionScreen::showMessage("Failed to connect.");
  }
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

    std::string rawData;
    try {
      rawData = charac->readValue();
    } catch (...) {
      Serial.println("❌ Failed to read value from characteristic.");
      continue;
    }

    std::string data = sanitize(rawData);
    std::string &lastMessage = lastMessages[client];

    // Skip if same message
    if (data.empty() || data == lastMessage)
      continue;

    lastMessage = data;

    Serial.printf("📥 Received from client: %s\n", rawData.c_str());

    // Handle Tic Tac Toe game state
    if (data.rfind("ttt@", 0) == 0) {
      readTicTacToeString("", data.c_str()); // mirror and sync board
    } else {
      Serial.println("⚠️ Unknown message format, ignored.");
    }
  }
}

// ###################### Send Message to Specific Device
// #####################
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
    // Serial.printf("📤 Sent to [%s]: %s\n",
    //               client->getPeerAddress().toString().c_str(),
    //               message.c_str());
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

const std::vector<NimBLEClient *> &
BluetoothCentral::getConnectedClients() const {
  return connectedClients;
}

std::string BluetoothCentral::sanitize(const std::string &input) {
  size_t start = input.find_first_not_of(" \n\r\t");
  size_t end = input.find_last_not_of(" \n\r\t");

  if (start == std::string::npos || end == std::string::npos)
    return "";

  std::string trimmed = input.substr(start, end - start + 1);
  for (char &c : trimmed) {
    c = tolower(c);
  }
  return trimmed;
}
