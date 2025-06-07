#include <TFT_eSPI.h>
#include "../src2/Core/JpegDrawing.hpp"
#include "../src2/Core/JpegDrawing.cpp"
#include <SD.h>

TFT_eSPI tft = TFT_eSPI();
JpegDrawing jpegDrawer(tft);

// --- Pin Definitions ---
#define PIN_UP     35
#define PIN_DOWN   34
#define PIN_LEFT   34
#define PIN_RIGHT  35
#define PIN_SELECT 22

// --- Analog thresholds ---
#define LEFT_MIN   3000
#define LEFT_MAX   3400
#define DOWN_MIN   3900
#define DOWN_MAX   4200
#define UP_MIN     3000
#define UP_MAX     3400
#define RIGHT_MIN  3900
#define RIGHT_MAX  4200

const int CELL_SIZE = 30;
const int COLS = 7;
const int ROWS = 6;
const int GRID_TOP = 40;
// Playable grid area starts 15px from edge of image
const int IMAGE_X = 80;   // x offset to center 320px image on 480px screen
const int IMAGE_Y = 40;   // y offset to center 240px image on 320px screen

const int CELL_W = 40;    // 30px circle + 10px horizontal spacing
const int CELL_H = 37;    // 30px circle + 7px vertical spacing

const int PIECE_RADIUS = 14; // 30px diameter → radius = 15, minus a little padding

int board[ROWS][COLS] = {0}; // 0 = empty, 1 = red, 2 = yellow
int cursorCol = 0;
int gridOffsetX = 0;
int gridOffsetY = 0;

int player1Wins = 0;
int player2Wins = 0;
int currentPlayer = 1;

void setup();
void loop();
void drawGrid();
void drawCursor();
bool dropPiece(int col, int player);
bool checkWin(int player);
void resetGame();
void drawWinLine(int player);
void animatePieceDrop(int col, int finalRow, int player);
void drawScorePanel(int p1Wins, int p2Wins, int currentPlayer);

#include <SD.h>

void setup()
{
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  if (!SD.begin())
  {
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("SD Init Failed!");
    while (1);
  }

  gridOffsetX = (tft.width() - COLS * CELL_SIZE) / 2;
  gridOffsetY = ((tft.height() - ROWS * CELL_SIZE) / 2) + 10;

  drawGrid(); // now safe
  drawCursor();
  drawScorePanel(player1Wins, player2Wins, currentPlayer);
}

