// src/Games/connect4/Connect4.hpp

#pragma once

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "EndScreen/EndScreen.hpp"
#include "HostGame/HostGame.hpp"
#include "JoinHost/JoinHost.hpp"
#include "Menu/MenuReturn.hpp"
#include "Notifications/NotificationManager.hpp"
#include "NumPad/NumPad.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/BadgesMenu/Badges.hpp"

#include <TFT_eSPI.h>

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

extern TFT_eSPI tft;
extern JpegDrawing drawing;

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runConnect4();

// ========== Manual Loop ========== //
void handleConnect4Frame();

// ####################################################################################################
//  Game Logic
// ####################################################################################################

// ========== Drop Asset ========== //
static bool dropPiece(int col, int player);

// ========== Check Winner ========== //
static bool checkWin(int player);

// ========== Reset Game Board ========== //
static void resetConnect4Board(bool clearScreen = true);

// ========== Reset Single Player Defaults ========== //
static void resetToSinglePlayerDefaults();

// ========== Reset Multiplayer State ========== //
static void resetMultiplayerState(bool clearScreen = true);

// ========== Center Piece ========== //
static void getCellCenter(int col, int row, int &px, int &py);

// ========== Flash Cursor Red (Invalid Move Feedback) ==========
static void flashCursorRed();

// ========== AI Setup ==========
static int findBestConnect4Move(int aiPlayer, int humanPlayer);

// ========== Get Available Row ==========
static int getAvailableRow(int col);

// ========== Can Move ==========
static bool canDrop(int col);

// ========== Board Full ==========
static bool isBoardFull();

// ####################################################################################################
//  Game Drawing
// ####################################################################################################

// ========== Draw Grid ========== //
static void drawGrid();

// ========== Draw Cursor ========== //
static void drawCursor();

// ========== Draw Winner Line ========== //
static void drawWinLine(int player);

// ========== Draw Score Panel ========== //
static void drawScorePanel(int p1Wins, int p2Wins, int currentPlayer);

// ========== Dropping Animation ========== //
static void animatePieceDrop(int col, int finalRow, int player);

// ========== Draw HomeScreen ========== //
static void drawHomeScreen();

// ========== Draw HomeScreen Buttons ========== //
static void drawHomescreenSelect();

// ========== Draw Title & Grid ========== //
static void drawTitleAndGrid();

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Dropping Piece ========== //
static void playDropSound();

// ========== Winner Sound ========== //
static void playWinSound();

// ========== Error Sound ========== //
static void playErrorSound();