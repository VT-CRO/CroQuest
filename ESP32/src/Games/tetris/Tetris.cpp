#include "Tetris.hpp"
#include "EndScreen/EndScreen.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include <unordered_map>

// Speaker PIN
#define SPEAKER_PIN 21

// Tetris States
enum State {
  HOMESCREEN,
  PLAYING,
  ENDSCREEN,
  MULTIPLAYER_SELECTION,
  JOIN_SCREEN,
  BLUETOOTH_NUMPAD
};
static State currentState = HOMESCREEN;

// Selection
static int selection = 0;
static int subselection = 0;

// Timing variables
static unsigned long lastButtonPressTime = 0;
static unsigned long buttonDebounceDelay = 200;

static uint16_t bgColor = TFT_BLACK;

static const int GRID_HEIGHT = 20;
static const int GRID_WIDTH = 10;

enum Rotation { ROT_0 = 0, ROT_90 = 1, ROT_180 = 2, ROT_270 = 3 };

// Block Types
enum BlockType { I = 0, O = 1, T = 2, S = 3, Z = 4, J = 5, L = 6 };

// Block Colors
static const uint16_t COLORS[7] = {TFT_BLUE, TFT_GREEN,  TFT_RED, TFT_ORANGE,
                                   TFT_CYAN, TFT_YELLOW, TFT_PINK};

struct Piece {
  uint16_t color;
  uint8_t piece[4][4];
  int type;
  int x;
  int y;
  Rotation rotation;
};

// ================== GAME VARIABLES ============== //

static uint16_t grid[GRID_HEIGHT][GRID_WIDTH];
static int score = 0;
static int lines = 0;
static int highscore = 0;
static unsigned long fallInterval = 300;
static unsigned long spedupDropInterval = 50;
// current piece
Piece currentPiece;
// next piece
Piece nextPiece;
// Shadow piece
Piece shadowPiece;

std::unordered_map<uint16_t, String> assetMap = {
    {TFT_BLUE, "/tetris/assets/dark_blue"},
    {TFT_GREEN, "/tetris/assets/green"},
    {TFT_RED, "/tetris/assets/red"},
    {TFT_ORANGE, "/tetris/assets/orange"},
    {TFT_CYAN, "/tetris/assets/light_blue"},
    {TFT_YELLOW, "/tetris/assets/yellow"},
    {TFT_PINK, "/tetris/assets/pink"},
};

// DRAWING CONSTANTS

int const SCREEN_WIDTH = 480;
int const SCREEN_HEIGHT = 320;
int const BLOCK_SIZE = 15;
int const GRID_ORIGIN_X = SCREEN_WIDTH / 2 - (GRID_WIDTH * BLOCK_SIZE) / 2;
int const GRID_ORIGIN_Y = SCREEN_HEIGHT / 2 - (GRID_HEIGHT * BLOCK_SIZE) / 2;
const int PADDING = 20;

const int KICK_TABLE[4][5][2] = {
    // ROT_0 → ROT_90
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
    // ROT_90 → ROT_180
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
    // ROT_180 → ROT_270
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}},
    // ROT_270 → ROT_0
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
};

