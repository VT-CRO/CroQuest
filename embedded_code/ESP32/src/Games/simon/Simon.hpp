#pragma once

#include "Bluetooth/BluetoothHost.hpp"
#include "Bluetooth/BluetoothSlave.hpp"
#include "Bluetooth/ConnectionScreen.hpp"
#include "Bluetooth/UUIDs.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== API ==========
void runSimon();
void handleSimonFrame();

// ========== Game States ==========
enum SimonState {
  SIMON_HOMESCREEN,
  SIMON_GAMEOVER_SCREEN,
  SIMON_BLUETOOTH_NUMPAD,
  SIMON_MULTIPLAYER_SELECTION,
  SIMON_JOIN_SCREEN,
  SIMON_LEVELUP,
  SIMON_STATE_WATCH,
  SIMON_STATE_PLAY
};

// Button identifiers
enum ButtonID {
  BUTTON_UP = 0,
  BUTTON_DOWN = 1,
  BUTTON_LEFT = 2,
  BUTTON_RIGHT = 3
};

// ========== Globals ==========
extern TFT_eSPI tft;
extern HostBLEServer hostBLE;
extern JpegDrawing drawing;
