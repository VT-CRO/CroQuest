#include <TFT_eSPI.h>
#include "JpegDrawing.hpp"
TFT_eSPI tft = TFT_eSPI();
JpegDrawing drawing(tft);
//Assets
const char* BOARD_PATH = "/tic_tac_toe_assets/board.jpg";
const char* X_PATH = "/tic_tac_toe_assets/x.jpg";
const char* O_PATH = "/tic_tac_toe_assets/o.jpg";
const char* DIS_O_PATH = "/tic_tac_toe_assets/disappearing_o.jpg";
const char* DIS_X_PATH = "/tic_tac_toe_assets/disappearing_x.jpg";

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// Pin definitions
#define BTN_UP     35
#define BTN_DOWN   34
#define BTN_LEFT   36
#define BTN_RIGHT  39
#define BTN_SELECT 21
#define SD_CS 5

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

String board[9] = { "**", "**", "**", "**", "**", "**", "**", "**", "**" };
char currentPlayer = 'X';
char winner = 'N';  // 'X', 'O', 'D' (draw), or 'N' (none)
int winCombo[3] = {-1, -1, -1};  // indices of the winning 3 cells
unsigned long winTime = 0;
bool roundEnded = false;

// Cursor position (0–8)
int cursorIndex = 0;

int screen_width, screen_height;
const int cell_size = 80;
int x_start, y_start;  // computed in setup()

struct Move {
  int index;
  char symbol;
};

Move moveQueue[6];  // FIFO queue of last 6 moves
int moveCount = 0;  // total moves placed

//Player score
int xWins = 0;
int oWins = 0;

//Background orange color
uint16_t orange_color = tft.color565(0xFF, 0x70, 0x00);

static bool buttonPreviouslyPressed = false;

//Gamestate
typedef enum state{ 
            HOMESCREEN, 
            MULTIPLAYER, 
            SINGLE_PLAYER,
            GAMEOVER_SCREEN,
          }State;

State game_state = HOMESCREEN;

void setup() {
  // Initialize display
  tft.init();
  tft.setRotation(1);

  tft.fillScreen(orange_color);

  // Get screen dimensions dynamically
  screen_width = tft.width();    // e.g., 240 or 320
  screen_height = tft.height();  // e.g., 320

  // Initialize buttons
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  // Compute board position to center it
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(BOARD_PATH); 
  x_start = (SCREEN_WIDTH - dim.width) / 2;
  y_start = (SCREEN_HEIGHT - dim.height) / 2;

  drawHomeScreen();
}

void loop() {
  static int lastCursor = -1;
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;

  if(game_state == HOMESCREEN){
    if(millis() - lastMoveTime > moveDelay/2){
      if(digitalRead(BTN_SELECT) == LOW){
        game_state = SINGLE_PLAYER;
        // Clear the screen with orange background
        tft.fillScreen(orange_color);
        // Draw initial screen
        drawAllPlaying();
      }
      lastMoveTime = millis();
    }
  }
  else if(game_state == SINGLE_PLAYER){
    if (!roundEnded && millis() - lastMoveTime > moveDelay) {
      if (digitalRead(BTN_UP) == LOW && cursorIndex >= 3) {
        cursorIndex -= 3;
        lastMoveTime = millis();
      } else if (digitalRead(BTN_DOWN) == LOW && cursorIndex <= 5) {
        cursorIndex += 3;
        lastMoveTime = millis();
      } else if (digitalRead(BTN_LEFT) == LOW && cursorIndex % 3 != 0) {
        cursorIndex -= 1;
        lastMoveTime = millis();
      } else if (digitalRead(BTN_RIGHT) == LOW && cursorIndex % 3 != 2) {
        cursorIndex += 1;
        lastMoveTime = millis();
      }
    }

    // Piece Placement
    bool selectPressed = digitalRead(BTN_SELECT) == LOW;

    if (!roundEnded && selectPressed && !buttonPreviouslyPressed && board[cursorIndex] == "**") {
      if (moveCount >= 6) {
        int oldIndex = moveQueue[0].index;
        board[oldIndex] = "**";
        for (int i = 1; i < 6; i++) moveQueue[i - 1] = moveQueue[i];
        moveCount = 5;
      }

      board[cursorIndex] = String(currentPlayer);
      moveQueue[moveCount].index = cursorIndex;
      moveQueue[moveCount].symbol = currentPlayer;
      moveCount++;

      currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
      checkWinner();
    }

    buttonPreviouslyPressed = selectPressed;

    // Redraw
    if (cursorIndex != lastCursor || selectPressed) {
      drawAllPlaying();
      drawWinLine();
      if (roundEnded) drawWinnerMessage();
      lastCursor = cursorIndex;
    }
    // Auto Restart
    if (roundEnded && millis() - winTime >= 5000 && xWins < 2 && oWins < 2) {
      for (int i = 0; i < 9; i++) board[i] = "**";
      currentPlayer = 'X';
      cursorIndex = 0;
      lastCursor = -1;
      winner = 'N';
      winCombo[0] = winCombo[1] = winCombo[2] = -1;
      roundEnded = false;
      moveCount = 0;
      tft.fillScreen(orange_color);
      drawAllPlaying();
    }
    else if(xWins >= 2 || oWins >= 2){
      game_state = GAMEOVER_SCREEN;
      for (int i = 0; i < 9; i++) board[i] = "**";
      currentPlayer = 'X';
      cursorIndex = 0;
      lastCursor = -1;
      winner = 'N';
      winCombo[0] = winCombo[1] = winCombo[2] = -1;
      roundEnded = false;
      moveCount = 0;
      // Clear the screen with orange background
      tft.fillScreen(orange_color);
    }
  }
  else if(game_state == GAMEOVER_SCREEN){
    drawEndScreen();
    if(millis() - lastMoveTime > moveDelay){
      if(digitalRead(BTN_SELECT) == LOW){
        game_state = HOMESCREEN;
        oWins = 0;
        xWins = 0;

        // Draw homescreen
        drawHomeScreen();
      }
      lastMoveTime = millis();
    }
  }
}

