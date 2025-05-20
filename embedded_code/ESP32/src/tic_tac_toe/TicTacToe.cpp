// src/tic_tac_toe/TicTacToe.cpp

#include "TicTacToe.hpp"

// ======================== Global Definitions ========================

static const char *BOARD_PATH = "/tic_tac_toe_assets/board.jpg";
static const char *X_PATH = "/tic_tac_toe_assets/x.jpg";
static const char *O_PATH = "/tic_tac_toe_assets/o.jpg";
static const char *DIS_O_PATH = "/tic_tac_toe_assets/disappearing_o.jpg";
static const char *DIS_X_PATH = "/tic_tac_toe_assets/disappearing_x.jpg";

// Game Board
static String board[9] = {"**", "**", "**", "**", "**", "**", "**", "**", "**"};
static Move moveQueue[6];
static int moveCount = 0;
static int cursorIndex = 0;
static char currentPlayer = 'X';
static char winner = 'N';
static int winCombo[3] = {-1, -1, -1};
static bool roundEnded = false;
static unsigned long winTime = 0;

// Game State
State game_state = HOMESCREEN;
static int selection = 0;
static int subselection = 0;
static const unsigned long moveDelay = 100;
static bool buttonPreviouslyPressed = false;
static bool firstDrawingPlaying = true;

// Screen
int screen_width, screen_height;
static const int cell_size = 80;
static int x_start, y_start;
static uint16_t orange_color = tft.color565(0xFF, 0x70, 0x00);

// Score
static int xWins = 0;
static int oWins = 0;

// Jpeg and Numpad initialization
JpegDrawing drawing(tft);
NumPad pad(drawHomeScreen, drawAllPlaying, &game_state, HOMESCREEN, PLAYING);

// ======================== Game Entry ========================
void runTicTacToe() {
  tft.setRotation(3);
  tft.fillScreen(orange_color);

  screen_width = tft.width();
  screen_height = tft.height();

  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(BOARD_PATH);
  x_start = (screen_width - dim.width) / 2;
  y_start = (screen_height - dim.height) / 2;

  drawHomeScreen();

  while (true) {
    handleTicTacToeFrame();

    if (game_state == HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu from Tic Tac Toe");
      delay(500);
      return;
    }
  }
}

