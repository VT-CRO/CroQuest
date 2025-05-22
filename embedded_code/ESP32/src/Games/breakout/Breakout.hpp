#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== API ==========
void runBreakout();         // Call once to start the game
void handleBreakoutFrame(); // Call repeatedly from loop()

// ========== Game States ==========
enum BreakoutState {
  BREAKOUT_HOMESCREEN,
  BREAKOUT_PLAYING,
  BREAKOUT_GAMEOVER,
  BREAKOUT_WIN,
  BREAKOUT_MULTIPLAYER_SELECTION,
  BREAKOUT_JOIN_SCREEN,
  BREAKOUT_BLUETOOTH_NUMPAD,
  BREAKOUT_GAMEOVER_SCREEN,
};

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;

extern int breakout_selection;
extern int breakout_subselection;
extern BreakoutState currentBreakoutState;

void initBricks();

void drawBreakoutGameOverSelect();

void drawBreakoutGameOverScreen();

void drawBreakoutHomeSelection();

void drawBreakoutHomeScreen();

void resetBall();

void eraseOldBall();

void drawBall();

void eraseOldPaddle();

void drawPaddle();

void drawHUD();

void updateBreakoutGame();

void drawBreakoutFrame();

void moveBallSafely();

bool checkBallCollisions();