// I-piece kick table
const int I_KICK_TABLE[4][5][2] = {
    // 0 -> 1
    {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
    // 1 -> 2
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
    // 2 -> 3
    {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
    // 3 -> 0
    {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}};

const uint8_t SHAPES[7][4][4][4] = {
    // I-piece rotations
    {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}},

    // O-piece rotations (all same, no change)
    {{{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},

    // T-piece rotations
    {{{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},

    // S-piece rotations
    {{{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
     {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},

    // Z-piece rotations
    {{{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}}},

    // J-piece rotations
    {{{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}}},

    // L-piece rotations
    {{{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
     {{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}}};

// ============ Drawing ============= //
static void drawBlock(int x, int y, uint16_t color, bool shadowBlock = false);
static void drawPiece(uint16_t color, Piece piece, bool shadow = false);
static void drawHomeSelection();
static void drawGameInfoBoxes();
static void drawHomeScreen();
static void drawHighScore();
static void drawNextPiece();
static void drawScore();
static void drawLines();
static void drawGrid();

// ============== GAME Functions ============== //
static bool checkCollision(int testX, int testY, const uint8_t shape[4][4]);
static void spawnPiece(int type);
static void handleTetrisFrame();
static void singlePieceLogic();
bool tryRotatePieceClockwise();
static void lockPieceToGrid();
static bool setcurrentPiece();
static int findShadowPieceY();
static void startNewGame();
static void clearGrid();
static void clearRow();

static void playMoveSound();
static void playRotateSound();
static void playDropSound();
static void playClearSound();
static void playGameOverSound();

static unsigned long prevBlockFall;
static bool bottomCollision;

// Numpad
static NumPad<State> pad(drawHomeScreen, &currentState,
                         HOMESCREEN);

// Setup and entry point from menu
void runTetris() {

  resetExitFlag(); // Resets flag for Main Menu

  score = 0;
  currentState = HOMESCREEN;
  selection = 0;
  subselection = 0;

  // Initialize and reset prevBlockFall and bottomCollision
  prevBlockFall = 0;
  // Needs to be true to generate the first piece
  bottomCollision = true;

  // clear sprite and cache
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  // Clear Screen and drawing object
  tft.fillScreen(TFT_BLACK);
  drawHomeScreen();

  // Loop
  while (true) {

    handleTetrisFrame();

    if (getExitFlag())
      return;

    if (currentState == HOMESCREEN && B.wasJustPressed() ||
        currentState == HOMESCREEN && left.wasJustPressed()) {
      Serial.println("Returning to menu from Tetris");
      backAudio();
      delay(500);
      return;
    }
  }
}

// Handles States, input and generally controls gameplay
static void handleTetrisFrame() {

  if (checkStartButtonAndExit(tft))
    return;

  switch (currentState) {
  case HOMESCREEN:
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (A.wasJustPressed()) {
        playSelectConfirmSound();

        // Only single-player allowed
        tft.fillScreen(TFT_BLACK);
        startNewGame();
        currentState = PLAYING;

        lastButtonPressTime = millis();
      }
    }
    break;

    // Multiplayer is not finished yet
    // if (millis() - lastButtonPressTime > buttonDebounceDelay) {
    //   if (A.wasJustPressed()) {
    //     playSelectConfirmSound();
    //     if (selection == 1) {
    //       currentState = MULTIPLAYER_SELECTION;
    //       drawHomeSelection();
    //     } else {
    //       tft.fillScreen(TFT_BLACK);
    //       startNewGame();
    //       currentState = PLAYING;
    //     }
    //     lastButtonPressTime = millis();
    //   } else if (up.isPressed()) {
    //     if (selection == 1) {
    //       selection = 0;
    //       drawHomeSelection();
    //       playFocusMoveSound();
    //     }
    //     lastButtonPressTime = millis();
    //   } else if (down.isPressed()) {
    //     if (selection == 0) {
    //       selection = 1;
    //       drawHomeSelection();
    //       playFocusMoveSound();
    //     }
    //     lastButtonPressTime = millis();
    //   }
    // }
    // break;

  case MULTIPLAYER_SELECTION:
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (left.wasJustPressed()) {
        if (subselection == 1) {
          subselection = 0;
          drawHomeSelection();
          playFocusMoveSound();
        }
      } else if (right.wasJustPressed()) {
        if (subselection == 0) {
          subselection = 1;
          drawHomeSelection();
          playFocusMoveSound();
        }
      } else if (A.wasJustPressed()) {
        playSelectConfirmSound();
        if (subselection == 0) {
          tft.fillScreen(TFT_BLUE);
          currentState = JOIN_SCREEN;
        } else {
          pad.numPadSetup();
          currentState = BLUETOOTH_NUMPAD;
          break;
        }
      } else if (up.wasJustPressed()) {
        currentState = HOMESCREEN;
        subselection = 0;
        selection = 1;
        drawHomeSelection();
        playFocusMoveSound();
      }
      lastButtonPressTime = millis();
    }
    break;
  case PLAYING:
    singlePieceLogic();

    // ========= Badge Unlock Condition ========= //
    if (score >= 5000 && !badgeProgress[7] && !session.badgeUnlocked) {
      badgeProgress[7] = true;
      isUnlocked[7] = true;
      saveBadgeProgress();
      checkFinalBadgeUnlock();
      session.badgeUnlocked = true;

      hasPendingNotification = true;
      pendingNotificationMessage = "Tetris Badge Unlocked!";
      pendingNotificationDuration = 3000;
    }

    break;
  case ENDSCREEN:
    // ENDSCREEN HANDLING
    std::vector<String> playerNames = {settings.name}; // TEMP
    std::vector<int> playerScores = {score};           // TEMP

    EndScreen endScreen(playerNames, playerScores, false, settings.name, score);
    if (endScreen.handleUserInput()) {
      // Clear Screen
      tft.fillScreen(TFT_BLACK);
      startNewGame();
      currentState = PLAYING; // handleUserInput returns true : game restarts
    } else {
      if (endScreen.exit) { // exit to menu
        return;
      }
      currentState = HOMESCREEN;
      // Clear Screen
      tft.fillScreen(TFT_BLACK);
      drawHomeScreen(); // handleUserInput returns false : returns to game menu
    }
    break;
  }
}

// ============ GAME LOGIC ============= //

// Starts a new game
static void startNewGame() {
  score = 0;
  lines = 0;

  clearGrid();
  drawGrid();

  drawGameInfoBoxes();
  drawHighScore();
  drawScore();
  drawLines();

  spawnPiece(random(0, 7));
}

static void lockPieceToGrid() {
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      // Only apply non-empty blocks of the piece
      if (currentPiece.piece[y][x]) {
        int gx = currentPiece.x + x;
        int gy = currentPiece.y + y;

        // Safety check: don't write out-of-bounds
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
          grid[gy][gx] = currentPiece.color; // Lock piece into grid
        }
      }
    }
  }
}

// MAIN LOGIC FOR FALLING PIECES + COLLISIONS
static void singlePieceLogic() {
  if (bottomCollision) {
    if (setcurrentPiece()) {
      drawPiece(shadowPiece.color, shadowPiece);
      playGameOverSound();
      // Delay added to make sure the player knows it's gameover
      delay(500);
      currentState = ENDSCREEN;

      if (score > highscore) {
        // New Highscore
        highscore = score;
        File file = SD.open("/tetris/highscore.txt", FILE_WRITE);
        if (file) {
          file.println(highscore);
          file.close();
        }
      }
      return;
    }
    spawnPiece(random(0, 7));
    drawNextPiece();

    drawPiece(currentPiece.color, currentPiece);
    playMoveSound();
    drawPiece(shadowPiece.color, shadowPiece, true);
    bottomCollision = false;
  } else {
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      // Movies pieces to the right
      if (right.isPressed()) {
        if (!checkCollision(currentPiece.x + 1, currentPiece.y,
                            currentPiece.piece)) {
          drawPiece(TFT_BLACK, currentPiece); // Erase current position
          currentPiece.x++;
          drawPiece(currentPiece.color, currentPiece);

          drawPiece(TFT_BLACK, shadowPiece, true);
          shadowPiece.x = currentPiece.x;
          shadowPiece.y = findShadowPieceY();
          drawPiece(shadowPiece.color, shadowPiece, true);

          // Drawing the current piece again
          drawPiece(currentPiece.color, currentPiece);
        }
        lastButtonPressTime = millis();
      }
      // Moves pieces to the left
      else if (left.isPressed()) {
        if (!checkCollision(currentPiece.x - 1, currentPiece.y,
                            currentPiece.piece)) {
          drawPiece(TFT_BLACK, currentPiece); // Erase current position
          currentPiece.x--;

          drawPiece(currentPiece.color, currentPiece);

          drawPiece(TFT_BLACK, shadowPiece, true);
          shadowPiece.x = currentPiece.x;
          shadowPiece.y = findShadowPieceY();
          drawPiece(shadowPiece.color, shadowPiece, true);

          // Drawing the current piece again
          drawPiece(currentPiece.color, currentPiece);
        }
        lastButtonPressTime = millis();
      }
      // Rotates pieces
      else if (up.isPressed()) {
        int temp[4][4];
        drawPiece(TFT_BLACK, currentPiece);
        tryRotatePieceClockwise();
        drawPiece(currentPiece.color, currentPiece);
        playRotateSound();

        drawPiece(TFT_BLACK, shadowPiece, true);
        shadowPiece.rotation = currentPiece.rotation;
        shadowPiece.x = currentPiece.x;
        memcpy(shadowPiece.piece, currentPiece.piece,
               sizeof(shadowPiece.piece));
        shadowPiece.y = findShadowPieceY();
        drawPiece(shadowPiece.color, shadowPiece, true);

        // Drawing the current piece again
        drawPiece(currentPiece.color, currentPiece);

        lastButtonPressTime = millis();
      } else if (A.wasJustPressed()) {
        drawPiece(TFT_BLACK, currentPiece); // Clear current Piece
        score += (shadowPiece.y - currentPiece.y) * 2;
        drawScore();
        currentPiece.y = shadowPiece.y;
        currentPiece.x = shadowPiece.x;
        lockPieceToGrid();
        playDropSound();
        drawPiece(currentPiece.color,
                  currentPiece); // draw new location of the piece
        clearRow();
        lastButtonPressTime = millis();

        bottomCollision = true;
        return;
      }
    }
    // Speeds up falling pieces
    unsigned long currentFallInterval =
        down.isPressed() ? spedupDropInterval : fallInterval;

    if (millis() - prevBlockFall > currentFallInterval) {
      // Collision w/ block or bottom of the board
      if (checkCollision(currentPiece.x, currentPiece.y + 1,
                         currentPiece.piece)) {
        bottomCollision = true;
        // Lock piece into grid
        lockPieceToGrid();
        drawPiece(TFT_BLACK, shadowPiece, true);
        drawPiece(currentPiece.color, currentPiece);
        clearRow();
        bottomCollision = true;
      } else {
        drawPiece(TFT_BLACK, currentPiece); // Erase current position
        currentPiece.y++;
        drawPiece(currentPiece.color, currentPiece); // Draw new position
        if (currentFallInterval == spedupDropInterval) {
          score++;
          drawScore();
        }
      }
      prevBlockFall = millis();
    }
  }
}

// Checks if there is a row to clear and clears it if that's the case
// Also moves all rows down if a row is cleared
static void clearRow() {
  int cleared[4] = {0, 0, 0, 0}; // 0 == false (no rows cleared)

  for (int y = 0; y < 4; y++) {
    if (currentPiece.y + y < 0 || currentPiece.y + y >= GRID_HEIGHT)
      continue;
    bool clear = true;
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[currentPiece.y + y][x] == 0) {
        clear = false;
        break;
      }
    }
    // Clears Rows
    if (clear) {
      cleared[y] = 1; // 1 == true (row cleared)
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[currentPiece.y + y][x] = 0;
        drawBlock(x, currentPiece.y + y, TFT_BLACK);
      }
      playClearSound();
    }
  }

  // Moves blocks down
  for (int y = 0; y < 4; y++) {
    if (cleared[y]) {
      for (int row = currentPiece.y + y; row > 1; row--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[row][x] = grid[row - 1][x];
          grid[row - 1][x] = 0;
          drawBlock(x, row - 1, TFT_BLACK);
          drawBlock(x, row, grid[row][x]);
        }
      }
      lines++;
      score += 100;
    }
  }
  drawLines();
  drawScore();
}

// Spawns a next piece
static void spawnPiece(int type) {
  for (int y = 0; y < 4; ++y)
    for (int x = 0; x < 4; ++x)
      nextPiece.piece[y][x] = SHAPES[type][0][y][x];

  nextPiece.type = type;
  nextPiece.color = COLORS[type];
}

// Sets the next piece to be the current piece
// Returns true if the game is over
// Returns false if the game can continue
static bool setcurrentPiece() {
  currentPiece.type = nextPiece.type;
  shadowPiece.type = nextPiece.type;

  currentPiece.rotation = ROT_0;
  shadowPiece.rotation = ROT_0;

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      currentPiece.piece[y][x] = nextPiece.piece[y][x];
      shadowPiece.piece[y][x] = nextPiece.piece[y][x];
    }
  }
  currentPiece.x = 3;
  shadowPiece.x = 3;

  // "I" piece starts lower than the rest of the blocks
  currentPiece.y = currentPiece.type == I ? -2 : -1;
  currentPiece.color = nextPiece.color;
  shadowPiece.color = nextPiece.color;

  // Need to set the shadow piece y-location
  shadowPiece.y = findShadowPieceY();
  if (shadowPiece.y <= -1) {
    return true;
  }
  return false;
}