// ========== MANUAL LOOP ==========
void handleTicTacToeFrame() {
  static int lastCursor = -1;
  static unsigned long lastMoveTime = 0;

  if (game_state == HOMESCREEN) {
    if (millis() - lastMoveTime > moveDelay / 2) {
      if (A.wasJustPressed()) {
        if (selection == 0) {
          game_state = PLAYING;
          // Draw initial screen
          drawAllPlaying();
        } else if (selection == 1) {
          game_state = MULTIPLAYER_SELECTION;
          drawHomescreenSelect();
        }
      }
      // Selection logic
      if (up.isPressed()) {
        selection = 0;
        drawHomescreenSelect();
      } else if (down.isPressed()) {
        selection = 1;
        drawHomescreenSelect();
      }
      lastMoveTime = millis();
    }
  } else if (game_state == MULTIPLAYER_SELECTION) {
    if (!roundEnded && millis() - lastMoveTime > moveDelay) {
      if (A.wasJustPressed()) {
        if (subselection == 0) {
          game_state = JOIN_SCREEN;
          tft.fillScreen(TFT_BLUE);
        } else {
          game_state = BLUETOOTH_NUMPAD;
          pad.numPadSetup();
        }
      }
      if (up.isPressed()) {
        game_state = HOMESCREEN;
        drawHomescreenSelect();
      } else if (left.isPressed()) {
        if (subselection == 1) {
          subselection = 0;
          drawHomescreenSelect();
        }
      } else if (right.isPressed()) {
        if (subselection == 0) {
          subselection = 1;
          drawHomescreenSelect();
        }
      }
      lastMoveTime = millis();
    }
  } else if (game_state == PLAYING) {
    if (!roundEnded && millis() - lastMoveTime > moveDelay / 2) {
      if (up.isPressed() && cursorIndex >= 3) {
        cursorIndex -= 3;
        lastMoveTime = millis();
      } else if (down.isPressed() && cursorIndex <= 5) {
        cursorIndex += 3;
        lastMoveTime = millis();
      } else if (left.isPressed() && cursorIndex % 3 != 0) {
        cursorIndex -= 1;
        lastMoveTime = millis();
      } else if (right.isPressed() && cursorIndex % 3 != 2) {
        cursorIndex += 1;
        lastMoveTime = millis();
      }
    }

    // Piece Placement
    bool selectPressed = A.wasJustPressed();

    if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
        board[cursorIndex] == "**") {
      if (moveCount >= 6) {
        int oldIndex = moveQueue[0].index;
        board[oldIndex] = "**";
        for (int i = 1; i < 6; i++)
          moveQueue[i - 1] = moveQueue[i];
        moveCount = 5;
      }

      board[cursorIndex] = String(currentPlayer);
      moveQueue[moveCount].index = cursorIndex;
      moveQueue[moveCount].symbol = currentPlayer;
      moveCount++;

      currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
      checkWinner();

      // Player's move render before AI thinks
      drawAllPlaying();
      drawWinLine();
      if (roundEnded)
        drawWinnerMessage();

      delay(400);
    }

    if (currentPlayer == 'O' && !roundEnded) {
      delay(300); // Optional: makes AI feel more human

      int aiMove = findBestMove('O', 'X');
      if (aiMove != -1) {
        if (moveCount >= 6) {
          int oldIndex = moveQueue[0].index;
          board[oldIndex] = "**";
          for (int i = 1; i < 6; i++)
            moveQueue[i - 1] = moveQueue[i];
          moveCount = 5;
        }

        board[aiMove] = "O";
        moveQueue[moveCount].index = aiMove;
        moveQueue[moveCount].symbol = 'O';
        moveCount++;

        currentPlayer = 'X';
        checkWinner();

        drawAllPlaying();
        drawWinLine();
        if (roundEnded)
          drawWinnerMessage();
      }
    }

    buttonPreviouslyPressed = selectPressed;

    // Redraw
    if (cursorIndex != lastCursor || selectPressed) {
      drawAllPlaying();
      drawWinLine();
      if (roundEnded)
        drawWinnerMessage();
      lastCursor = cursorIndex;
    }
    // Auto Restart
    if (roundEnded && millis() - winTime >= 5000 && xWins < 2 && oWins < 2) {
      for (int i = 0; i < 9; i++)
        board[i] = "**";
      currentPlayer = 'X';
      cursorIndex = 0;
      lastCursor = -1;
      winner = 'N';
      winCombo[0] = winCombo[1] = winCombo[2] = -1;
      roundEnded = false;
      moveCount = 0;
      tft.fillScreen(orange_color);
      drawAllPlaying();
    } else if (xWins >= 2 || oWins >= 2) {
      game_state = GAMEOVER_SCREEN;
      for (int i = 0; i < 9; i++)
        board[i] = "**";
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
  } else if (game_state == GAMEOVER_SCREEN) {
    firstDrawingPlaying = true;
    drawEndScreen();
    if (millis() - lastMoveTime > moveDelay) {
      if (A.wasJustPressed()) {
        game_state = HOMESCREEN;
        oWins = 0;
        xWins = 0;

        // Draw homescreen
        drawHomeScreen();
      }
      lastMoveTime = millis();
    }
  } else if (game_state == BLUETOOTH_NUMPAD) {
    pad.handleButtonInput(&lastMoveTime, moveDelay);
  }
}

static void checkWinner() {
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

      if (winner == 'X')
        xWins++;
      else if (winner == 'O')
        oWins++;

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

static void drawEndScreen() {

  // Set text properties
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);

  // Display winner
  String winnerText = (xWins > oWins) ? "X WINS THE GAME!" : "O WINS THE GAME!";
  tft.drawString(winnerText, screen_width / 2, 80);

  // Display final score
  tft.setTextSize(3);
  tft.drawString("FINAL SCORE", screen_width / 2, 130);

  // Draw score display
  int scoreYPos = 180;
  int xPos1 = screen_width / 2 - 80;
  int xPos2 = screen_width / 2 + 80;

  // X score
  tft.setTextSize(5);
  tft.drawString("X", xPos1, scoreYPos);
  tft.drawString(String(xWins), xPos1, scoreYPos + 50);

  // Divider
  tft.drawString("-", screen_width / 2, scoreYPos);

  // O score
  tft.drawString("O", xPos2, scoreYPos);
  tft.drawString(String(oWins), xPos2, scoreYPos + 50);

  // Instructions to continue
  tft.setTextSize(2);
  tft.drawString("Press SELECT to return to menu", screen_width / 2, 280);

  // Draw trophy next to winner's symbol
  if (xWins > oWins) {
    // Draw small graphic for winner (could use a custom trophy JPG if
    // available)
    tft.fillTriangle(xPos1 - 40, scoreYPos, xPos1 - 60, scoreYPos + 20,
                     xPos1 - 20, scoreYPos + 20, TFT_YELLOW);
  } else {
    // Draw small graphic for winner
    tft.fillTriangle(xPos2 + 40, scoreYPos, xPos2 + 60, scoreYPos + 20,
                     xPos2 + 20, scoreYPos + 20, TFT_YELLOW);
  }
}

