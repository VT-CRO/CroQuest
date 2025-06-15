// /src/Bluetooth/BluetoothManager.hpp

#pragma once

#include "BluetoothCentral.hpp"
#include "BluetoothPeripheral.hpp"

// Forward Declaration to avoid include errors
class BluetoothCentral;
class BluetoothPeripheral;

namespace BluetoothManager {

//  Central

// =========== Initialize Central ============ //
void initCentral(TFT_eSPI &display);

// =========== Get Central ============ //
BluetoothCentral &getCentral();

//  Peripheral

// =========== Initialize Peripheral ============ //
void initPeripheral(TFT_eSPI &display);

// =========== Get Peripheral ============ //
BluetoothPeripheral &getPeripheral();

// =========== Get Clients ============ //
static bool getPeripheralActive();

//  Connectivity

// =========== Stop Scanning ============ //
void stopScan();

// =========== Reset Bluetooth ============ //
void reset(bool exitToMenu = true);

} // namespace BluetoothManager
