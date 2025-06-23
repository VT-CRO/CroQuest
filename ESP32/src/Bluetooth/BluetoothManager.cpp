// src/Bluetooth/BluetoothManager.cpp

#include "BluetoothManager.hpp"

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

static BluetoothPeripheral *peripheral = nullptr;
static BluetoothCentral *central = nullptr;

enum ActiveRole { NONE, ROLE_PERIPHERAL, ROLE_CENTRAL };
static ActiveRole currentRole = NONE;

// ####################################################################################################
//  Central
// ####################################################################################################

// =========== Initialize Central ============ //
void BluetoothManager::initCentral(TFT_eSPI &display) {
  if (currentRole == ROLE_PERIPHERAL && peripheral) {
    delete peripheral;
    peripheral = nullptr;
    Serial.println("🔄 Switching from PERIPHERAL to CENTRAL");
  }

  if (!central) {
    central = new BluetoothCentral(display);
    currentRole = ROLE_CENTRAL;
    Serial.println("🔧 Initialized CENTRAL");
  }
}

// =========== Get Central ============ //
BluetoothCentral &BluetoothManager::getCentral() { return *central; }

// ####################################################################################################
//  Peripheral
// ####################################################################################################

// =========== Initialize Peripheral ============ //
void BluetoothManager::initPeripheral(TFT_eSPI &display) {
  if (currentRole == ROLE_CENTRAL && central) {
    delete central;
    central = nullptr;
    Serial.println("🔄 Switching from CENTRAL to PERIPHERAL");
  }

  if (!peripheral) {
    peripheral = new BluetoothPeripheral(display);
    currentRole = ROLE_PERIPHERAL;
    Serial.println("🔧 Initialized PERIPHERAL");
  }
}

// =========== Get Peripheral ============ //
BluetoothPeripheral &BluetoothManager::getPeripheral() { return *peripheral; }

// =========== Get Active Clients ============ //
bool BluetoothManager::getPeripheralActive() {
  return currentRole == ROLE_PERIPHERAL && peripheral != nullptr;
}

// ####################################################################################################
//  Connectivity
// ####################################################################################################

// =========== Stop Scanning ============ //
void BluetoothManager::stopScan() {
  if (currentRole == ROLE_CENTRAL && central) {
    NimBLEScan *scanner = NimBLEDevice::getScan();
    if (scanner && scanner->isScanning()) {
      scanner->stop();
      Serial.println("🛑 BLE Scan stopped.");
      ConnectionScreen::showMessage("Stopped scanning.");
    }
  }
}

// =========== Reset Bluetooth ============ //
void BluetoothManager::reset(bool exitToMenu) {
  if (central) {
    central->setHostScreenExit(true);
    if(!exitToMenu){
      central->sendExit();
      delay(100);
    }
    delete central;
    central = nullptr;
  }
  if (peripheral) {
    if(!exitToMenu){
      peripheral->sendExit();
      delay(100);
    }
    delete peripheral;
    peripheral = nullptr;
  }

  if (NimBLEDevice::getScan()->isScanning()) {
    NimBLEDevice::getScan()->stop();
    delay(50); 
}

  NimBLEDevice::deinit(true); // Force full deinit
  delay(100);                 // Let it settle
  currentRole = NONE;

  Serial.println("🔁 Bluetooth stack fully reset.");
}