static int findShadowPieceY() {
  // Need to set the shadow piece y-location
  for (int y = currentPiece.y; y < GRID_HEIGHT; y++) {
    if (checkCollision(shadowPiece.x, y, shadowPiece.piece)) {
      return y - 1;
    }
  }
  return currentPiece.y;
}

// Initializes/resets the grid
static void clearGrid() {
  for (int y = 0; y < GRID_HEIGHT; ++y)
    for (int x = 0; x < GRID_WIDTH; ++x)
      grid[y][x] = 0; // EMPTY
}

static bool checkCollision(int testX, int testY, const uint8_t shape[4][4]) {
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      if (shape[y][x] == 0)
        continue;

      int gridX = testX + x;
      int gridY = testY + y;

      // Off the left/right edges
      if (gridX < 0 || gridX >= GRID_WIDTH)
        return true;

      // Off the bottom
      if (gridY >= GRID_HEIGHT)
        return true;

      // Above the grid (allowed on spawn)
      if (gridY < 0)
        continue;

      // Collides with existing grid block
      if (grid[gridY][gridX] != 0)
        return true;
    }
  }
  return false;
}

bool tryRotatePieceClockwise() {
  // if(currentPiece.type == O) return true;

  Rotation oldRotation = currentPiece.rotation;
  Rotation newRotation = static_cast<Rotation>((currentPiece.rotation + 1) % 4);
  const uint8_t(*rotated)[4] = SHAPES[currentPiece.type][newRotation];

  // Select appropriate kick table
  const int(*kickTable)[4][5][2] =
      (currentPiece.type == I) ? &I_KICK_TABLE : &KICK_TABLE;

  for (int i = 0; i < 5; ++i) {
    int dx = (*kickTable)[oldRotation][i][0];
    int dy = (*kickTable)[oldRotation][i][1];

    int testX = currentPiece.x + dx;
    int testY = currentPiece.y + dy;

    if (!checkCollision(testX, testY, rotated)) {
      // Success: update piece
      currentPiece.x = testX;
      currentPiece.y = testY;
      currentPiece.rotation = newRotation;

      // Copy rotated shape into current piece shape
      for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
          currentPiece.piece[y][x] = rotated[y][x];

      return true; // rotation succeeded
    }
  }

  return false; // rotation failed, no valid kick found
}

