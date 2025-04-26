#include "bluetooth_guest.hpp"
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Global variables
bool isConnected = false;
std::vector<String> receivedData;
BLEAdvertising* pAdvertising = nullptr;
BLEServer* pServer = nullptr;
String deviceName = "";

// Callback class for server events
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        isConnected = true;
    }
    
    void onDisconnect(BLEServer* pServer) {
        isConnected = false;
        // Restart advertising to allow reconnection
        pAdvertising->start();
    }
};

// Callback class for characteristic events
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            receivedData.push_back(String(value.c_str()));
        }
    }
};

void btStartAdvertising(uint16_t host_code) {
    // Generate a random suffix for the device name
    int randomSuffix = random(100000, 999999);
    deviceName = "quest-guest-" + String(host_code) + "-" + String(randomSuffix);
    
    // Initialize BLE
    BLEDevice::init(deviceName.c_str());
    
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Create a BLE Service
    BLEService* pService = pServer->createService(BLEUUID((uint16_t)0xFF00));
    
    // Create a BLE Characteristic
    BLECharacteristic* pCharacteristic = pService->createCharacteristic(
        BLEUUID((uint16_t)0xFF01),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    
    // Start the service
    pService->start();
    
    // Start advertising
    pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID((uint16_t)0xFF00));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();
}

void btStopAdvertising() {
    if (pAdvertising != nullptr) {
        pAdvertising->stop();
    }
}

bool btIsConnected() {
    return isConnected;
}

bool btSendToHost(const String& data) {
    if (!isConnected) {
        return false;
    }
    
    // In a real implementation, we would send data to the host
    // using BLE characteristics
    return true;
}

std::vector<String> btReceivedFromHost() {
    // Return and clear the received data
    std::vector<String> result = receivedData;
    receivedData.clear();
    return result;
} 