// /src/HostGame/HostGame.hpp

#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

#include "Core/Buttons.hpp"
#include "Menu/MenuReturn.hpp"

namespace HostGame {
// void init(TFT_eSPI &display);
void showCode(const std::string &code, int selection, std::vector<String> &playerNames, bool host);
void showStatus(const String &msg); // Optional line under code
void loopUntilConnected();

} // namespace HostGame