void checkWinner() {
  const int wins[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // cols
    {0, 4, 8}, {2, 4, 6}             // diagonals
  };

  for (int i = 0; i < 8; i++) {
    String a = board[wins[i][0]];
    String b = board[wins[i][1]];
    String c = board[wins[i][2]];

    if (a != "**" && a == b && b == c) {
      winner = a.charAt(0);
      winCombo[0] = wins[i][0];
      winCombo[1] = wins[i][1];
      winCombo[2] = wins[i][2];
      winTime = millis();
      roundEnded = true;

      if (winner == 'X') xWins++;
      else if (winner == 'O') oWins++;

      return;
    }
  }

  // Check draw
  bool full = true;
  for (int i = 0; i < 9; i++) {
    if (board[i] == "**") {
      full = false;
      break;
    }
  }
  if (full) {
    winner = 'D';
    winTime = millis();
    roundEnded = true;
  }
}

///////// DRAWING //////////////////

void drawEndScreen() {
  
  // Set text properties
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  
  // Display winner
  String winnerText = (xWins > oWins) ? "X WINS THE GAME!" : "O WINS THE GAME!";
  tft.drawString(winnerText, SCREEN_WIDTH / 2, 80);
  
  // Display final score
  tft.setTextSize(3);
  tft.drawString("FINAL SCORE", SCREEN_WIDTH / 2, 130);
  
  // Draw score display
  int scoreYPos = 180;
  int xPos1 = SCREEN_WIDTH / 2 - 80;
  int xPos2 = SCREEN_WIDTH / 2 + 80;
  
  // X score
  tft.setTextSize(5);
  tft.drawString("X", xPos1, scoreYPos);
  tft.drawString(String(xWins), xPos1, scoreYPos + 50);
  
  // Divider
  tft.drawString("-", SCREEN_WIDTH / 2, scoreYPos);
  
  // O score
  tft.drawString("O", xPos2, scoreYPos);
  tft.drawString(String(oWins), xPos2, scoreYPos + 50);
  
  // Instructions to continue
  tft.setTextSize(2);
  tft.drawString("Press SELECT to return to menu", SCREEN_WIDTH / 2, 280);
  
  // Draw trophy next to winner's symbol
  if (xWins > oWins) {
    // Draw small graphic for winner (could use a custom trophy JPG if available)
    tft.fillTriangle(xPos1 - 40, scoreYPos, xPos1 - 60, scoreYPos + 20, xPos1 - 20, scoreYPos + 20, TFT_YELLOW);
  } else {
    // Draw small graphic for winner
    tft.fillTriangle(xPos2 + 40, scoreYPos, xPos2 + 60, scoreYPos + 20, xPos2 + 20, scoreYPos + 20, TFT_YELLOW);
  }
}


