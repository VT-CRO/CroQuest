#include "Memory.hpp"
#include "EndScreen/EndScreen.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"

// Timing variables
static unsigned long lastButtonPressTime = 0;
static unsigned long buttonDebounceDelay = 200;

// --- Card/Grid Settings ---
static const int CARD_SIZE = 50;
static const int CARD_PADDING = 6;
static const int GRID_SPACING = 6;

static const int NUM_LEVELS = 6;
static const int LEVELS[NUM_LEVELS][2] = {{2, 2}, {2, 3}, {2, 4},
                                          {3, 4}, {4, 4}, {4, 5}};

static const int NUM_CARD_IMAGES = 20;

static int currentLevel = 0;
static int cardRows = LEVELS[0][0];
static int cardCols = LEVELS[0][1];
static int CARD_X_OFFSET = 0;
static int CARD_Y_OFFSET = 0;

static bool flipped[4][5] = {false};
static int tileValues[4][5];
static int firstRow = -1, firstCol = -1;
static int secondRow = -1, secondCol = -1;
static bool waitingForSecond = false;
static bool lockInput = false;
static unsigned long flipTime = 0;
static const unsigned long MATCH_DELAY = 1000;
static bool waitingForWinChoice = false;
static int movesThisLevel = 0;
static int totalMoves = 0;
static int totalTime = 0;
static unsigned long levelStartTime = 0;
static int timeRemaining = 0;
static bool gameOver = false;

// Game State
enum State { HOMESCREEN, PLAYING, ENDSCREEN };
State currentState = HOMESCREEN;

static void drawBackground();
static void drawCard(int row, int col);
static void drawCardBacks();
static void drawCursor();
static void clearCursor();
static void handleInput();
static void loadLevel(int level);
static void handleInput();
static void flipCard(int row, int col);
static void flipCardBack(int row, int col);
static void checkWinCondition();
static void showLevelIntroScreen();
static void updateMoveCounter();
static void updateTimerDisplay();
static void triggerGameOver();
static void runMemoryFrame();
static void drawTiles();
static void showHomeScreen();
static void clearAllCursors();

// sounds
static void playGameOverSound();
static void playLevelCompleteSound();

// --- Cursor State ---
static int cursorRow = 0, cursorCol = 0;

void runMemory() {

  resetExitFlag(); // Restes flag for Main Menu
  currentState = HOMESCREEN;

  // reset level
  currentLevel = 0;

  // clear sprite and cache
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  showHomeScreen();
  for (;;) {
    runMemoryFrame();

    if (getExitFlag())
      return;

    if (currentState == HOMESCREEN && B.wasJustPressed()) {
      backAudio();
      Serial.println("Returning to menu from Tic Tac Toe");
      delay(500);
      return;
    }
  }
}

static void runMemoryFrame() {

  // Check if the Start Button was pressed and goes back to Main Menu
  if (checkStartButtonAndExit(tft))
    return;

  switch (currentState) {
  case HOMESCREEN:
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (A.wasJustPressed()) {
        playPressSound();
        showLevelIntroScreen();
        loadLevel(currentLevel);
        currentState = PLAYING;
        lastButtonPressTime = millis();
      }
    }
    break;
  case PLAYING: {
    if (waitingForWinChoice) {
      if (left.isPressed()) {
        backAudio();
        waitingForWinChoice = false;
        showLevelIntroScreen();
        loadLevel(currentLevel);
        delay(300);
        return;
      }

      if (right.isPressed()) {
        playPressSound();
        waitingForWinChoice = false;

        if (currentLevel == NUM_LEVELS - 1) {
          totalMoves = 0;
          currentLevel = 0;
          totalTime = 0;
        } else {
          currentLevel++;
        }

        showLevelIntroScreen();
        loadLevel(currentLevel);
        delay(300);
        return;
      }
      return;
    }

    // Game over logic
    if (gameOver) {
      return;
    }

    // Timer countdown
    int elapsed = (millis() - levelStartTime) / 1000;
    int remaining = (cardRows * cardCols * 5) - elapsed;

    if (remaining != timeRemaining) {
      timeRemaining = remaining;
      updateTimerDisplay();
    }

    if (timeRemaining <= 0) {
      triggerGameOver();
      return;
    }

    handleInput();

    if (lockInput && millis() - flipTime >= MATCH_DELAY) {
      int v1 = tileValues[firstRow][firstCol];
      int v2 = tileValues[secondRow][secondCol];

      if (v1 != v2) {
        flipCardBack(firstRow, firstCol);
        flipCardBack(secondRow, secondCol);
        flipped[firstRow][firstCol] = false;
        flipped[secondRow][secondCol] = false;
      }

      firstRow = firstCol = secondRow = secondCol = -1;
      waitingForSecond = false;
      lockInput = false;

      checkWinCondition();
    }
    break;
  }
  case ENDSCREEN:
    // ENDSCREEN HANDLING
    std::vector<String> playerNames = {settings.name}; // TEMP
    std::vector<int> playerScores = {totalMoves};      // TEMP

    EndScreen endScreen(playerNames, playerScores, false, settings.name,
                        totalMoves);
    if (endScreen.handleUserInput()) {
      // Clear Screen
      tft.fillScreen(TFT_BLACK);
      loadLevel(currentLevel);
      // showLevelIntroScreen();
      // loadLevel(currentLevel);
      currentState = PLAYING; // handleUserInput returns true : game restarts
    } else {
      if (endScreen.exit) { // exit to menu
        return;
      }
      currentState = HOMESCREEN;
      showHomeScreen(); // handleUserInput returns false : returns to game menu
      delay(300);
    }
    break;
  }
}

