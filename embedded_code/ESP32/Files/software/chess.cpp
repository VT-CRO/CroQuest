// chess.cpp

#include <Arduino.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Uses setup in User_Setup.h

// #include "chess.hpp"

// ###################### Definitions ######################

//========================================================================
// TODO: Change this later
// External button input handlers (to be defined in your buttons code)
extern bool isLeftPressed();
extern bool isRightPressed();
extern bool isAButtonPressed();
//========================================================================

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define SD_CS 5 // Or whatever pin you're using

// ###################### Board & Pieces ######################
const char *CHESS_BOARD_PATH = "/chess/assets/board.jpg";

// White pieces
const char *WHITE_PAWN = "/chess/assets/wp.jpg";
const char *WHITE_ROOK = "/chess/assets/wr.jpg";
const char *WHITE_KNIGHT = "/chess/assets/wn.jpg";
const char *WHITE_BISHOP = "/chess/assets/wb.jpg";
const char *WHITE_QUEEN = "/chess/assets/wq.jpg";
const char *WHITE_KING = "/chess/assets/wk.jpg";

// Black pieces
const char *BLACK_PAWN = "/chess/assets/bp.jpg";
const char *BLACK_ROOK = "/chess/assets/br.jpg";
const char *BLACK_KNIGHT = "/chess/assets/bn.jpg";
const char *BLACK_BISHOP = "/chess/assets/bb.jpg";
const char *BLACK_QUEEN = "/chess/assets/bq.jpg";
const char *BLACK_KING = "/chess/assets/bk.jpg";

// ###################### Chess Logic ######################
enum PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum PieceColor { NO_COLOR, WHITE, BLACK };

typedef enum state { HOMESCREEN, IN_GAME, GAME_OVER } State;

State game_state = HOMESCREEN;

int cursorIndex = 0;    // 0 to 63
int selectedIndex = -1; // Used for selecting a piece
bool hasSelected = false;

struct ChessPiece {
  PieceType type;
  PieceColor color;
};

ChessPiece board[8][8];
int cursorX = 0, cursorY = 0;
int selectedX = -1, selectedY = -1;

// Function declarations
void highlightCursor(int index);
void drawHomeScreen();
void drawChessBoard();
void drawAllPlaying();
void handleInputs();
void drawSdJpeg(const char *filename, int xpos, int ypos);
void jpegRender(int xpos, int ypos);
const char *getPieceImage(ChessPiece piece);
bool isValidMove(int x1, int y1, int x2, int y2);
void setupBoard();
void drawCursorHighlight(int x, int y);
void movePiece(int x1, int y1, int x2, int y2) {
  board[y2][x2] = board[y1][x1];
  board[y1][x1] = {NONE, NO_COLOR};
}

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

JpegDrawing drawing(tft);

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {
  // Initialize TFT
  tft.init();
  tft.setRotation(1); // or 3 based on your design
  tft.fillScreen(TFT_BLACK);

  // Initialize SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }

  tileSize = dim.width / 8;
  x_start = (SCREEN_WIDTH - dim.width) / 2;
  y_start = (SCREEN_HEIGHT - dim.height) / 2;

  // Place this block right after SD card check
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(CHESS_BOARD_PATH);
  tileSize = dim.width / 8; // use a global `int tileSize`
  x_start = (SCREEN_WIDTH - dim.width) / 2;
  y_start = (SCREEN_HEIGHT - dim.height) / 2;

  // Setup board pieces
  setupBoard();

  // Draw the home screen or game screen
  drawHomeScreen();
}

// ####################################################################################################
//  Main loop
// ####################################################################################################
void loop() {
  handleInputs();
  drawChessBoard();
  drawCursorHighlight(cursorX, cursorY);
  delay(100);
}

