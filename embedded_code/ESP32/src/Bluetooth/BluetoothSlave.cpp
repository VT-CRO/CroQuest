#include "BluetoothSlave.hpp"
#include "ConnectionScreen.hpp"
#include "UUIDs.hpp"
#include <NimBLEDevice.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ==================== Config ====================
const char *DEVICE_NAME = "CroQuest_ESP";
const std::string ACCESS_CODE = "code:060970";

extern TFT_eSPI tft;

NimBLECharacteristic *characteristic = nullptr;
NimBLEAdvertising *pAdvertising = nullptr;

// ==================== Connection Callback ====================
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.println("✅ Host connected!");
    ConnectionScreen::showMessage("Connected to Host!");
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    Serial.println("🔌 Disconnected.");
    ConnectionScreen::showMessage("Disconnected from Host");
    if (pAdvertising) {
      delay(1000);
      pAdvertising->start();
    }
  }
};

// ==================== Setup ====================
void initBluetoothSlave() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);

  ConnectionScreen::showMessage("Starting BLE advertiser...");
  delay(1000);

  NimBLEDevice::init(DEVICE_NAME);
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *service = pServer->createService(SERVICE_UUID);
  characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  characteristic->setValue("ACK");
  service->start();

  pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData adData;
  adData.setName(DEVICE_NAME);
  adData.setManufacturerData(std::string("\x01\x02") + ACCESS_CODE);
  pAdvertising->setAdvertisementData(adData);
  pAdvertising->addServiceUUID(service->getUUID());
  pAdvertising->start();

  ConnectionScreen::showMessage("Advertising with code:\n" +
                                String(ACCESS_CODE.c_str()));
  Serial.println("Advertising with code:\n" + String(ACCESS_CODE.c_str()));
}

// ==================== Loop ====================
void updateBluetoothSlave() {
  static String lastHostMessage = "";
  static String lastSlaveReply = "";

  if (characteristic) {
    std::string received = characteristic->getValue();
    String receivedStr = String(received.c_str());

    if (!received.empty() && receivedStr != lastHostMessage &&
        receivedStr != lastSlaveReply) {
      lastHostMessage = receivedStr;
      Serial.print("📥 Host said: ");
      Serial.println(receivedStr);
      ConnectionScreen::showMessage("Host said:\n" + receivedStr);

      String reply;
      if (receivedStr == "hello slave")
        reply = "hello host";
      else if (receivedStr == "guess what")
        reply = "what is it";
      else if (receivedStr == "we can do this")
        reply = "yes we can!";
      else
        reply = "message received";

      delay(500);
      characteristic->setValue(reply.c_str());
      lastSlaveReply = reply;
      Serial.print("📤 Slave replied: ");
      Serial.println(reply);
    }
  }
  delay(100);
}
