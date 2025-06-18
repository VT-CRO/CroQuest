// src/Bluetooth/BluetoothCentral.cpp

#include "BluetoothCentral.hpp"
#include "ConnectionScreen.hpp"

extern bool simonStateChanged;
extern void readSimonString(String oldState, const char *data);
extern String generateSimonString(String mode = "full");

//Ready string
extern void handleReadyMessage(const std::string& message);

// Connect4
extern bool connect4StateChanged;
extern void readConnect4String(String oldState, const char *data);

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

// ###################### In case of Disconnect #####################
void BluetoothCentral::MyClientCallbacks::onDisconnect(NimBLEClient *client,
                                                       int reason) {
  Serial.println("🔌 Peripheral disconnected.");
  ConnectionScreen::showMessage("Peripheral disconnected.");

  delay(1000); // Optional delay to let user see the message

  if (!intentionalExit) {
    shouldExitToMenu = true; // Triggers return to menu
  }
  intentionalExit = false;
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

  callbacks = new MyClientCallbacks();
  client->setClientCallbacks(callbacks);

  // Try to connect up to 3 times before giving up
  bool connected = false;
  for (int i = 0; i < 3; i++) {
    if (client->connect(&device)) {
      connected = true;
      break;
    }
    delay(200);
  }

  if (connected) {
    Serial.printf("✅ Connected to %s\n", device.getName().c_str());
    this->connectedClients.push_back(client);

    ConnectionScreen::showMessage("Connected to 1 player!");

    // 🔔 Set up notify once after connection
    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (service) {
      NimBLERemoteCharacteristic *charac =
          service->getCharacteristic(CHARACTERISTIC_UUID);
      if (charac && charac->canNotify()) {
        bool success = charac->subscribe(
            true, [this](NimBLERemoteCharacteristic *c, uint8_t *data,
                         size_t length, bool isNotify) {
              std::string msg(reinterpret_cast<char *>(data), length);
              Serial.print("📥 Notification received: ");
              Serial.println(msg.c_str());

              if (msg.rfind("ttt@", 0) == 0) {
                Serial.println("PERIPHERAL RECEIVED: ");
                Serial.println(msg.c_str());

                // Apply the new state to the host
                readTicTacToeString("", msg.c_str());
                ticTacToeStateChanged = true;

                BluetoothCentral &central = BluetoothManager::getCentral();
                String confirmedState = String(msg.c_str());
                for (auto *client : central.getConnectedClients()) {
                  central.sendToDevice(client, confirmedState.c_str());
                }

              } else if (msg.rfind("s@", 0) == 0) {
                Serial.println("PERIPHERAL SENT SIMON: ");
                Serial.println(msg.c_str());

                readSimonString("", msg.c_str());
                simonStateChanged = true;

                // Sends updated state to all peripherals
                BluetoothCentral &central = BluetoothManager::getCentral();
                String confirmedState = generateSimonString();
                for (auto *client : central.getConnectedClients()) {
                  central.sendToDevice(client, confirmedState.c_str());
                }
              } else if (msg.rfind("@pong", 0) == 0) {
                this->latestMessage = msg;

              } else if (msg.rfind("c4@", 0) == 0) {
                Serial.println("PERIPHERAL SENT CONNECT4:");
                Serial.println(msg.c_str());
                readConnect4String("", msg.c_str());
              } else if (strcmp(msg.c_str(), "exit") == 0) {
                callbacks->intentionalExit = true;
              }else if(msg.rfind("ready@", 0) == 0){
                handleReadyMessage(msg.c_str());
              }
               else {
                Serial.println("⚠️ Unknown message format (notify).");
              }
            });

        if (success) {
          Serial.println("✅ Subscribed to notifications.");
        } else {
          Serial.println("❌ Failed to subscribe.");
        }
      }
    }

    sendToDevice(client, "Hello Slave");

  } else {
    Serial.println("❌ Connection failed.");
    NimBLEDevice::deleteClient(client);
    ConnectionScreen::showMessage("Failed to connect.");
  }
}

// ###################### Poll Devices for Incoming Data #####################
void BluetoothCentral::pollDevices() {}

