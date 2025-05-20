// TicTacToe.hpp
#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include "States/States.hpp"
#include <TFT_eSPI.h>

// ========== API ==========
void runTicTacToe();
void handleTicTacToeFrame();


// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, B, up, down, left, right;
extern NumPad pad;
extern int screen_width;
extern int screen_height;


struct Move {
  int index;
  char symbol;
};

// ========== Drawing ==========
static void drawScoreboard();
static void drawWinnerMessage();
static void drawWinLine();
static void checkWinner();
static void clearCursor(int index);
static void highlightCursor(int index);
static void drawGrid();
static void drawAllPlaying();
static void drawEndScreen();
static void drawHomeScreen();
static void drawHomescreenSelect();
// New Functions
static int findBestMove(char aiSymbol, char playerSymbol);