// ============== DRAWING =============== //

static void drawHomeScreen() {

  // Set title properties
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(4);
  tft.drawString("TETRIS", tft.width() / 2, 40);

  // Tagline
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Falling Block Puzzle", tft.width() / 2, 90);

  // Options
  tft.setTextSize(2);
  tft.drawString("Start Single Player", tft.width() / 2, 180);
  // tft.drawString("Start Multiplayer", tft.width() / 2, 230);

  drawHomeSelection();
}

static void drawHomeSelection() {
  int y_single = 180;
  int y_multi = 230;
  int y_sub = y_multi + 40;

  // Clear option areas
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, tft.width(), 35, TFT_BLACK);
  tft.fillRect(0, y_multi - 15, tft.width(), 80, TFT_BLACK);

  if (selection == 0) {
    // Single-player selected
    tft.setTextSize(3);
    tft.drawString("Press to Start", tft.width() / 2, y_single);

    // tft.setTextSize(2);
    // tft.drawString("Start Multiplayer", tft.width() / 2, y_multi);
  } else {
    // Multiplayer selected
    tft.setTextSize(2);
    tft.drawString("Start Single Player", tft.width() / 2, y_single);

    // tft.setTextSize(3);
    // tft.drawString("Start Multiplayer", tft.width() / 2, y_multi);

    if (currentState == MULTIPLAYER_SELECTION) {
      const char *sub1 = "Host Game";
      const char *sub2 = "Join Game";

      tft.setTextSize(2);
      int padding_x = 10;
      int padding_y = 4;
      int boxHeight = 20 + padding_y * 2;

      int sub1Width = tft.textWidth(sub1);
      int sub2Width = tft.textWidth(sub2);
      int sub1BoxWidth = sub1Width + padding_x * 2;
      int sub2BoxWidth = sub2Width + padding_x * 2;

      int x_sub1 = tft.width() / 4;
      int x_sub2 = 3 * tft.width() / 4;

      // Highlight rectangle
      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw sub-option labels
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }

  // ========== Author Credits ========== //
  tft.setTextColor(tft.color565(150, 150, 150)); // light grey
  tft.setTextSize(2);
  tft.drawString("Designed by: Lucas Shadoyan", tft.width() / 2,
                 tft.height() - 10);
}

