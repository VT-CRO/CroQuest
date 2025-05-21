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

// Assets paths
extern const char *DISK_PATH;
extern const char *UP_TRIANGLE;
extern const char *DOWN_TRIANGLE;
extern const char *LEFT_TRIANGLE;
extern const char *RIGHT_TRIANGLE;

// Button mapping for Simon game & Numpad Mapping
extern Button *simonButtons[];

extern int diskX, diskY;

extern int simonSequence[100];
extern int simonSequenceLength;
extern int simonPlayerPos;
extern int simonPlayerScore;
extern int simonSelection;
extern int simonSubselection;
extern SimonState currentState;

extern bool simonFirstRun;
extern bool simonGameStarted;

extern int simonDiskCenterX, simonDiskCenterY, simonDiskSize;
extern int simonScreenWidth, simonScreenHeight;
extern int simonCenterX, simonCenterY;
extern int simonButtonSize;

extern unsigned long simonLastButtonPress;
extern unsigned long simonButtonDebounceDelay;
extern unsigned long simonLastSequenceTime;
extern unsigned long simonSequenceDisplayDelay;
extern unsigned long simonLevelUpTime;

// ========== Drawing ==========
void drawSimonHomeScreen();
void drawSimonGameScreen();
void drawSimonGameOverScreen();
void drawSimonLevelUpScreen();
void drawSimonScore();
void highlightSimonButton(int buttonId);
void drawSimonHomeSelection();
void drawSimonGameOverSelect();
void drawCenteredOverlay(const char *imagePath);
void drawSimonTriangleOverlay(int buttonId);

// ========== Logic ==========
void simonGenerateSequence();
void simonExtendSequence();
void simonPlaySequence();
void simonCheckInput(int buttonPressed);
void simonStartNewGame();
void simonGameOver();
void simonLevelUp();
void simonHandleInput();
void drawPlayerStatusTable();
