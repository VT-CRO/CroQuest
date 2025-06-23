#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ==================== Config ====================
const char *DEVICE_NAME = "CroQuest_ESP";
const std::string ACCESS_CODE = "code:060970";

// ==================== Definitions ====================
void printMessage(const String &text);

TFT_eSPI tft = TFT_eSPI();
NimBLECharacteristic *characteristic = nullptr;
NimBLEAdvertising *pAdvertising = nullptr;

// ==================== Connection Callback ====================
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.println("✅ Host connected!");
    printMessage("Connected to Host!");
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    Serial.println("🔌 Disconnected.");
    printMessage("Disconnected from Host");
    if (pAdvertising) {
      delay(1000);
      pAdvertising->start();
    }
  }
};

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);

  printMessage("Starting BLE advertiser " + String(DEVICE_NAME) + " ...");
  delay(1000);

  NimBLEDevice::init(DEVICE_NAME);
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *service =
      pServer->createService("12345678-1234-1234-1234-123456789abc");
  characteristic = service->createCharacteristic(
      "abcdefab-1234-1234-1234-abcdefabcdef",
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  characteristic->setValue("ACK");
  service->start();

  pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData adData;
  adData.setName(DEVICE_NAME);
  adData.setManufacturerData(std::string("\x01\x02") + ACCESS_CODE);
  pAdvertising->setAdvertisementData(adData);
  pAdvertising->addServiceUUID(service->getUUID());
  pAdvertising->start();

  printMessage("Advertising with code:\n" + String(ACCESS_CODE.c_str()));
  Serial.println("Advertising with code:\n" + String(ACCESS_CODE.c_str()));
}

// ==================== Loop ====================
void loop() {
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
      printMessage("Host said:\n" + receivedStr);

      String reply;
      if (receivedStr == "hello slave")
        reply = "hello host";
      else if (receivedStr == "guess what")
        reply = "what is it";
      else if (receivedStr == "we can do this")
        reply = "yes we can!";
      else
        reply = "message received";

      delay(500); // Short delay before responding
      characteristic->setValue(reply.c_str());
      lastSlaveReply = reply;
      Serial.print("📤 Slave replied: ");
      Serial.println(reply);
    }
  }
  delay(100);
}

// ==================== Print Message ====================
void printMessage(const String &text) {
  tft.setRotation(3);
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