// Draws the piece onto the board
static void drawPiece(uint16_t color, Piece piece, bool shadow) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      // Makes sure the block is within the board
      if (piece.y + y >= 0) {
        if (piece.piece[y][x]) {
          drawBlock(piece.x + x, piece.y + y, color, shadow);
        }
      }
    }
  }
}

void drawBox(int x, int y, int w, int h, const char *label) {
  // Draw border
  tft.fillRect(x, y, w, h, tft.color565(20, 20, 20));
  tft.fillRect(x + 4, y + 4, w - 8, h - 8, TFT_BLACK);

  // Draw label text (centered at the top of the box)
  int16_t labelWidth = tft.textWidth(label);
  tft.setCursor(x + (w - labelWidth) / 2, y - 4);
  tft.print(label);
}
static void drawGameInfoBoxes() {

  drawBox(SCREEN_WIDTH - 120 - PADDING, PADDING, 120, 80, "NEXT");
  drawBox(SCREEN_WIDTH - 120 - PADDING + 10, PADDING + 120 + PADDING * 2, 100,
          60, "HIGHSCORE");
  drawBox(PADDING, SCREEN_HEIGHT / 2 - 100 - PADDING, 100, 60, "LINES");
  drawBox(PADDING, SCREEN_HEIGHT / 2 + PADDING, 100, 60, "SCORE");
}