static void loadLevel(int level) {
  cardRows = LEVELS[level][0];
  cardCols = LEVELS[level][1];

  int gridWidth = cardCols * CARD_SIZE + (cardCols - 1) * GRID_SPACING;
  int gridHeight = cardRows * CARD_SIZE + (cardRows - 1) * GRID_SPACING;
  CARD_X_OFFSET = (tft.width() - gridWidth) / 2;
  CARD_Y_OFFSET = (tft.height() - gridHeight) / 2;

  cursorRow = cursorCol = 0;
  firstRow = firstCol = secondRow = secondCol = -1;
  waitingForSecond = false;
  lockInput = false;
  movesThisLevel = 0;
  gameOver = false;

  // Set countdown timer for the level
  timeRemaining = cardRows * cardCols * 5; // 5 sec per card
  levelStartTime = millis();

  // Generate card values
  int totalCards = cardRows * cardCols;
  int numPairs = totalCards / 2;

  int imagePool[NUM_CARD_IMAGES];
  for (int i = 0; i < NUM_CARD_IMAGES; i++) {
    imagePool[i] = i + 1;
  }

  // Shuffle image pool
  for (int i = NUM_CARD_IMAGES - 1; i > 0; i--) {
    int j = random(i + 1);
    int temp = imagePool[i];
    imagePool[i] = imagePool[j];
    imagePool[j] = temp;
  }

  // shuffled pair values
  int pairValues[totalCards];
  for (int i = 0; i < numPairs; i++) {
    pairValues[2 * i] = imagePool[i];
    pairValues[2 * i + 1] = imagePool[i];
  }

  for (int i = totalCards - 1; i > 0; i--) {
    int j = random(i + 1);
    int temp = pairValues[i];
    pairValues[i] = pairValues[j];
    pairValues[j] = temp;
  }

  int index = 0;
  for (int row = 0; row < cardRows; row++) {
    for (int col = 0; col < cardCols; col++) {
      tileValues[row][col] = pairValues[index++];
      flipped[row][col] = false;
    }
  }

  // Draw screen
  drawBackground();
  clearAllCursors();
  drawCardBacks();
  drawCursor();
  updateTimerDisplay();
  updateMoveCounter();
}

static void drawBackground() {
  drawTiles();
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(2);

  String moveLabel = "Moves: " + String(movesThisLevel);
  tft.drawString(moveLabel, tft.width() - 5, 5);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.drawString("Time: " + String(timeRemaining), 5, 5);
}

static void drawCard(int row, int col) {
  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

  if (flipped[row][col]) {
    String path = "/Memory/card" + String(tileValues[row][col]) + ".jpg";
    drawing.drawSdJpeg(path.c_str(), x, y);
    drawing.addToCache(path.c_str());
    drawing.pushSprite(false);
  }

  else {
    drawing.drawSdJpeg("/Memory/backs.jpg", x, y);
    drawing.addToCache("/Memory/backs.jpg");
    drawing.pushSprite(false);
  }
}

