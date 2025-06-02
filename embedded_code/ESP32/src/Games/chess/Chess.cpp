// Games/chss/Chess.cpp

#include "Chess.hpp"

// ####################################################################################################
//  Functions Declarations
// ####################################################################################################

// ========== Drawing ==========
void drawChessBoard();
void drawAllChess();
void highlightCursor(int row, int col);

// ========== Sound ==========

// ========== Logic ==========
void resetChessBoard(bool clearScreen = true);
void handleCursorMovement();
void handleSelection();

// ========== Game States ==========
enum ChessState {
  CHESS_HOMESCREEN,
  CHESS_MULTIPLAYER,
  CHESS_PLAYING,
  CHESS_GAMEOVER,
  CHESS_SINGLE_PLAYER,
  CHESS_GAMEOVER_SCREEN,
  CHESS_BLUETOOTH_NUMPAD,
  CHESS_MULTIPLAYER_SELECTION,
  CHESS_MULTIPLAYER_PLAYING,
  CHESS_JOIN_SCREEN,
  CHESS_HOST_SCREEN,
};

static ChessState chess_state = CHESS_PLAYING;

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

// Speaker Pin
#define SPEAKER_PIN 21

#define LIGHT_BROWN 0xBC49
#define DARK_BROWN 0x6249
#define CURSOR_COLOR TFT_GREEN
#define TILE_SIZE 32

const int BOARD_SIZE = TILE_SIZE * 8;
const int BOARD_X = (320 - BOARD_SIZE) / 2;
const int BOARD_Y = (480 - BOARD_SIZE) / 2;

static int cursorX = 0;
static int cursorY = 0;

char chessBoard[8][8]; // 'P','p','R','r', etc. (uppercase = white, lowercase =
                       // black)

static bool firstFrame = true;
static unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 100;

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runChess() {
  resetExitFlag();
  tft.fillScreen(TFT_BLACK);
  resetChessBoard();

  firstFrame = true;

  while (true) {
    handleChessFrame();

    if (getExitFlag())
      return;

    if (B.wasJustPressed()) {
      Serial.println("Returning to menu from Chess");
      delay(500);
      return;
    }
  }
}

// ========== Manual Loop ========== //
void handleChessFrame() {
  if (checkStartButtonAndExit(tft))
    return;

  if (firstFrame) {
    drawAllChess();
    firstFrame = false;
  }

  if (millis() - lastMoveTime > moveDelay) {
    handleCursorMovement();
    handleSelection();
    lastMoveTime = millis();
  }
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Cursor Movement ========== //
void handleCursorMovement() {
  if (up.isPressed() && cursorY > 0)
    cursorY--;
  else if (down.isPressed() && cursorY < 7)
    cursorY++;
  else if (left.isPressed() && cursorX > 0)
    cursorX--;
  else if (right.isPressed() && cursorX < 7)
    cursorX++;

  drawAllChess(); // redraw board + cursor
}

// ========== Cursor Selection ========== //
void handleSelection() {
  if (A.wasJustPressed()) {
    // TODO: Add logic for selecting a piece, showing possible moves, or moving
    Serial.printf("🟢 Selected (%d,%d) = %c\n", cursorY, cursorX,
                  chessBoard[cursorY][cursorX]);
  }
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// ========== Draw Chess Board ========== //
void drawChessBoard() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      bool isLight = (row + col) % 2 == 0;
      uint16_t color = isLight ? LIGHT_BROWN : DARK_BROWN;
      tft.fillRect(BOARD_X + col * TILE_SIZE, BOARD_Y + row * TILE_SIZE,
                   TILE_SIZE, TILE_SIZE, color);
    }
  }
}

// ========== Draw Pieces ========== //
void drawAllChess() {
  drawChessBoard();
  highlightCursor(cursorY, cursorX);

  // TODO: Draw all pieces using image assets
}

// ========== Highlight Cursor ========== //
void highlightCursor(int row, int col) {
  int x = BOARD_X + col * TILE_SIZE;
  int y = BOARD_Y + row * TILE_SIZE;

  tft.drawRect(x, y, TILE_SIZE, TILE_SIZE, CURSOR_COLOR);
  tft.drawRect(x + 1, y + 1, TILE_SIZE - 2, TILE_SIZE - 2, CURSOR_COLOR);
}

// ####################################################################################################
//  Initialization
// ####################################################################################################

void resetChessBoard(bool clearScreen) {
  if (clearScreen)
    tft.fillScreen(TFT_BLACK);

  // Standard initial layout (you can improve this)
  const char *startingLayout[8] = {"rnbqkbnr", "pppppppp", "........",
                                   "........", "........", "........",
                                   "PPPPPPPP", "RNBQKBNR"};

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      chessBoard[row][col] = startingLayout[row][col];
    }
  }
}