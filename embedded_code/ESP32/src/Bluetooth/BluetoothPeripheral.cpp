// src/Bluetooth/BluetoothPeripheral.cpp

#include "BluetoothPeripheral.hpp"

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
      CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
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
  if (!characteristic)
    return;

  std::string received = characteristic->getValue();
  String receivedStr = String(received.c_str());

  if (!received.empty() && receivedStr != String(lastHostMessage.c_str()) &&
      received != lastReply) {
    lastHostMessage = receivedStr.c_str();

    Serial.print("📥 Host said: ");
    Serial.println(receivedStr);
    ConnectionScreen::showMessage("Host said:\n" + receivedStr);

    std::string reply;

    if (responseHandler) {
      reply = responseHandler(receivedStr.c_str());
    } else {
      // Default fallback
      if (receivedStr == "hello slave")
        reply = "hello host";
      else if (receivedStr == "guess what")
        reply = "what is it";
      else if (receivedStr == "we can do this")
        reply = "yes we can!";
      else
        reply = "message received";
    }

    delay(200);
    characteristic->setValue(reply);
    lastReply = reply;
    Serial.print("📤 Replied: ");
    Serial.println(reply.c_str());
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
}

// ###################### Disconnect #####################
void BluetoothPeripheral::ServerCallbacks::onDisconnect(
    NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) {
  Serial.println("🔌 Disconnected.");
  ConnectionScreen::showMessage("Disconnected from Host");

  if (parent->advertising) {
    delay(1000);
    parent->advertising->start();
  }
}