static void drawScore() {
  tft.setTextSize(2);
  tft.fillRect(PADDING + 4, SCREEN_HEIGHT / 2 + PADDING + 4, 100 - 8, 60 - 8,
               TFT_BLACK); // clear score box

  // Redraw "SCORE"
  int16_t labelWidth = tft.textWidth("SCORE");
  tft.setCursor(PADDING + (100 - labelWidth) / 2,
                SCREEN_HEIGHT / 2 + PADDING - 4);
  tft.print("SCORE");

  int boxX = PADDING;
  int boxY = SCREEN_HEIGHT / 2 + PADDING;
  int boxW = 100;
  int boxH = 60;

  String scoreStr = String(score);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int textSize = 3;
  while (textSize > 0) {
    tft.setTextSize(textSize);
    if (tft.textWidth(scoreStr) <= boxW - 10)
      break;
    textSize--;
  }

  tft.drawString(scoreStr, boxX + boxW / 2, boxY + boxH / 2);
}

static void drawLines() {
  tft.setTextSize(2);
  tft.fillRect(PADDING + 4, SCREEN_HEIGHT / 2 - 100 - PADDING + 4, 100 - 8,
               60 - 8, TFT_BLACK); // clear lines box

  // Redraw "LINES"
  int16_t labelWidth = tft.textWidth("LINES");
  tft.setCursor(PADDING + (100 - labelWidth) / 2,
                SCREEN_HEIGHT / 2 - 100 - PADDING - 4);
  tft.print("LINES");

  int boxX = PADDING;
  int boxY = SCREEN_HEIGHT / 2 - 100 - PADDING;
  int boxW = 100;
  int boxH = 60;

  String linesStr = String(lines);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int textSize = 3;
  while (textSize > 0) {
    tft.setTextSize(textSize);
    if (tft.textWidth(linesStr) <= boxW - 10)
      break;
    textSize--;
  }

  tft.drawString(linesStr, boxX + boxW / 2, boxY + boxH / 2);
}

static void drawHighScore() {
  File file = SD.open("/tetris/highscore.txt", "r");

  if (file) {
    highscore = file.parseInt();
    file.close();
  }

  int boxX = SCREEN_WIDTH - 120 - PADDING + 10;
  int boxY = PADDING + 120 + PADDING * 2;
  int boxW = 100;
  int boxH = 60;

  String scoreStr = String(highscore);

  // Set default text settings
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Try decreasing text size until it fits within the box width
  int textSize = 3;
  while (textSize > 0) {
    tft.setTextSize(textSize);
    if (tft.textWidth(scoreStr) <= boxW - 10)
      break;
    textSize--;
  }

  tft.drawString(scoreStr, boxX + boxW / 2, boxY + boxH / 2);
}

