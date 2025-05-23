#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

namespace JoinHost {
void init(TFT_eSPI &display);
void showCode(const String &code);
void showStatus(const String &msg); // Optional line under code
} // namespace JoinHost
