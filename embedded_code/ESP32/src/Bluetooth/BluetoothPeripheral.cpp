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
  if (!characteristic) {
    Serial.println("⚠️ characteristic is null, skipping update.");
    return;
  }

  std::string received;
  try {
    received = characteristic->getValue();
  } catch (...) {
    Serial.println("❌ Exception when reading from characteristic.");
    return;
  }

  if (received.empty() || received == lastHostMessage)
    return;

  lastHostMessage = received;

  Serial.print("📥 Host said: ");
  Serial.println(received.c_str());
  ConnectionScreen::showMessage("Host said:\n" + String(received.c_str()));

  std::string reply;

  // ✉️ Handle predefined conversation flow
  if (responseHandler) {
    reply = responseHandler(received.c_str());
  } else {
    if (received == "Hello Slave")
      reply = "Hello Host, guess what?";
    else if (received == "what is it?")
      reply = "We can do this";
    else if (received == "yes we can")
      reply = "Let's go!";
    else
      return;
  }

  // ✅ Only send if it's different from last reply
  if (reply != lastReply) {
    delay(200); // Optional: feel more natural
    try {
      characteristic->setValue(reply);
      lastReply = reply;
      Serial.print("📤 Replied: ");
      Serial.println(reply.c_str());
    } catch (...) {
      Serial.println("❌ Exception when writing to characteristic.");
    }
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

  if (parent->advertising) {
    delay(1000);
    parent->advertising->start();
  }
}
