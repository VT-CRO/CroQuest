# Ping Pong 

This will be the documentation for pong_game. 

Compile with `make pong_game` or simply `make`
Run the game with `make run`

Step 1:

Bluetooth Low Energy: BLE

a six digit random number is generated. The host will begin broadcasting as "quest-XXXXXX"

As long as the host code is visible, the bluetooth will be broadcasted.

Both players will have to be ready to start the game. Here are the commands that can be sent between the players:

### Commands sent via BLE   

`ready@1` - player one is ready

`ball@10, 20, 5.6, -2.3` - ball is at (x, y) = (10, 20) and the speed in x is 5.6, speed in y is -2.3. This command shuold be sent right after each player hits the ball on their CROQUEST.

`score@1` - player one scored.

`platform@2,52` - player two's platform moved to y=52.

`pause@1` - player one paused the game.

`resume@1` - player one resumed the game (only player who can paused can resume, until 3 seconds have passed, then either can resume.)

### example code using BLE:

```
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID         "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID  "abcdefab-cdef-abcd-efab-cdef12345678"
#define RXp2 16
#define TXp2 17

BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;
bool advStarted = false;
BLEAdvertising *pAdvertising;
String incomingBuffer = "";

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    advStarted = false;
  }
  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    advStarted = false;
  }
};

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  BLEDevice::init("WMW-Can-Warmer");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  advStarted = true;
}

void loop() {
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    incomingBuffer += c;
    if (c == '\n') {
      if (deviceConnected) {
        pCharacteristic->setValue(incomingBuffer.c_str());
        pCharacteristic->notify();
      }
      incomingBuffer = "";
    }
  }
  if (!deviceConnected && !advStarted) {
    pAdvertising->start();
    advStarted = true;
  }
  delay(10);
}

```