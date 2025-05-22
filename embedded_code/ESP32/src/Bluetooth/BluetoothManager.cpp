// /src/Bluetooth/BluetoothManager.cpp

#include "BluetoothManager.hpp"

static BluetoothPeripheral *peripheral = nullptr;
BluetoothCentral *central = nullptr;

void BluetoothManager::initPeripheral(TFT_eSPI &display) {
  if (!peripheral)
    peripheral = new BluetoothPeripheral(display);
}

BluetoothPeripheral &BluetoothManager::getPeripheral() { return *peripheral; }

void BluetoothManager::initCentral(TFT_eSPI &display) {
  if (!central)
    central = new BluetoothCentral(display);
}

BluetoothCentral &BluetoothManager::getCentral() { return *central; }

void BluetoothManager::stopScan() {
  if (central) {
    NimBLEScan *scanner = NimBLEDevice::getScan();
    if (scanner && scanner->isScanning()) {
      scanner->stop();
      Serial.println("🛑 BLE Scan stopped.");
      ConnectionScreen::showMessage("Stopped scanning.");
    }
  } else {
    Serial.println("⚠️ Central not initialized; scan not stopped.");
  }
}
