// src/Bluetooth/BluetoothPeripheral.cpp

#include "BluetoothPeripheral.hpp"

BluetoothPeripheral::BluetoothPeripheral(TFT_eSPI &display) : tft(display) {}

// ###################### Start Advertising #####################
void BluetoothPeripheral::beginAdvertising(const std::string &code) {
  accessCode = code;

  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);

  ConnectionScreen::showMessage("Starting BLE advertiser...");
  delay(1000);

  NimBLEDevice::init(BLE_NAME_PREFIX.c_str()); // Initialize with dynamic name
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
  adData.setManufacturerData(std::string("\x01\x02") + accessCode);
  advertising->setAdvertisementData(adData);
  advertising->addServiceUUID(service->getUUID());
  advertising->start();

  Serial.println("Advertising as: " + String(BLE_NAME_PREFIX.c_str()));
  ConnectionScreen::showMessage(
      "Advertising as:\n" + String(BLE_NAME_PREFIX.c_str()) +
      "\nAccess Code:\n" + String(accessCode.c_str()));
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