static void drawGrid() {
  // Draw grid border
  tft.fillRect(GRID_ORIGIN_X - 5, GRID_ORIGIN_Y - 5,
               GRID_WIDTH * BLOCK_SIZE + 10, GRID_HEIGHT * BLOCK_SIZE + 10,
               tft.color565(20, 20, 20));
  for (int y = 0; y < GRID_HEIGHT; ++y) {
    for (int x = 0; x < GRID_WIDTH; ++x) {
      uint16_t color = grid[y][x];
      drawBlock(x, y, TFT_BLACK);
    }
  }
}

static void drawBlock(int x, int y, uint16_t color, bool shadowBlock) {
  int screenX = GRID_ORIGIN_X + x * BLOCK_SIZE;
  int screenY = GRID_ORIGIN_Y + y * BLOCK_SIZE;
  // Removes block
  if (color == TFT_BLACK) {
    // Draw filled block
    tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);
  } else {
    String path = assetMap[color];

    // Shadow block / block at bottom of screen corresponding
    // to the location the block will fall in
    if (shadowBlock) {
      path += "_selection";
    }

    path += ".jpg";

    // Push blocks
    drawing.drawSdJpeg(path.c_str(), screenX, screenY);
    drawing.addToCache(path.c_str());
    drawing.pushSprite();
  }
  tft.drawRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE,
               tft.color565(30, 30, 30));
}

static void drawNextPiece() {
  // reset text size
  tft.setTextSize(2);

  int boxX = SCREEN_WIDTH - 120 - PADDING;
  int boxY = PADDING;
  int boxW = 120;
  int boxH = 80;

  // Clear inner box area before drawing
  tft.fillRect(boxX + 4, boxY + 4, boxW - 8, boxH - 8, TFT_BLACK);

  // Redraw the "NEXT" label (centered at the top of the box)
  const char *label = "NEXT";
  int16_t labelWidth = tft.textWidth(label);
  tft.setCursor(boxX + (boxW - labelWidth) / 2, boxY - 4);
  tft.print(label);

  // Determine dimensions of the piece (4x4 max)
  int minX = 4, maxX = -1, minY = 4, maxY = -1;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (nextPiece.piece[y][x]) {
        if (x < minX)
          minX = x;
        if (x > maxX)
          maxX = x;
        if (y < minY)
          minY = y;
        if (y > maxY)
          maxY = y;
      }
    }
  }

  int pieceW = (maxX - minX + 1) * BLOCK_SIZE;
  int pieceH = (maxY - minY + 1) * BLOCK_SIZE;

  // Top-left corner to start drawing to center it in the box
  int offsetX = boxX + (boxW - pieceW) / 2;
  int offsetY = boxY + (boxH - pieceH) / 2;

  // Draw each block of the piece
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (nextPiece.piece[y][x]) {
        int px = offsetX + (x - minX) * BLOCK_SIZE;
        int py = offsetY + (y - minY) * BLOCK_SIZE;

        // Draw using the same logic as drawBlock but override grid-based offset
        if (nextPiece.color == TFT_BLACK) {
          tft.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, TFT_BLACK);
        } else {
          String path = assetMap[nextPiece.color] + ".jpg";
          drawing.drawSdJpeg(path.c_str(), px, py);
          drawing.addToCache(path.c_str());
          drawing.pushSprite();
        }
        tft.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, tft.color565(30, 30, 30));
      }
    }
  }
}

static void playMoveSound() {
  playTone(700, volume); // gentle movement blip
  delay(25);
  playTone(0, 0);
}

static void playRotateSound() {
  playTone(850, volume); // slightly higher for rotate
  delay(25);
  playTone(0, 0);
}

static void playDropSound() {
  playTone(600, volume); // deeper drop tone
  delay(40);
  playTone(0, 0);
}

static void playClearSound() {
  playTone(1200, volume); // clear row "ping"
  delay(80);
  playTone(0, 0);
}

static void playGameOverSound() {
  playTone(400, volume);
  delay(200);
  playTone(300, volume);
  delay(200);
  playTone(0, 0);
}
