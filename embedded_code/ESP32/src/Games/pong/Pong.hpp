#pragma once

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "HostGame/HostGame.hpp"
#include "JoinHost/JoinHost.hpp"
#include "Menu/MenuReturn.hpp"
#include "Notifications/NotificationManager.hpp"
#include "NumPad/NumPad.hpp"
#include "SettingsMenu/BadgesMenu/Badges.hpp"
#include <TFT_eSPI.h>

#define HOST_CODE_SIZE 6

// ========== Globals ==========
extern TFT_eSPI tft;
extern Button A, B, up, down, left, right;

//============ API ==============
void runPong();
void handlePongFrame();