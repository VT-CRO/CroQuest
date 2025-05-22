// /src/Bluetooth/BluetoothManager.hpp

#pragma once

#include "BluetoothCentral.hpp"
#include "BluetoothPeripheral.hpp"

// Forward Declaration to avoid include errors
class BluetoothCentral;
class BluetoothPeripheral;

namespace BluetoothManager {
void initPeripheral(TFT_eSPI &display);
BluetoothPeripheral &getPeripheral();

// Central (Client)
void initCentral(TFT_eSPI &display);
BluetoothCentral &getCentral();

// Utilities
void stopScan();

} // namespace BluetoothManager