static void drawCardBacks() {
  for (int row = 0; row < cardRows; row++) {
    for (int col = 0; col < cardCols; col++) {
      drawCard(row, col);
    }
  }
}

static void drawCursor() {
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_RED);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_RED);
}

static void clearCursor() {
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_WHITE);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_WHITE);
}

static void handleInput() {
  int val35 = analogRead(35);
  int val34 = analogRead(34);

  if (right.isPressed() && cursorCol < cardCols - 1) {
    playSelectBeep();
    clearCursor();
    cursorCol++;
    drawCursor();
  }

  else if (up.isPressed() && cursorRow > 0) {
    playSelectBeep();
    clearCursor();
    cursorRow--;
    drawCursor();
  }

  if (down.isPressed() && cursorRow < cardRows - 1) {
    playSelectBeep();
    clearCursor();
    cursorRow++;
    drawCursor();
  }

  else if (left.isPressed() && cursorCol > 0) {
    playSelectBeep();
    clearCursor();
    cursorCol--;
    drawCursor();
  }

  delay(100);

  if (!lockInput && A.wasJustPressed()) {
    playPressSound();
    if (!flipped[cursorRow][cursorCol]) {
      flipCard(cursorRow, cursorCol);
      movesThisLevel++;
      updateMoveCounter();

      if (!waitingForSecond) {
        firstRow = cursorRow;
        firstCol = cursorCol;
        waitingForSecond = true;
      }

      else {
        secondRow = cursorRow;
        secondCol = cursorCol;
        waitingForSecond = false;
        lockInput = true;
        flipTime = millis();
      }
    }
  }
}

static void clearAllCursors() {
  int w = CARD_SIZE - 2 * CARD_PADDING;

  for (int row = 0; row < cardRows; row++) {
    for (int col = 0; col < cardCols; col++) {
      int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
      int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

      tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_WHITE);
      tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_WHITE);
    }
  }
}

static void flipCard(int row, int col) {
  flipped[row][col] = true;

  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

  String path = "/Memory/card" + String(tileValues[row][col]) + ".jpg";
  drawing.drawSdJpeg(path.c_str(), x, y);
  drawing.addToCache(path.c_str());
  drawing.pushSprite(false);
}

static void flipCardBack(int row, int col) {
  flipped[row][col] = false;
  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);
  drawing.drawSdJpeg("/Memory/backs.jpg", x, y);
  drawing.addToCache("/Memory/backs.jpg");
  drawing.pushSprite(false);
}

static void checkWinCondition() {
  for (int row = 0; row < cardRows; row++) {
    for (int col = 0; col < cardCols; col++) {
      if (!flipped[row][col])
        return;
    }
  }
  // Update total time taken for this level
  totalTime += (cardRows * cardCols * 5) - timeRemaining;
  waitingForWinChoice = true;
  totalMoves += movesThisLevel;
  drawTiles();

  // Draw centered UI
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // Title
  tft.setTextSize(3);
  tft.drawString("Level Complete!", tft.width() / 2, tft.height() / 2 - 70);

  // Game stats
  tft.setTextSize(2);
  tft.drawString("Total Time: " + String(totalTime) + "s", tft.width() / 2,
                 tft.height() / 2 - 30);
  tft.drawString("Total Moves: " + String(totalMoves), tft.width() / 2,
                 tft.height() / 2 - 5);

  // Options
  if (currentLevel == NUM_LEVELS - 1) {
    tft.drawString("LEFT: Replay", tft.width() / 2, tft.height() / 2 + 40);
    tft.drawString("RIGHT: Restart", tft.width() / 2, tft.height() / 2 + 65);
  } else {
    tft.drawString("LEFT: Replay", tft.width() / 2, tft.height() / 2 + 40);
    tft.drawString("RIGHT: Next", tft.width() / 2, tft.height() / 2 + 65);
  }

  playLevelCompleteSound();
  movesThisLevel = 0;

  // ================= Badge Unlock Logic =================
  if (currentLevel == NUM_LEVELS - 1) {
    // Unlock if totalTime is under 150 seconds
    if (totalTime < 150 && !badgeProgress[6] && !session.badgeUnlocked) {
      // if (currentLevel == 0 && !badgeProgress[3] && !session.badgeUnlocked) {
      badgeProgress[6] = true;
      isUnlocked[6] = true;
      saveBadgeProgress();
      checkFinalBadgeUnlock();
      session.badgeUnlocked = true;

      hasPendingNotification = true;
      pendingNotificationMessage = "Memory Badge Unlocked!";
      pendingNotificationDuration = 3000;
    }
  }
}