// ###################### Draw SD JPEG ######################
void drawSdJpeg(const char *filename, int xpos, int ypos) {
  File jpegFile = SD.open(filename, FILE_READ);
  if (!jpegFile) {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  if (JpegDec.decodeSdFile(jpegFile)) {
    jpegRender(xpos, ypos);
  } else {
    Serial.println("Jpeg file format not supported!");
  }
  delay(3600);
}

// ###################### Render JPEG images ######################
void jpegRender(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width + xpos;
  uint32_t max_y = JpegDec.height + ypos;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  while (JpegDec.read()) {
    pImg = JpegDec.pImage;
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    uint32_t win_w = (mcu_x + mcu_w <= max_x) ? mcu_w : max_x - mcu_x;
    uint32_t win_h = (mcu_y + mcu_h <= max_y) ? mcu_h : max_y - mcu_y;

    if ((mcu_x + win_w <= tft.width()) && (mcu_y + win_h <= tft.height()))
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort();
  }

  tft.setSwapBytes(swapBytes);
}

// ###################### Draw Chess Board ######################
void drawChessBoard() {
  drawSdJpeg(CHESS_BOARD, 0, 0);
  int tileSize = 60;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      ChessPiece p = board[y][x];
      const char *img = getPieceImage(p);
      if (img)
        drawSdJpeg(img, x * tileSize, y * tileSize);
    }
  }
}

// ###################### Get Chess Piece ######################
const char *getPieceImage(ChessPiece piece) {
  if (piece.color == WHITE) {
    switch (piece.type) {
    case PAWN:
      return WHITE_PAWN;
    case ROOK:
      return WHITE_ROOK;
    case KNIGHT:
      return WHITE_KNIGHT;
    case BISHOP:
      return WHITE_BISHOP;
    case QUEEN:
      return WHITE_QUEEN;
    case KING:
      return WHITE_KING;
    default:
      return nullptr;
    }
  } else if (piece.color == BLACK) {
    switch (piece.type) {
    case PAWN:
      return BLACK_PAWN;
    case ROOK:
      return BLACK_ROOK;
    case KNIGHT:
      return BLACK_KNIGHT;
    case BISHOP:
      return BLACK_BISHOP;
    case QUEEN:
      return BLACK_QUEEN;
    case KING:
      return BLACK_KING;
    default:
      return nullptr;
    }
  }
  return nullptr;
}

// ###################### Handle Inputs ######################
void handleInputs() {
  // if (isLeftPressed())
  //   cursorX = max(0, cursorX - 1);
  // if (isRightPressed())
  //   cursorX = min(7, cursorX + 1);
  // // Add up/down movement as needed

  // if (isAButtonPressed()) {
  //   if (!hasSelected) {
  //     if (board[cursorY][cursorX].type != NONE) {
  //       selectedX = cursorX;
  //       selectedY = cursorY;
  //       hasSelected = true;
  //     }
  //   } else {
  //     if (isValidMove(selectedX, selectedY, cursorX, cursorY)) {
  //       movePiece(selectedX, selectedY, cursorX, cursorY);
  //       hasSelected = false;
  //     }
  //   }
  // }
}

// ###################### Valid Move ######################
bool isValidMove(int x1, int y1, int x2, int y2) {
  ChessPiece piece = board[y1][x1];
  // TODO: Add rules per piece type
  return true;
}

// ###################### Setup board ######################
void setupBoard() {
  for (int x = 0; x < 8; x++) {
    board[1][x] = {PAWN, BLACK};
    board[6][x] = {PAWN, WHITE};
  }

  // Black Rows
  board[0][0] = board[0][7] = {ROOK, BLACK};
  board[0][1] = board[0][6] = {KNIGHT, BLACK};
  board[0][2] = board[0][5] = {BISHOP, BLACK};
  board[0][3] = {QUEEN, BLACK};
  board[0][4] = {KING, BLACK};

  // White Rows
  board[7][0] = board[7][7] = {ROOK, WHITE};
  board[7][1] = board[7][6] = {KNIGHT, WHITE};
  board[7][2] = board[7][5] = {BISHOP, WHITE};
  board[7][3] = {QUEEN, WHITE};
  board[7][4] = {KING, WHITE};
}

// ###################### Draw Current Box ######################
void drawCursorHighlight(int x, int y) {
  int tileSize = 60;
  tft.drawRect(x * tileSize, y * tileSize, tileSize, tileSize, TFT_YELLOW);
}

// ###################### Draw All Playing ######################
void drawAllPlaying() {
  drawChessBoard();             // Draw the board background and all pieces
  highlightCursor(cursorIndex); // Highlight current cursor square
}

// ###################### Draw Home Screen ######################
void drawHomeScreen() {
  // Your actual home screen drawing logic goes here
}

// ###################### Highlight Cursor ######################
void highlightCursor(int index) {
  int row = index / 8;
  int col = index % 8;
  int x = x_start + col * tileSize;
  int y = y_start + row * tileSize;
  tft.drawRect(x, y, tileSize, tileSize, TFT_YELLOW);
}
