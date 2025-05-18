// TicTacToe.hpp
#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
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
  JOIN_SCREEN,
};

extern State game_state;

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;

extern const char *BOARD_PATH;
extern const char *X_PATH;
extern const char *O_PATH;
extern const char *DIS_O_PATH;
extern const char *DIS_X_PATH;

extern Button A, B, up, down, left, right;
extern NumPad pad;

extern String board[9];
extern char currentPlayer;
extern char winner;
extern int winCombo[3];
extern unsigned long winTime;
extern bool roundEnded;
extern int cursorIndex;
extern int screen_width, screen_height;
extern const int cell_size;
extern int x_start, y_start;

struct Move {
  int index;
  char symbol;
};

extern Move moveQueue[6];
extern int moveCount;
extern int xWins, oWins;
extern uint16_t orange_color;
extern bool buttonPreviouslyPressed;
extern int selection, subselection;
extern const unsigned long moveDelay;

// ========== Drawing ==========
void drawScoreboard();
void drawWinnerMessage();
void drawWinLine();
void checkWinner();
void clearCursor(int index);
void highlightCursor(int index);
void drawGrid();
void drawAllPlaying();
void drawEndScreen();
void drawHomeScreen();
void drawHomescreenSelect();

// New Functions
int findBestMove(char aiSymbol, char playerSymbol);
