#include "bluetooth.hpp"

// Bluetooth state variables
static bool isHost = true; // Starts as a host
static String hostCode = "quest-"; //start of host code
static String clientCode = "";
static BLEAdvertisedDevice* myDevice = nullptr;
static BLECharacteristic* pCharacteristic = nullptr;
static BLEAdvertising *pAdvertising = nullptr;
static BLEClient* pClient = nullptr;
static BLEScan* pBLEScan = nullptr;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
static bool deviceConnected = false;
static bool host_ready = false;
static bool client_ready = false;
static bool multiplayer_enabled = false;

// Callback function pointers
static PaddleUpdateCallback paddleUpdateCallback = nullptr;
static BallUpdateCallback ballUpdateCallback = nullptr;
static ScoreUpdateCallback scoreUpdateCallback = nullptr;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
      String name = advertisedDevice.getName().c_str();
      if (name == ("quest-" + clientCode)) {
          Serial.println("Found matching host: " + name);
          myDevice = new BLEAdvertisedDevice(advertisedDevice);
          pBLEScan->stop();
      }
  }
};

class MyBLECallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String receivedData = pCharacteristic->getValue().c_str();
    if (receivedData.startsWith("ready")) {
      // Handle player readiness
      if (receivedData == "ready@1") {
        host_ready = true;
        Serial.println("Host is ready");
      }
      else {
        client_ready = true;
        Serial.println("Client is ready");
      }
    } 
    else if (receivedData.startsWith("ball")) {
      // Handle ball position and speed
      double x, y, speedX, speedY;
      sscanf(receivedData.c_str(), "ball@%lf,%lf,%lf,%lf", &x, &y, &speedX, &speedY);
      Serial.printf("Ball position: (%lf, %lf), speed: (%lf, %lf)\n", x, y, speedX, speedY);
      
      // Call the registered callback to update the ball in the game
      if (ballUpdateCallback) {
        ballUpdateCallback(x, y, speedX, speedY);
      }
    } 
    else if (receivedData.startsWith("score")) {
      // Handle score updates
      int player = receivedData.charAt(6) - '0';  // Get player number
      Serial.printf("Player %d scored!\n", player);
      
      // Call the registered callback to update the score in the game
      if (scoreUpdateCallback) {
        scoreUpdateCallback(player);
      }
    } 
    else if (receivedData.startsWith("platform")) {
      // Handle paddle movement
      int player, y;
      sscanf(receivedData.c_str(), "platform@%d,%d", &player, &y);
      
      // Call the registered callback to update the paddle in the game
      if (paddleUpdateCallback) {
        paddleUpdateCallback(player, y);
      }
      
      Serial.printf("Player %d's platform moved to y=%d\n", player, y);
    }
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true; 
    Serial.println("Client connected");
  }
  
  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("Client disconnected");
  }
};

class MyClientCallbacks: public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) override {
    deviceConnected = true;
  }
  
  void onDisconnect(BLEClient* pClient) override {
    deviceConnected = false;
  }
};

void initializeBluetooth() {
  // Create random host code
  hostCode += String(random(100000, 999999));
  
  // Initialize BLE functionality
  BLEDevice::init(hostCode.c_str());
}

void setupHost() {
  BLEServer *pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pCharacteristic = pService->createCharacteristic(
                    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_WRITE |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyBLECallbacks());
  pServer->setCallbacks(new MyServerCallbacks());
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pAdvertising->start();
}

bool connectToHost() {
  if (myDevice == nullptr) return false;

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());

  if (!pClient->connect(myDevice)) {
    Serial.println("Failed to connect.");
    return false;
  }

  BLERemoteService* pRemoteService = pClient->getService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service.");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8");
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find characteristic.");
    pClient->disconnect();
    return false;
  }

  deviceConnected = true;
  Serial.println("Connected to host!");
  return true;
}

void startScan() {
  if (pAdvertising) {
    pAdvertising->stop();
  }
  
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false); // 5 seconds scan
}

void sendPaddlePosition(int player, int y) {
  if (deviceConnected && pCharacteristic != nullptr) {
    String command = "platform@" + String(player) + "," + String(y);
    pCharacteristic->setValue(command.c_str());
    pCharacteristic->notify();
  }
}

void sendBallPosition(double x, double y, double speedX, double speedY) {
  if (deviceConnected && pCharacteristic != nullptr) {
    String command = "ball@" + String(x) + "," + String(y) + "," + String(speedX) + "," + String(speedY);
    pCharacteristic->setValue(command.c_str());
    pCharacteristic->notify();
  }
}

void sendScore(int player) {
  if (deviceConnected && pCharacteristic != nullptr) {
    String command = "score@" + String(player);
    pCharacteristic->setValue(command.c_str());
    pCharacteristic->notify();
  }
}

void sendReadyStatus(bool isHostMode) {
  if (deviceConnected && pCharacteristic != nullptr) {
    String readyCommand = isHostMode ? "ready@1" : "ready@2";
    pCharacteristic->setValue(readyCommand.c_str());
    pCharacteristic->notify();
    
    if (isHostMode) {
      host_ready = true;
    } else {
      client_ready = true;
    }
  }
}

bool isDeviceConnected() {
  return deviceConnected;
}

void startAdvertising() {
  if (pAdvertising) {
    pAdvertising->start();
  }
}

void stopAdvertising() {
  if (pAdvertising) {
    pAdvertising->stop();
  }
}

void setConnectionCode(String code) {
  clientCode = code;
}

String getHostCode() {
  return hostCode.substring(6); // Return just the numeric part
}

void setHostMode(bool isHostMode) {
  isHost = isHostMode;
}

bool isInHostMode() {
  return isHost;
}

bool isHostReady() {
  return host_ready;
}

bool isClientReady() {
  return client_ready;
}

void resetReadyStatus() {
  host_ready = false;
  client_ready = false;
}

void enableMultiplayer() {
  multiplayer_enabled = true;
}

void disableMultiplayer() {
    multiplayer_enabled = false;
}

bool isMultiplayerEnabled() {
  return multiplayer_enabled;
}

// Register callback functions
void registerPaddleUpdateCallback(PaddleUpdateCallback callback) {
  paddleUpdateCallback = callback;
}

void registerBallUpdateCallback(BallUpdateCallback callback) {
  ballUpdateCallback = callback;
}

void registerScoreUpdateCallback(ScoreUpdateCallback callback) {
  scoreUpdateCallback = callback;
}