static void drawHomeScreen() {
  // Clear the screen with orange background
  tft.fillScreen(orange_color);

  // Set text properties for title
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);

  // Draw game title
  tft.drawString("TIC TAC TOE", screen_width / 2, 40);

  // Draw tic-tac-toe grid manually in center
  int gridSize = 90; // Total grid size
  int cellSize = gridSize / 3;
  int gridX = (screen_width - gridSize) / 2;
  int gridY = 30;

  // Draw the grid lines with thickness of 5 pixels
  // Vertical lines
  tft.fillRect(gridX + cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  tft.fillRect(gridX + 2 * cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);

  // Horizontal lines
  tft.fillRect(gridX, gridY + cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
  tft.fillRect(gridX, gridY + 2 * cellSize - 2 + 50, gridSize, 5, TFT_WHITE);

  // Display instructions
  tft.setTextSize(2);
  tft.drawString("Press for Single-Player", screen_width / 2, 200);
  tft.drawString("Press for Multiplayer", screen_width / 2, 250);

  drawHomescreenSelect();
}

static void drawHomescreenSelect() {
  int y_single = 200;
  int y_multi = 250;
  int y_sub1 = y_multi + 20;
  int y_sub2 = y_multi + 40;

  // Clear areas
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1); // reset default

  tft.fillRect(0, y_single - 15, screen_width, 35, orange_color);
  tft.fillRect(0, y_multi - 15, screen_width, 80,
               orange_color); // extra space for sub-options

  if (selection == 0) {
    // Single-player selected
    tft.setTextSize(3);
    tft.drawString("Press for Single-Player", screen_width / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Press for Multiplayer", screen_width / 2, y_multi);
  } else {
    // Multiplayer selected
    tft.setTextSize(2);
    tft.drawString("Press for Single-Player", screen_width / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Press for Multiplayer", screen_width / 2, y_multi);

    // Draw sub-options
    tft.setTextSize(1);
    const char *sub1 = "Press to Host a Game";
    const char *sub2 = "Press to Join a Game";

    int sub1Width = tft.textWidth(sub1);
    int sub2Width = tft.textWidth(sub2);

    // Draw highlight rectangles for selected sub-option
    if (game_state == MULTIPLAYER_SELECTION) {
      const char *sub1 = "Host a Game";
      const char *sub2 = "Join a Game";

      int textSize = 2;
      tft.setTextSize(textSize);
      tft.setTextDatum(MC_DATUM);

      int y_sub = y_multi + 40; // vertical position for both buttons
      int padding_x = 10;       // horizontal padding around text
      int padding_y = 2;        // vertical padding around text

      int sub1Width = tft.textWidth(sub1);
      int sub2Width = tft.textWidth(sub2);

      int sub1BoxWidth = sub1Width + padding_x * 2;
      int sub2BoxWidth = sub2Width + padding_x * 2;
      int boxHeight = 16 * textSize + padding_y * 2;

      int x_sub1 = screen_width / 4;
      int x_sub2 = 3 * screen_width / 4;

      // Draw highlight rectangle if selected
      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw the sub-option text
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}

static void drawAllPlaying() {
  if(firstDrawingPlaying){
    // Clear the screen with orange background
    tft.fillScreen(orange_color);
    firstDrawingPlaying = false;
  }
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
      int x = col * cell_size + cell_size / 3 - 3;
      int y = row * cell_size + cell_size / 3 - 3;

      if (board[i] == "X") {
        if (moveCount >= 5 && moveQueue[0].index == i) {
          drawing.drawSdJpeg(DIS_X_PATH, x, y);
        } else {
          drawing.drawSdJpeg(X_PATH, x, y);
        }
      } else {
        if (moveCount >= 5 && moveQueue[0].index == i) {
          drawing.drawSdJpeg(DIS_O_PATH, x, y);
        } else {
          drawing.drawSdJpeg(O_PATH, x, y);
        }
      }
    }
  }
}

static void highlightCursor(int index) {
  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size + cell_size / 3 - 3;
  int y = y_start + row * cell_size + cell_size / 3 - 3;

  tft.drawRect(x, y, cell_size - 30, cell_size - 30, TFT_WHITE);
}

void clearCursor(int index) {
  if (index < 0 || index > 8)
    return;

  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size;
  int y = y_start + row * cell_size;

  // Fully clear and redraw the tile as white
  tft.fillRect(x, y, cell_size, cell_size, TFT_WHITE);
  tft.drawRect(x, y, cell_size, cell_size, TFT_BLACK);

  // Redraw the grid lines manually if needed (for crossovers)
  if (col > 0)
    tft.drawLine(x, y, x, y + cell_size, TFT_BLACK); // left
  if (col < 2)
    tft.drawLine(x + cell_size, y, x + cell_size, y + cell_size,
                 TFT_BLACK); // right
  if (row > 0)
    tft.drawLine(x, y, x + cell_size, y, TFT_BLACK); // top
  if (row < 2)
    tft.drawLine(x, y + cell_size, x + cell_size, y + cell_size,
                 TFT_BLACK); // bottom
}

static void drawWinLine() {
  if (winner != 'X' && winner != 'O')
    return;

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

static void drawWinnerMessage() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  String msg = "";
  if (winner == 'X')
    msg = "X Wins!";
  else if (winner == 'O')
    msg = "O Wins!";
  else if (winner == 'D')
    msg = "Draw!";

  tft.drawString(msg, tft.width() / 2, 30); // above grid
}

static void drawScoreboard() {
  int centerY = tft.height() / 2;
  int padding = 20;

  // Settings for big scoreboard
  int textSize = 4;
  tft.setTextSize(textSize);
  tft.setTextDatum(MC_DATUM);

  int underlineWidth = 40;
  int underlineThickness = 4; // <== THICKNESS OF THE LINE
  int underlineOffset = 24;   // Vertical distance from text to line
  int scoreOffset = 32;       // Distance from underline to score

  // === X Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xX = padding + underlineWidth;
  int yX = centerY - (underlineOffset + scoreOffset) / 2;
  tft.drawString("X", xX, yX);

  // Thick underline using fillRect
  int xLineY = yX + underlineOffset;
  tft.fillRect(xX - underlineWidth / 2, xLineY, underlineWidth,
               underlineThickness, TFT_WHITE);

  // Score for X
  tft.drawString(String(xWins), xX, xLineY + scoreOffset);

  // === O Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xO = tft.width() - padding - underlineWidth;
  int yO = yX;
  tft.drawString("O", xO, yO);

  // Thick underline using fillRect
  int oLineY = yO + underlineOffset;
  tft.fillRect(xO - underlineWidth / 2, oLineY, underlineWidth,
               underlineThickness, TFT_WHITE);

  // Score for O
  tft.drawString(String(oWins), xO, oLineY + scoreOffset);
}

static int findBestMove(char aiSymbol, char playerSymbol) {
  // Winning combinations
  const int wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                          {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

  // 1. Try to win
  for (const auto &combo : wins) {
    int countAI = 0, empty = -1;
    for (int idx : combo) {
      if (board[idx] == String(aiSymbol))
        countAI++;
      else if (board[idx] == "**")
        empty = idx;
    }
    if (countAI == 2 && empty != -1)
      return empty;
  }

  // 2. Block opponent
  for (const auto &combo : wins) {
    int countPlayer = 0, empty = -1;
    for (int idx : combo) {
      if (board[idx] == String(playerSymbol))
        countPlayer++;
      else if (board[idx] == "**")
        empty = idx;
    }
    if (countPlayer == 2 && empty != -1)
      return empty;
  }

  // 3. Take center
  if (board[4] == "**")
    return 4;

  // 4. Take a corner
  for (int i : {0, 2, 6, 8}) {
    if (board[i] == "**")
      return i;
  }

  // 5. Take any side
  for (int i : {1, 3, 5, 7}) {
    if (board[i] == "**")
      return i;
  }

  // If somehow none of these work
  return -1;
}
