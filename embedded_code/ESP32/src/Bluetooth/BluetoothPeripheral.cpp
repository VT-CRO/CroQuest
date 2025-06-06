// src/Bluetooth/BluetoothPeripheral.cpp

#include "BluetoothPeripheral.hpp"

//Simon func
extern void readSimonString(String oldState, const char *data);

static bool bleInitialized = false;

BluetoothPeripheral::BluetoothPeripheral(TFT_eSPI &display) : tft(display) {}

// ###################### Start Advertising #####################
void BluetoothPeripheral::beginAdvertising(const std::string &code) {
  accessCode = code;
  Serial.printf("Access code length: %d\n", accessCode.length());

  initializeBluetoothIdentifiers();

  ConnectionScreen::showMessage("Starting BLE advertiser...");
  delay(500);

  // 🔄 Manually track BLE initialization
  if (bleInitialized) {
    Serial.println("♻️ BLE was initialized. Resetting...");
    NimBLEDevice::deinit(true);
    delay(100);
  }

  NimBLEDevice::init(BLE_NAME_PREFIX.c_str());
  bleInitialized = true;
  Serial.println("🔧 NimBLE initialized");

  server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks(this));

  NimBLEService *service = server->createService(SERVICE_UUID);
  characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  characteristic->setValue("ACK");
  service->start();

  advertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData adData;
  adData.setName(BLE_NAME_PREFIX.c_str());

  std::string fullData = "code:" + accessCode;
  adData.setManufacturerData(fullData);

  advertising->setAdvertisementData(adData);
  advertising->start();

  Serial.print("🔎 Advertising raw data: ");
  Serial.println(fullData.c_str());

  Serial.print("📏 Length: ");
  Serial.println(fullData.length());

  Serial.printf("🟢 Advertising state: %s\n",
                advertising->isAdvertising() ? "ON" : "OFF");

  Serial.println("✅ Advertising started.");
  Serial.println("BLE Name: " + String(BLE_NAME_PREFIX.c_str()));
  Serial.println("Access Code: " + String(accessCode.c_str()));
}

// ###################### Update Data #####################
void BluetoothPeripheral::update() {
  // Check if characteristic is valid
  if (!characteristic) {
    Serial.println("⚠️ characteristic is null, skipping update.");
    return;
  }

  // Attempt to read the value from the characteristic
  std::string received;
  try {
    received = characteristic->getValue();
  } catch (...) {
    Serial.println("❌ Exception when reading from characteristic.");
    return;
  }

  // Skip empty or repeated messages
  if (received.empty() || received == lastHostMessage)
    return;

  lastHostMessage = received;

  Serial.print("📥 Game data received: ");
  Serial.println(received.c_str());

  ConnectionScreen::showMessage("Host sent:\n" + String(received.c_str()));

  // Check if it's a Tic Tac Toe game state
  if (received.rfind("ttt@", 0) == 0) {
    readTicTacToeString("", received.c_str());
  }else if (received.rfind("s@", 0) == 0) {
    readSimonString("", received.c_str());
  } else {
    Serial.println("⚠️ Unknown message format, ignored.");
  }
}

// ###################### Voicemail #####################
void BluetoothPeripheral::setResponseHandler(
    std::function<std::string(const std::string &)> handler) {
  this->responseHandler = handler;
}

// ####################################################################################################
//  Server Callbacks
// ####################################################################################################

// ###################### Connect #####################
void BluetoothPeripheral::ServerCallbacks::onConnect(NimBLEServer *pServer,
                                                     NimBLEConnInfo &connInfo) {
  Serial.println("✅ Host connected!");
  ConnectionScreen::showMessage("Connected to Host!");

  // // Optional: Start the conversation from the peripheral
  // if (parent && parent->characteristic) {
  //   // std::string greeting = "hello host";
  //   parent->characteristic->setValue(greeting);
  //   parent->lastReply = greeting;
  //   Serial.print("📤 Sent initial: ");
  //   Serial.println(greeting.c_str());
  // }
}

// ###################### Disconnect #####################
void BluetoothPeripheral::ServerCallbacks::onDisconnect(
    NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) {
  Serial.println("🔌 Disconnected.");
  ConnectionScreen::showMessage("Disconnected from Host");

  delay(1000); // Optional: brief pause before returning

  shouldExitToMenu = true; // Triggers exit in runTicTacToe()

  if (parent->advertising) {
    parent->advertising->start(); // Optional: resume advertising if desired
  }
}

// ###################### Send Arrays #####################
bool BluetoothPeripheral::sendAction(const std::string &message) {
  if (!characteristic) {
    Serial.println("⚠️ Cannot send: characteristic is null.");
    return false;
  }

  try {
    characteristic->setValue(message);
    bool success = characteristic->notify();

    if (!success) {
      Serial.println("❌ Notify failed — client likely disconnected.");
      return false;
    }

    delay(50); // optional, helps reduce congestion
    Serial.print("📤 Sent action: ");
    Serial.println(message.c_str());
    return true;
  } catch (...) {
    Serial.println("❌ Exception while sending action.");
    return false;
  }
}

// ###################### Read Messages #####################
std::string BluetoothPeripheral::readMessage() {
  if (!characteristic)
    return "";

  try {
    std::string val = characteristic->getValue();
    if (!val.empty() && val != lastHostMessage) {
      lastHostMessage = val;
      Serial.print("📥 Peripheral read: ");
      Serial.println(val.c_str());
      return val;
    }
  } catch (...) {
    Serial.println("❌ Error while reading characteristic.");
  }

  return "";
}

// ###################### Send Messages #####################
void BluetoothPeripheral::sendMessage(const std::string &message) {
  sendAction(message);
}
