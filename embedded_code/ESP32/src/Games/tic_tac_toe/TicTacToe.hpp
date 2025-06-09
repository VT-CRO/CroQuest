// src/Games/tic_tac_toe/TicTacToe.hpp

#pragma once

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "HostGame/HostGame.hpp"
#include "JoinHost/JoinHost.hpp"
#include "Menu/MenuReturn.hpp"
#include "Notifications/NotificationManager.hpp"
#include "NumPad/NumPad.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/BadgesMenu/Badges.hpp"

#include <TFT_eSPI.h>

// ========== API ==========
void runTicTacToe();
void handleTicTacToeFrame();

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, B, up, down, left, right;

extern bool ticTacToeStateChanged;

struct Move {
  int index;
  char symbol;
};

struct TicTacToeSession {
  int consecutiveWins = 0;
  bool badgeUnlocked = false;
};

extern TicTacToeSession session;

String generateTicTacToeStateString();

void readTicTacToeString(String oldState, const char *data);

void drawWinLine();

// ========== Draw Title and Grid ========== //
static void drawTitleAndGrid();

void drawAllPlaying();