#include "Memory.hpp"

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

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

// ========== Game States ========== //
enum State { HOMESCREEN, PLAYING, ENDSCREEN };
State currentState = HOMESCREEN;

// ####################################################################################################
//  Functions Declarations
// ####################################################################################################

// ========== Logic ========== //
static void handleInput();
static void loadLevel(int level);
static void flipCard(int row, int col);
static void flipCardBack(int row, int col);
static void checkWinCondition();
static void showLevelIntroScreen();
static void updateMoveCounter();
static void updateTimerDisplay();
static void triggerGameOver();
static void runMemoryFrame();

// ========== Drawing ========== //
static void drawBackground();
static void drawCard(int row, int col);
static void drawCardBacks();
static void drawCursor();
static void clearCursor();
static void drawTiles();
static void showHomeScreen();
static void clearAllCursors();

// ========== Sound ========== //
static void playGameOverSound();
static void playLevelCompleteSound();
static void playWinSound();

// --- Cursor State ---
static int cursorRow = 0, cursorCol = 0;

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runMemory() {

  resetExitFlag(); // Restes flag for Main Menu
  currentState = HOMESCREEN;

  // reset level
  currentLevel = 0;
  waitingForWinChoice = false;

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

// ========== Manual Loop ========== //
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
          playWinSound();
          currentState = ENDSCREEN;
          return;
        } else {
          currentLevel++;
          showLevelIntroScreen();
          loadLevel(currentLevel);
          delay(300);
          return;
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
      currentState = ENDSCREEN;
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
    Serial.println(timeRemaining);
    Serial.println(currentLevel);
    Serial.println(NUM_LEVELS);
    bool playerWon = (timeRemaining > 0 && currentLevel == NUM_LEVELS - 1);
    Serial.println(playerWon);

    // Prepare name label with total time
    String timeLabelStr = "Time: " + String(totalTime) + "s";
    char timeLabel[32]; // must be long enough
    timeLabelStr.toCharArray(timeLabel, sizeof(timeLabel));

    int finalScore = playerWon ? -1 : totalTime; // -1 means "don't show score"
    

    std::vector<String> playerNames = {settings.name};
    std::vector<int> playerScores = {finalScore};

    // Create a fake player in order to display win screen
    if(playerWon){
      playerNames.push_back("Game won!");
      playerScores.push_back(-2);
    }

    EndScreen endScreen(playerNames, playerScores, false, timeLabel,
                        finalScore);

    if (endScreen.handleUserInput()) {
      totalTime = 0;
      totalMoves = 0;
      currentLevel = 0;
      waitingForWinChoice = false;

      // Clear Screen
      tft.fillScreen(TFT_BLACK);
      showLevelIntroScreen();
      loadLevel(currentLevel);
      currentState = PLAYING; // handleUserInput returns true : game restarts
    } else {
      if (endScreen.exit) { // exit to menu
        totalTime = 0;
        totalMoves = 0;
        currentLevel = 0;
        waitingForWinChoice = false;
        return;
      }
      totalTime = 0;
      totalMoves = 0;
      currentLevel = 0;
      waitingForWinChoice = false;
      currentState = HOMESCREEN;
      showHomeScreen(); // handleUserInput returns false : returns to game menu
      delay(300);
    }
    break;
  }
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Handle User Input ========== //
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

// ========== Load Level ========== //
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

// ========== Flip Card ========== //
static void flipCard(int row, int col) {
  flipped[row][col] = true;

  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

  String path = "/Memory/card" + String(tileValues[row][col]) + ".jpg";
  drawing.drawSdJpeg(path.c_str(), x, y);
  drawing.addToCache(path.c_str());
  drawing.pushSprite(false);
}

// ========== Flip Card Back ========== //
static void flipCardBack(int row, int col) {
  flipped[row][col] = false;
  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);
  drawing.drawSdJpeg("/Memory/backs.jpg", x, y);
  drawing.addToCache("/Memory/backs.jpg");
  drawing.pushSprite(false);
}

// ========== Check Winner ========== //
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

    // ================= Badge Unlock Logic =================
  if (currentLevel == NUM_LEVELS - 1) {

    playWinSound();

    // Badge Logic | 150s
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

    // Jump to final ENDSCREEN — skip drawing “Level Complete” UI
    currentState = ENDSCREEN;
    return;
  }

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
}

// ========== Game Menu Screen ========== //
static void showLevelIntroScreen() {
  drawTiles();
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);

  String msg = "Level " + String(currentLevel + 1) + "/6";
  tft.drawString(msg, tft.width() / 2, tft.height() / 2 - 20);

  delay(1500);
}

// ========== Update Counter ========== //
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

// ========== Timer ========== //
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

// ========== Game Over ========== //
static void triggerGameOver() {
  playGameOverSound();
  currentState = ENDSCREEN;
  gameOver = true;
  currentLevel = 0;
  totalMoves = 0;
  movesThisLevel = 0;
}

// ========== Home Screen ========== //
void showHomeScreen() {
  tft.fillScreen(TFT_BLACK);  // Background

  // ---------- Layout Constants ----------
  int centerX = tft.width() / 2;
  int titleY = 80; 
  int subtitleY = titleY + 60; 
  int promptY = tft.height() - 80;
  int authorsY = tft.height() - 25; 

  // ---------- TITLE ----------
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(6);  // Bigger!
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString("MEMORY", centerX + 2, titleY + 2);  // shadow
  tft.setTextColor(TFT_WHITE);
  tft.drawString("MEMORY", centerX, titleY);

  // ---------- Subtitle ----------
  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("Focus. Match. Win.", centerX, subtitleY);


  tft.setTextSize(2); 
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("Press A to start", centerX + 1, promptY + 1);  // shadow
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("Press A to start", centerX, promptY);

  // ---------- Author Credits ----------
  tft.setTextSize(2);
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString("Designed by McCue & Shadoyan", centerX, authorsY);
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// ========== Draw Background ========== //
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

// ========== Draw Cards ========== //
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

// ========== Draw Card Back ========== //
static void drawCardBacks() {
  for (int row = 0; row < cardRows; row++) {
    for (int col = 0; col < cardCols; col++) {
      drawCard(row, col);
    }
  }
}

// ========== Draw Cursor ========== //
static void drawCursor() {
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_RED);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_RED);
}

// ========== Clear Cursor ========== //
static void clearCursor() {
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_WHITE);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_WHITE);
}

// ========== Clear All Cursors ========== //
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

// ========== Draw Background ========== //
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

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Level Completed Sound ========== //
static void playLevelCompleteSound() {
  const int noteDuration = 100; // milliseconds

  int melody[] = {523, 659, 784, 1046}; // C5, E5, G5, C6
  for (int i = 0; i < 4; i++) {
    playTone(melody[i], volume);
    delay(noteDuration);
  }
  playTone(0, 0); // Stop tone
}

// ========== Game Over Sound ========== //
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

// ========== WinnerSound ========== //
void playWinSound() {
  int melody[] = {880, 988, 1047, 1175}; // A5, B5, C6, D6
  int duration = 150;

  for (int i = 0; i < 4; i++) {
    ledcWriteTone(0, melody[i]);
    delay(duration);
    ledcWriteTone(0, 0); // Stop sound
    delay(50);
  }
}
