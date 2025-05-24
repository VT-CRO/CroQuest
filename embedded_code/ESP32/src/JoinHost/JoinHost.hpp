// /src/JoinHost/JoinHost.hpp#pragma once
#include <TFT_eSPI.h>
#include <string>
#include <type_traits>

#include "Bluetooth/BluetoothCentral.hpp"
#include "Bluetooth/BluetoothManager.hpp"
#include "Bluetooth/ConnectionScreen.hpp"

namespace JoinHost {
bool attemptJoinGame(const std::string &code);
}
