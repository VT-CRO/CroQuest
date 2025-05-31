// src/Games/breakout/Breakout.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "Menu/MenuReturn.hpp"
#include "NumPad/NumPad.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include <TFT_eSPI.h>

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

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

// ========== Structures ==========

// Structure for Bricks
struct Brick {
  int x, y;
  bool active;
  uint16_t color;
};

// Structure for Ball
struct Ball {
  int x, y;
  int w, h;
  float vx, vy;
};

// Struct for Paddle
struct Paddle {
  int x, y;
  int w, h;
};

extern TFT_eSPI tft;
extern JpegDrawing drawing;

extern int breakout_selection;
extern int breakout_subselection;
extern BreakoutState currentBreakoutState;

// ####################################################################################################
//  Launch Game
// ####################################################################################################

// ========== Run Game ========== //
void runBreakout();

// ####################################################################################################
//  Game Logic
// ####################################################################################################

// ========== MANUAL LOOP ========== //
void handleBreakoutFrame();

// ========== Initialize Bricks Logic ========== //
void initBricks();

// ========== Resets Ball Position ========== //
void resetBall();

// ========== Update Game Status ========== //
void updateBreakoutGame();

// ========== Update Ball Status ========== //
void updateBall(Ball *b, Paddle *paddle);

// ========== Update Paddle Status ========== //
void updatePaddle(Paddle *p);

// ####################################################################################################
//  Game Drawing
// ####################################################################################################

// ========== Draw GameOver Selection Buttons ========== //
void drawBreakoutGameOverSelect();

// ========== Draw GameOver Screen ========== //
void drawBreakoutGameOverScreen();

// ========== Draw HomeScreen Selection Buttons ========== //
void drawBreakoutHomeSelection();

// ========== Draw HomeScreen Screen ========== //
void drawBreakoutHomeScreen();

// ========== Draw Game Frame (Ball and Paddle logic) ========== //
void drawBreakoutFrame();

// ========== Draw Ball ========== //
void drawBall(const Ball *b);

// ========== Remove Ball ========== //
void eraseBall(const Ball *b);

// ========== Draw Paddle ========== //
void drawPaddle(const Paddle *p);

// ========== Remove Paddle ========== //
void erasePaddle(const Paddle *p);

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Start Game Sound ========== //
void playStartSound();

// ========== Bouncing Sound ========== //
void playBounceSound();

// ========== Life Lost Sound ========== //
void playLoseLifeSound();

// ========== Breaking Brick Sound ========== //
void playBreakSound();
