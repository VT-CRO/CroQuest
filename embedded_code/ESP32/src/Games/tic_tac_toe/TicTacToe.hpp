// TicTacToe.hpp
#pragma once

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "JoinHost/JoinHost.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== API ==========
void runTicTacToe();
void handleTicTacToeFrame();

// ========== Game States ==========
enum State {
  HOMESCREEN,
  MULTIPLAYER,
  SINGLE_PLAYER,
  GAMEOVER_SCREEN,
  BLUETOOTH_NUMPAD,
  MULTIPLAYER_SELECTION,
  MULTIPLAYER_PLAYING,
  JOIN_SCREEN,
  HOST_SCREEN,
};

// extern State game_state;

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, B, up, down, left, right;

struct Move {
  int index;
  char symbol;
};