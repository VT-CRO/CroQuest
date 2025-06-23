// /src/Bluetooth/ConnectionScreen.hpp

#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace ConnectionScreen {
void init(TFT_eSPI &display);
void showMessage(const String &msg);
} // namespace ConnectionScreen