#include "bluetooth_host.hpp"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEUtils.h>

// Global variables to store connected devices and received data
std::vector<String> connectedDevices;
std::vector<String> receivedData;
uint16_t currentHostCode = 0;

// Callback class for BLE scan results
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Check if the device name starts with "quest-guest-" and contains the host code
        String deviceName = advertisedDevice.getName().c_str();
        if (deviceName.startsWith("quest-guest-")) {
            // Extract host code from device name (format: quest-guest-xxx-yyyyyy)
            String hostCodeStr = deviceName.substring(12, 15); // Extract xxx part
            uint16_t deviceHostCode = hostCodeStr.toInt();
            
            if (deviceHostCode == currentHostCode) {
                // Connect to this device
                BLEAddress address = advertisedDevice.getAddress();
                BLEClient* client = BLEDevice::createClient();
                client->connect(address);
                connectedDevices.push_back(deviceName);
            }
        }
    }
};

void btScanAndConnect(uint16_t host_code) {
    currentHostCode = host_code;
    connectedDevices.clear();
    
    // Initialize BLE
    BLEDevice::init("CroQuest Host");
    
    // Start scanning
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30, false); // Scan for 30 seconds
}

std::vector<String> btConnectedDevices() {
    return connectedDevices;
}

bool btSendToGuests(const String& data) {
    // Implementation would send data to all connected devices
    // This is a placeholder implementation
    for (const String& device : connectedDevices) {
        // In a real implementation, we would send data to each connected device
        // using BLE characteristics
    }
    return true;
}

std::vector<String> btReceivedFromGuest() {
    // Return and clear the received data
    std::vector<String> result = receivedData;
    receivedData.clear();
    return result;
} 