// ###################### Send Message to Specific Device
// #####################
bool BluetoothCentral::sendToDevice(NimBLEClient *client,
                                    const std::string &message) {
  if (!client || !client->isConnected()) {
    Serial.println("❌ Cannot send: client is null or disconnected.");
    return false;
  }

  NimBLERemoteService *service = client->getService(SERVICE_UUID);
  if (!service) {
    Serial.println("❌ Cannot send: service not found.");
    return false;
  }

  NimBLERemoteCharacteristic *characteristic =
      service->getCharacteristic(CHARACTERISTIC_UUID);
  if (!characteristic || !characteristic->canNotify()) {
    Serial.println("❌ Cannot send: invalid characteristic.");
    return false;
  }

  try {
    characteristic->writeValue(
        message); // or setValue + notify() depending on setup
    Serial.print("📤 Sent to client: ");
    Serial.println(message.c_str());
    return true;
  } catch (...) {
    Serial.println("❌ Exception while sending to client.");
    return false;
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

// ###################### Update Poll #####################
void BluetoothCentral::update() {
  std::string msg = readMessage();

  if (msg.empty())
    return;

  Serial.print("📥 Central received via polling: ");
  Serial.println(msg.c_str());

  if (msg.rfind("ttt@", 0) == 0) {
    readTicTacToeString("", msg.c_str());
    ticTacToeStateChanged = true;

  } else if (msg.rfind("s@", 0) == 0) {
    readSimonString("", msg.c_str());
    simonStateChanged = true;

  } else if (msg.rfind("c4@", 0) == 0) {
    readConnect4String("", msg.c_str());

  } else if (msg == "exit") {
    callbacks->intentionalExit = true;

  } else {
    Serial.println("⚠️ Unknown message format (polled).");
  }
}

// ###################### Read Messages #####################
std::string BluetoothCentral::readMessage() {
  std::string msg = this->latestMessage;
  this->latestMessage.clear(); // Clear so it doesn't repeat
  return msg;
}

// ###################### Send Messages (not array) #####################
bool BluetoothCentral::sendMessage(const std::string &msg) {
  if (connectedClients.empty())
    return false;

  for (auto *client : this->connectedClients) {
    if (!client || !client->isConnected())
      return false;

    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (!service)
      return false;

    NimBLERemoteCharacteristic *characteristic =
        service->getCharacteristic(CHARACTERISTIC_UUID);
    if (!characteristic || !characteristic->canWrite()) {
      return false;
    }

    try {
      bool success =
          characteristic->writeValue((uint8_t *)msg.data(), msg.size(), true);
      return success;
    } catch (...) {
      return false;
    }
  }

  // ⚠️ Make sure to return a value here:
  return true; // or false depending on your logic
}

bool BluetoothCentral::sendExit() {
  if (callbacks) {
    callbacks->intentionalExit = true;
  }

  if (connectedClients.empty())
    return false;

  for (auto *client : this->connectedClients) {
    if (!client || !client->isConnected())
      return false;

    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (!service)
      return false;

    NimBLERemoteCharacteristic *characteristic =
        service->getCharacteristic(CHARACTERISTIC_UUID);
    if (!characteristic || !characteristic->canWrite()) {
      return false;
    }

    try {
      std::string msg = "exit";
      bool success =
          characteristic->writeValue((uint8_t *)msg.data(), msg.size(), true);
      return success;
    } catch (...) {
      return false;
    }
  }

  // ⚠️ Make sure to return a value here:
  return true; // or false depending on your logic
}

// ###################### Send Messages (not array) #####################
bool BluetoothCentral::sendMessagePong(const std::string &msg) {
  if (connectedClients.empty())
    return false;

  for (auto *client : this->connectedClients) {
    if (!client || !client->isConnected())
      return false;

    NimBLERemoteService *service = client->getService(SERVICE_UUID);
    if (!service)
      return false;

    NimBLERemoteCharacteristic *characteristic =
        service->getCharacteristic(CHARACTERISTIC_UUID);
    if (!characteristic || !characteristic->canWrite()) {
      return false;
    }

    try {
      bool success =
          characteristic->writeValue((uint8_t *)msg.data(), msg.size(), false);
      return success;
    } catch (...) {
      return false;
    }
  }

  // Make sure to return a value here:
  return true; // or false depending on your logic
}