void loop() 
{
  int val34 = analogRead(34);
  int val35 = analogRead(35);

  // Move cursor LEFT
  if (val34 >= LEFT_MIN && val34 <= LEFT_MAX && cursorCol > 0) 
  {
    cursorCol--;
    delay(150);
    drawCursor();
    return;
  }

  // Move cursor RIGHT
  if (val35 >= RIGHT_MIN && val35 <= RIGHT_MAX && cursorCol < COLS - 1) 
  {
    cursorCol++;
    delay(150);
    drawCursor();
    return;
  }

  // Drop piece with SELECT
  if (digitalRead(PIN_SELECT) == HIGH) 
  {
    bool placed = dropPiece(cursorCol, currentPlayer);

    if (placed) 
    {
      drawGrid();
      drawCursor();

      if (checkWin(currentPlayer)) 
      {
        if (currentPlayer == 1)
        {
          player1Wins++;
        }
        else
        {
          player2Wins++;
        }

        drawScorePanel(player1Wins, player2Wins, currentPlayer);  // update display

        drawWinLine(currentPlayer);
        delay(1500);

        uint16_t bgColor = tft.color565(32, 58, 66);
        tft.fillScreen(bgColor);

        jpegDrawer.drawSdJpeg("/connect4/connectBoard.jpg", IMAGE_X, IMAGE_Y);
        jpegDrawer.pushSprite(true, TFT_WHITE);  // Or bgColor if needed — doesn't matter now



        tft.setTextColor(currentPlayer == 1 ? TFT_RED : TFT_BLUE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(3);
        tft.drawString("Player " + String(currentPlayer) + " Wins!", tft.width() / 2, tft.height() / 2 - 20);

        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Press SELECT to restart", tft.width() / 2, tft.height() / 2 + 20);

        // Wait for SELECT press
        while (true)
        {
          if (digitalRead(PIN_SELECT) == HIGH) 
          {
            delay(300);  // debounce
            resetGame();
            break;
          }
        }
        return;
      }

      // Switch player after successful and non-winning move
      currentPlayer = (currentPlayer == 1) ? 2 : 1;
      drawScorePanel(player1Wins, player2Wins, currentPlayer);
    }

    return;
  }
}

void drawGrid()
{
  // Explicit background fill before any JPEGs
  uint16_t bgColor = tft.color565(32, 58, 66);
  tft.fillScreen(bgColor);

// Now draw your board on top of that
  jpegDrawer.drawSdJpeg("/connect4/connectBoard.jpg", IMAGE_X, IMAGE_Y);
jpegDrawer.pushSprite(true, TFT_WHITE);  // Or bgColor if needed — doesn't matter now
  tft.setTextColor(TFT_WHITE);

  for (int r = 0; r < ROWS; r++) 
  {
    for (int c = 0; c < COLS; c++) 
    {
      int cx = IMAGE_X + 15 + c * CELL_W + 25;
      int cy = IMAGE_Y + 15 + r * CELL_H + 12;
      int px = cx - 15;
      int py = cy - 15;

      if (board[r][c] == 1) {
        jpegDrawer.drawSdJpeg("/connect4/redPiece.jpg", px, py);
        jpegDrawer.pushSprite(true, bgColor);
      } 
      else if (board[r][c] == 2) {
        jpegDrawer.drawSdJpeg("/connect4/bluePiece.jpg", px, py);
        jpegDrawer.pushSprite(true, bgColor);
      }
    }
  }
}

void drawCursor()
{
  tft.fillRect(0, 0, tft.width(), 40, TFT_BLACK);  // clear top area

  int x = IMAGE_X + 15 + cursorCol * CELL_W + 25;  // left edge of column
  int y = IMAGE_Y - 20;                       // above board

  tft.fillTriangle(x - 8, y, x + 8, y, x, y + 13, TFT_GREEN);
}

bool dropPiece(int col, int player) {
  for (int r = ROWS - 1; r >= 0; r--) {
    if (board[r][col] == 0) {
      animatePieceDrop(col, r, player);  // 👈 animate it
      board[r][col] = player;
      return true;
    }
  }
  return false;  // Column full
}

// These global variables are used to draw the win line
int winStartRow, winStartCol;
int winDirRow, winDirCol;

bool checkWin(int player)
{
  // Horizontal check
  for (int r = 0; r < ROWS; r++)
  {
    for (int c = 0; c < COLS - 3; c++)
    {
      if (board[r][c] == player &&
          board[r][c + 1] == player &&
          board[r][c + 2] == player &&
          board[r][c + 3] == player)
      {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 0;
        winDirCol = 1;
        return true;
      }
    }
  }

  // Vertical check
  for (int c = 0; c < COLS; c++) 
  {
    for (int r = 0; r < ROWS - 3; r++)
    {
      if (board[r][c] == player &&
          board[r + 1][c] == player &&
          board[r + 2][c] == player &&
          board[r + 3][c] == player)
      {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 1;
        winDirCol = 0;
        return true;
      }
    }
  }

  // Diagonal (\) check
  for (int r = 0; r < ROWS - 3; r++)
  {
    for (int c = 0; c < COLS - 3; c++)
    {
      if (board[r][c] == player &&
          board[r+1][c+1] == player &&
          board[r+2][c+2] == player &&
          board[r+3][c+3] == player)
      {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 1;
        winDirCol = 1;
        return true;
      }
    }
  }

  // Diagonal (/) check
  for (int r = 3; r < ROWS; r++)
  {
    for (int c = 0; c < COLS - 3; c++)
    {
      if (board[r][c] == player &&
          board[r-1][c+1] == player &&
          board[r-2][c+2] == player &&
          board[r-3][c+3] == player)
      {
        winStartRow = r;
        winStartCol = c;
        winDirRow = -1;
        winDirCol = 1;
        return true;
      }
    }
  }
  return false;  // no win found
}

void resetGame() 
{
  for (int r = 0; r < ROWS; r++)
    for (int c = 0; c < COLS; c++)
      board[r][c] = 0;

  cursorCol = 0;
  currentPlayer = 1;

  uint16_t bgColor = tft.color565(32, 58, 66);
  tft.fillScreen(bgColor);
  jpegDrawer.drawSdJpeg("/connect4/connectBoard.jpg", IMAGE_X, IMAGE_Y);
  jpegDrawer.pushSprite(true, bgColor);

  drawGrid();
  drawCursor();
  drawScorePanel(player1Wins, player2Wins, currentPlayer);
}

void drawWinLine(int player)
{
  uint16_t color = (player == 1) ? TFT_RED : TFT_BLUE;

  // Start cell
  int startCol = winStartCol;
  int startRow = winStartRow;

  // End cell = start + 3 steps in the win direction
  int endCol = winStartCol + winDirCol * 3;
  int endRow = winStartRow + winDirRow * 3;

  // Compute exact pixel center of each cell (same as piece draw logic)
  int x1 = IMAGE_X + 15 + startCol * CELL_W + 25;
  int y1 = IMAGE_Y + 15 + startRow * CELL_H + 12;

  int x2 = IMAGE_X + 15 + endCol * CELL_W + 25;
  int y2 = IMAGE_Y + 15 + endRow * CELL_H + 12;

  // Draw a thick line (3px)
  for (int offset = -1; offset <= 1; offset++) {
    tft.drawLine(x1 + offset, y1, x2 + offset, y2, color);  // horizontal thickness
    tft.drawLine(x1, y1 + offset, x2, y2 + offset, color);  // vertical thickness
  }
}

void animatePieceDrop(int col, int targetRow, int player)
{
  String piecePath = (player == 1) ? "/connect4/redPiece.jpg" : "/connect4/bluePiece.jpg";
  String emptyPath = "/connect4/empty.jpg";
  uint16_t bgColor = tft.color565(32, 58, 66);  // Match background

  int prevPx = -1, prevPy = -1;

  for (int r = 0; r <= targetRow; r++)
  {
    int cx = IMAGE_X + 15 + col * CELL_W + 25;
    int cy = IMAGE_Y + 15 + r * CELL_H + 12;
    int px = cx - 15;
    int py = cy - 15;

    if (r > 0)
    {
      jpegDrawer.drawSdJpeg(emptyPath.c_str(), prevPx, prevPy);
      jpegDrawer.pushSprite(true, TFT_WHITE);
    }

    jpegDrawer.drawSdJpeg(piecePath.c_str(), px, py);
    jpegDrawer.pushSprite(true, TFT_WHITE);

    prevPx = px;
    prevPy = py;

    delay(20);
  }
}

void drawScorePanel(int p1Wins, int p2Wins, int currentPlayer)
{
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);

  // Player 1 on the left
  tft.setTextColor(currentPlayer == 1 ? TFT_RED : TFT_LIGHTGREY);
  tft.fillRect(0, 40, 60, 60, TFT_BLACK);  // clear area
  tft.drawString("P1", 20, 45);
  tft.drawString(String(p1Wins), 25, 70);
  tft.drawLine(15, 63, 10 + tft.textWidth("P1", 2), 63, currentPlayer == 1 ? TFT_RED : TFT_LIGHTGREY);

  // Player 2 on the right
  tft.setTextColor(currentPlayer == 2 ? TFT_BLUE : TFT_LIGHTGREY);
  tft.fillRect(420, 40, 60, 60, TFT_BLACK);  // clear area
  tft.drawString("P2", 430, 45);
  tft.drawString(String(p2Wins), 440, 70);
  tft.drawLine(425, 63, 430 + tft.textWidth("P2", 2), 63, currentPlayer == 2 ? TFT_BLUE : TFT_LIGHTGREY);
}
