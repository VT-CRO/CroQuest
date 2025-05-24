#include "BluetoothManager.hpp"

static BluetoothPeripheral *peripheral = nullptr;
static BluetoothCentral *central = nullptr;

enum ActiveRole { NONE, ROLE_PERIPHERAL, ROLE_CENTRAL };
static ActiveRole currentRole = NONE;

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

BluetoothPeripheral &BluetoothManager::getPeripheral() { return *peripheral; }
BluetoothCentral &BluetoothManager::getCentral() { return *central; }

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