static void showLevelIntroScreen() {
  drawTiles();
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);

  String msg = "Level " + String(currentLevel + 1) + "/6";
  tft.drawString(msg, tft.width() / 2, tft.height() / 2 - 20);

  delay(1500);
}

static void updateMoveCounter() {
  tft.setTextDatum(MC_DATUM);
  int x = tft.width() - 5;
  int y = 5;

  int clearWidth = 120;
  int clearHeight = 20;
  tft.fillRect(x - clearWidth, y - 2, clearWidth, clearHeight,
               tft.color565(220, 220, 220));
  tft.drawRect(x - clearWidth - 1, y - 3, clearWidth + 2, clearHeight + 2,
               TFT_WHITE);
  tft.drawRect(x - clearWidth - 2, y - 4, clearWidth + 4, clearHeight + 4,
               TFT_WHITE);
  tft.drawRect(x - clearWidth - 3, y - 5, clearWidth + 6, clearHeight + 6,
               TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Moves: " + String(movesThisLevel), x - clearWidth / 2,
                 y + clearHeight / 2);
}

static void updateTimerDisplay() {
  tft.setTextDatum(MC_DATUM);
  int x = 4, y = 5;
  tft.fillRect(x - 2, y - 2, 120, 20, tft.color565(220, 220, 220));
  tft.drawRect(x - 2, y - 2, 120, 20, TFT_WHITE);
  tft.drawRect(x - 3, y - 3, 120 + 2, 20 + 2, TFT_WHITE);
  tft.drawRect(x - 4, y - 4, 120 + 4, 20 + 4, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Time: " + String(timeRemaining), x + 60, y + 10);
}

static void triggerGameOver() {
  playGameOverSound();
  currentState = ENDSCREEN;
  gameOver = true;
  currentLevel = 0;
  totalMoves = 0;
  movesThisLevel = 0;
}

void showHomeScreen() {
  drawTiles();
  tft.setTextDatum(MC_DATUM);

  // Draw the game title in large font
  tft.setTextColor(TFT_NAVY); // dark blue for contrast
  tft.setTextSize(4);
  tft.drawString("Memory", tft.width() / 2, tft.height() / 2 - 50);

  // Draw the author name
  tft.setTextColor(TFT_DARKGREY); // soft but readable
  tft.setTextSize(2);
  tft.drawString("Designed by CroQuest", tft.width() / 2,
                 tft.height() / 2 + 10);

  // Draw the "Press A to start" prompt
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Press A to start", tft.width() / 2, tft.height() - 50);
}

static void drawTiles() {
  const int BLOCKSIZE = 80;
  const int WIDTH = 6;
  const int HEIGHT = 4;

  uint16_t light_grey = tft.color565(205, 205, 205);
  uint16_t dark_grey = tft.color565(164, 164, 164);

  for (int row = 0; row < HEIGHT; row++) {
    for (int column = 0; column < WIDTH; column++) {
      tft.fillRect(column * BLOCKSIZE, row * BLOCKSIZE, BLOCKSIZE, BLOCKSIZE,
                   (column + row) % 2 == 0 ? light_grey : dark_grey);
    }
  }
}

static void playLevelCompleteSound() {
  const int noteDuration = 100; // milliseconds

  int melody[] = {523, 659, 784, 1046}; // C5, E5, G5, C6
  for (int i = 0; i < 4; i++) {
    playTone(melody[i], volume);
    delay(noteDuration);
  }
  playTone(0, 0); // Stop tone
}

static void playGameOverSound() {
  const int volume = 80;        // Percent
  const int noteDuration = 150; // milliseconds

  int melody[] = {659, 523, 392, 261}; // E5, C5, G4, C4
  for (int i = 0; i < 4; i++) {
    playTone(melody[i], volume);
    delay(noteDuration);
  }
  playTone(0, 0); // Stop tone
}
