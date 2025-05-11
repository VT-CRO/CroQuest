#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// Pin definitions
#define BTN_UP     18
#define BTN_DOWN   22
#define BTN_LEFT   21
#define BTN_RIGHT  19
#define BTN_SELECT 23

String board[9] = { "**", "**", "**", "**", "**", "**", "**", "**", "**" };
char currentPlayer = 'X';
char winner = 'N';  // 'X', 'O', 'D' (draw), or 'N' (none)
int winCombo[3] = {-1, -1, -1};  // indices of the winning 3 cells
unsigned long winTime = 0;
bool gameEnded = false;

// Cursor position (0–8)
int cursorIndex = 0;

int screen_width, screen_height;
const int cell_size = 60;
int x_start, y_start;  // computed in setup()

struct Move {
  int index;
  char symbol;
};

Move moveQueue[6];  // FIFO queue of last 6 moves
int moveCount = 0;  // total moves placed

int xWins = 0;
int oWins = 0;

void setup() {
  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  // Get screen dimensions dynamically
  screen_width = tft.width();    // e.g., 240 or 320
  screen_height = tft.height();  // e.g., 320

  // Compute board position to center it
  x_start = (screen_width - 3 * cell_size) / 2;
  y_start = (screen_height - 3 * cell_size) / 2;

  // Initialize buttons
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  // Draw initial screen
  drawScoreboard();
  drawGrid();
  highlightCursor(cursorIndex);
}

void loop() {
  static int lastCursor = -1;
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;

  // Cursor Movement
  if (!gameEnded && millis() - lastMoveTime > moveDelay) {
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
  static bool buttonPreviouslyPressed = false;
  bool selectPressed = digitalRead(BTN_SELECT) == LOW;

  if (!gameEnded && selectPressed && !buttonPreviouslyPressed && board[cursorIndex] == "**") {
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
    tft.fillScreen(TFT_WHITE);
    drawScoreboard();
    drawGrid();
    highlightCursor(cursorIndex);
    drawWinLine();
    if (gameEnded) drawWinnerMessage();
    lastCursor = cursorIndex;
  }

  // Auto Restart
  if (gameEnded && millis() - winTime >= 5000) {
    for (int i = 0; i < 9; i++) board[i] = "**";
    currentPlayer = 'X';
    cursorIndex = 0;
    lastCursor = -1;
    winner = 'N';
    winCombo[0] = winCombo[1] = winCombo[2] = -1;
    gameEnded = false;
    moveCount = 0;
    tft.fillScreen(TFT_WHITE);
    drawScoreboard();
    drawGrid();
    highlightCursor(cursorIndex);
  }
}

void drawGrid() {
  // Draw grid lines
  for (int i = 1; i < 3; i++) 
  {
    tft.drawLine(x_start + i * cell_size, y_start, x_start + i * cell_size, y_start + 3 * cell_size, TFT_BLACK);
    tft.drawLine(x_start, y_start + i * cell_size, x_start + 3 * cell_size, y_start + i * cell_size, TFT_BLACK);
  }

  tft.drawRect(x_start, y_start, 3 * cell_size, 3 * cell_size, TFT_BLACK);

  // Draw current board state
  for (int i = 0; i < 9; i++) {
    if (board[i] != "**") {
      int row = i / 3;
      int col = i % 3;
      int x = x_start + col * cell_size + cell_size / 2;
      int y = y_start + row * cell_size + cell_size / 2;

      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(board[i] == "X" ? TFT_RED : TFT_BLUE, TFT_WHITE);
      tft.setTextSize(4);
      tft.drawString(board[i], x, y);
    }
  }
}

void highlightCursor(int index) {
  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size;
  int y = y_start + row * cell_size;

  tft.drawRect(x, y, cell_size, cell_size, TFT_GREEN);
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
      gameEnded = true;

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
    gameEnded = true;
  }
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

  for (int offset = -1; offset <= 1; offset++) {
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
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);

  tft.setTextColor(TFT_RED, TFT_WHITE);
  tft.drawString("X: " + String(xWins), 5, 5);

  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.drawString("O: " + String(oWins), tft.width() - 60, 5);
}