void drawHomeScreen() {
  // Clear the screen with orange background
  tft.fillScreen(orange_color);
  
  // Set text properties for title
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  
  // Draw game title
  tft.drawString("TIC TAC TOE", SCREEN_WIDTH / 2, 60);
  
  // Draw tic-tac-toe grid manually in center
  int gridSize = 90; // Total grid size
  int cellSize = gridSize / 3;
  int gridX = (SCREEN_WIDTH - gridSize) / 2;
  int gridY = 50;
  
  // Draw the grid lines with thickness of 5 pixels
  // Vertical lines
  tft.fillRect(gridX + cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  tft.fillRect(gridX + 2*cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  
  // Horizontal lines
  tft.fillRect(gridX, gridY + cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
  tft.fillRect(gridX, gridY + 2*cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
  
  // Display instructions
  tft.setTextSize(2);
  tft.drawString("Press SELECT to start", SCREEN_WIDTH / 2, 260);
  tft.drawString("Best of Three!", SCREEN_WIDTH / 2, 290);
}

void drawAllPlaying(){
  drawScoreboard();
  drawGrid();
  drawing.pushSprite();
  highlightCursor(cursorIndex);
}

void drawGrid() {

  drawing.drawSdJpeg(BOARD_PATH, x_start, y_start);
  
  // Draw current board state
  for (int i = 0; i < 9; i++) {
    if (board[i] != "**") {
      int row = i / 3;
      int col = i % 3;
      int x = col * cell_size + cell_size/3 - 3;
      int y = row * cell_size + cell_size/3 - 3;

      if(board[i] == "X"){
        if(moveCount >= 5 && moveQueue[0].index == i){
          drawing.drawSdJpeg(DIS_X_PATH, x, y);
        }else{
          drawing.drawSdJpeg(X_PATH, x, y);
        }
      }else{
        if(moveCount >= 5 && moveQueue[0].index == i){
          drawing.drawSdJpeg(DIS_O_PATH, x, y);
        }else{
          drawing.drawSdJpeg(O_PATH, x, y);
        }
      }
    }
  }
}

void highlightCursor(int index) {
  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size + cell_size/3 - 3;
  int y = y_start + row * cell_size + cell_size/3 - 3;

  tft.drawRect(x, y, cell_size - 30, cell_size - 30, TFT_WHITE);
}

void clearCursor(int index) {
  if (index < 0 || index > 8) return;

  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size;
  int y = y_start + row * cell_size;

  // Fully clear and redraw the tile as white
  tft.fillRect(x, y, cell_size, cell_size, TFT_WHITE);
  tft.drawRect(x, y, cell_size, cell_size, TFT_BLACK);

  // Redraw the grid lines manually if needed (for crossovers)
  if (col > 0) tft.drawLine(x, y, x, y + cell_size, TFT_BLACK); // left
  if (col < 2) tft.drawLine(x + cell_size, y, x + cell_size, y + cell_size, TFT_BLACK); // right
  if (row > 0) tft.drawLine(x, y, x + cell_size, y, TFT_BLACK); // top
  if (row < 2) tft.drawLine(x, y + cell_size, x + cell_size, y + cell_size, TFT_BLACK); // bottom
}

void drawWinLine() {
  if (winner != 'X' && winner != 'O') return;

  int i1 = winCombo[0];
  int i3 = winCombo[2];

  int row1 = i1 / 3, col1 = i1 % 3;
  int row3 = i3 / 3, col3 = i3 % 3;

  int x1 = x_start + col1 * cell_size + cell_size / 2;
  int y1 = y_start + row1 * cell_size + cell_size / 2;
  int x3 = x_start + col3 * cell_size + cell_size / 2;
  int y3 = y_start + row3 * cell_size + cell_size / 2;

  uint16_t color = (winner == 'X') ? TFT_RED : TFT_BLUE;

  for (int offset = -2; offset <= 2; offset++) {
    tft.drawLine(x1 + offset, y1, x3 + offset, y3, color);
    tft.drawLine(x1, y1 + offset, x3, y3 + offset, color);
  }
}

void drawWinnerMessage() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  String msg = "";
  if (winner == 'X') msg = "X Wins!";
  else if (winner == 'O') msg = "O Wins!";
  else if (winner == 'D') msg = "Draw!";

  tft.drawString(msg, tft.width() / 2, 30);  // above grid
}

void drawScoreboard() {
  int centerY = tft.height() / 2;
  int padding = 20;

  // Settings for big scoreboard
  int textSize = 4;
  tft.setTextSize(textSize);
  tft.setTextDatum(MC_DATUM);

  int underlineWidth = 40;
  int underlineThickness = 4;   // <== THICKNESS OF THE LINE
  int underlineOffset = 24;     // Vertical distance from text to line
  int scoreOffset = 32;         // Distance from underline to score

  // === X Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xX = padding + underlineWidth;
  int yX = centerY - (underlineOffset + scoreOffset) / 2;
  tft.drawString("X", xX, yX);

  // Thick underline using fillRect
  int xLineY = yX + underlineOffset;
  tft.fillRect(xX - underlineWidth / 2, xLineY, underlineWidth, underlineThickness, TFT_WHITE);

  // Score for X
  tft.drawString(String(xWins), xX, xLineY + scoreOffset);

  // === O Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xO = tft.width() - padding - underlineWidth;
  int yO = yX;
  tft.drawString("O", xO, yO);

  // Thick underline using fillRect
  int oLineY = yO + underlineOffset;
  tft.fillRect(xO - underlineWidth / 2, oLineY, underlineWidth, underlineThickness, TFT_WHITE);

  // Score for O
  tft.drawString(String(oWins), xO, oLineY + scoreOffset);
}