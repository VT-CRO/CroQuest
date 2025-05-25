#include <TFT_eSPI.h>
#include "../src2/Core/JpegDrawing.hpp"
#include "../src2/Core/JpegDrawing.cpp"

TFT_eSPI tft = TFT_eSPI();
JpegDrawing jpegDrawer(tft);

// --- Pin Definitions ---
#define PIN_UP     35
#define PIN_DOWN   34
#define PIN_LEFT   34
#define PIN_RIGHT  35
#define PIN_A      22
#define PIN_B      1
#define PIN_SPEAKER 21

#define BTN_SELECT PIN_A
#define BTN_LEFT PIN_LEFT

// --- Card/Grid Settings ---
const int CARD_SIZE = 50;
const int CARD_PADDING = 6;
const int GRID_SPACING = 6;

const int NUM_LEVELS = 6;
const int LEVELS[NUM_LEVELS][2] = 
{
  {2, 2}, {2, 3}, {2, 4}, {3, 4}, {4, 4}, {4, 5}
};

const int NUM_CARD_IMAGES = 20;

int currentLevel = 0;
int cardRows = LEVELS[0][0];
int cardCols = LEVELS[0][1];
int CARD_X_OFFSET = 0;
int CARD_Y_OFFSET = 0;

bool flipped[4][5] = {false};
int tileValues[4][5];
int firstRow = -1, firstCol = -1;
int secondRow = -1, secondCol = -1;
bool waitingForSecond = false;
bool lockInput = false;
unsigned long flipTime = 0;
const unsigned long MATCH_DELAY = 1000;
bool waitingForWinChoice = false;
int movesThisLevel = 0;
int totalMoves = 0;
unsigned long levelStartTime = 0;
int timeRemaining = 0;
bool gameOver = false;


void drawBackground();
void drawCard(int row, int col);
void drawCardBacks();
void drawCursor();
void clearCursor();
void handleInput();
void loadLevel(int level);
void handleInput();
void flipCard(int row, int col);
void flipCardBack(int row, int col);
void checkWinCondition();
void showLevelIntroScreen();
void updateMoveCounter();
void updateTimerDisplay(); 
void triggerGameOver();

// --- Cursor State ---
int cursorRow = 0, cursorCol = 0;

void setup() 
{
  delay(1000);
  tft.init();
  tft.setRotation(3);

  if (!SD.begin()) 
  {
    tft.setTextColor(TFT_RED);
    tft.drawString("SD init failed", 10, 10, 2);
    while (true);
  }

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SPEAKER, OUTPUT);

  showLevelIntroScreen();
  loadLevel(currentLevel);
}

void loop() 
{
  if (waitingForWinChoice) 
  {
    int val35 = analogRead(35);
    int val34 = analogRead(34);

    if (val34 > 3000 && val34 < 3400) 
    {
      waitingForWinChoice = false;
      showLevelIntroScreen();
      loadLevel(currentLevel);
      delay(300);
      return;
    }

    if (val35 > 3900 && val35 < 4200) 
    {
      waitingForWinChoice = false;

      if (currentLevel == NUM_LEVELS - 1) 
      {
        totalMoves = 0;
        currentLevel = 0;
      } 
      
      else 
      {
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
  if (gameOver) 
  {
    return;
  }

  // Timer countdown
  int elapsed = (millis() - levelStartTime) / 1000;
  int remaining = (cardRows * cardCols * 5) - elapsed;

  if (remaining != timeRemaining) 
  {
    timeRemaining = remaining;
    updateTimerDisplay();
  }

  if (timeRemaining <= 0) 
  {
    triggerGameOver();
    return;
  }

  handleInput();

  if (lockInput && millis() - flipTime >= MATCH_DELAY) 
  {
    int v1 = tileValues[firstRow][firstCol];
    int v2 = tileValues[secondRow][secondCol];

    if (v1 != v2) 
    {
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
}


void loadLevel(int level) 
{
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
  timeRemaining = cardRows * cardCols * 5;  // 5 sec per card
  levelStartTime = millis();

  // Generate card values
  int totalCards = cardRows * cardCols;
  int numPairs = totalCards / 2;

  int imagePool[NUM_CARD_IMAGES];
  for (int i = 0; i < NUM_CARD_IMAGES; i++) 
  {
    imagePool[i] = i + 1;
  }

  // Shuffle image pool
  for (int i = NUM_CARD_IMAGES - 1; i > 0; i--) 
  {
    int j = random(i + 1);
    int temp = imagePool[i];
    imagePool[i] = imagePool[j];
    imagePool[j] = temp;
  }

  // shuffled pair values
  int pairValues[totalCards];
  for (int i = 0; i < numPairs; i++) 
  {
    pairValues[2 * i] = imagePool[i];
    pairValues[2 * i + 1] = imagePool[i];
  }

  for (int i = totalCards - 1; i > 0; i--) 
  {
    int j = random(i + 1);
    int temp = pairValues[i];
    pairValues[i] = pairValues[j];
    pairValues[j] = temp;
  }

  int index = 0;
  for (int row = 0; row < cardRows; row++)
  {
    for (int col = 0; col < cardCols; col++)
    {
      tileValues[row][col] = pairValues[index++];
      flipped[row][col] = false;
    }
  }

  // Draw screen
  drawBackground();
  drawCardBacks();
  drawCursor();
  updateTimerDisplay();
  updateMoveCounter();
}

void drawBackground() 
{
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(2);

  String moveLabel = "Moves: " + String(movesThisLevel);
  tft.drawString(moveLabel, tft.width() - 5, 5);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.drawString("Time: " + String(timeRemaining), 5, 5);

}

void drawCard(int row, int col) 
{
  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

  if (flipped[row][col]) 
  {
    String path = "/Memory/card" + String(tileValues[row][col]) + ".jpg";
    jpegDrawer.drawSdJpeg(path.c_str(), x, y);
    jpegDrawer.pushSprite(false);
  }

  else 
  {
    jpegDrawer.drawSdJpeg("/Memory/backs.jpg", x, y);
    jpegDrawer.pushSprite(false);
  }
}


void drawCardBacks() 
{
  for (int row = 0; row < cardRows; row++) 
  {
    for (int col = 0; col < cardCols; col++) 
    {
      drawCard(row, col);
    }
  }
}

void drawCursor() 
{
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_RED);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_RED);
}

void clearCursor()
{
  int w = CARD_SIZE - 2 * CARD_PADDING;
  int x = CARD_X_OFFSET + cursorCol * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + cursorRow * (CARD_SIZE + GRID_SPACING);

  tft.drawRect(x - 3, y - 3, w + 18, w + 18, TFT_WHITE);
  tft.drawRect(x - 4, y - 4, w + 20, w + 20, TFT_WHITE);
}


void handleInput() 
{
  int val35 = analogRead(35);
  int val34 = analogRead(34);

  if (val35 > 3900 && val35 < 4200 && cursorCol < cardCols - 1)
  {
    clearCursor(); 
    cursorCol++; 
    drawCursor();
  } 

  else if (val35 > 3000 && val35 < 3400 && cursorRow > 0)
  {
    clearCursor();
    cursorRow--;
    drawCursor();
  }

  if (val34 > 3900 && val34 < 4200 && cursorRow < cardRows - 1)
  {
    clearCursor();
    cursorRow++;
    drawCursor();
  }

  else if (val34 > 3000 && val34 < 3400 && cursorCol > 0)
  {
    clearCursor();
    cursorCol--;
    drawCursor();
  }

  delay(100);

  if (!lockInput && digitalRead(BTN_SELECT) == HIGH)
  {
    if (!flipped[cursorRow][cursorCol])
    {
      flipCard(cursorRow, cursorCol);
      movesThisLevel++;
      updateMoveCounter();

      if (!waitingForSecond)
      {
        firstRow = cursorRow;
        firstCol = cursorCol;
        waitingForSecond = true;
      }

      else
      {
        secondRow = cursorRow;
        secondCol = cursorCol;
        waitingForSecond = false;
        lockInput = true;
        flipTime = millis();
      }
    }
  }
}


void flipCard(int row, int col)
{
  flipped[row][col] = true;

  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);

  String path = "/Memory/card" + String(tileValues[row][col]) + ".jpg";
  jpegDrawer.drawSdJpeg(path.c_str(), x, y);
  jpegDrawer.pushSprite(false);
}

void flipCardBack(int row, int col)
{
  flipped[row][col] = false;
  int x = CARD_X_OFFSET + col * (CARD_SIZE + GRID_SPACING);
  int y = CARD_Y_OFFSET + row * (CARD_SIZE + GRID_SPACING);
  jpegDrawer.drawSdJpeg("/Memory/backs.jpg", x, y);
  jpegDrawer.pushSprite(false);
}

void checkWinCondition() 
{
  for (int row = 0; row < cardRows; row++)
  {
    for (int col = 0; col < cardCols; col++)
    {
      if (!flipped[row][col]) return;
    }
  }
  waitingForWinChoice = true;
  totalMoves += movesThisLevel;

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString("Level Complete!", tft.width() / 2, tft.height() / 2 - 40);

  tft.setTextSize(2);
  tft.drawString("Total Moves: " + String(totalMoves), tft.width() / 2, tft.height() / 2 - 10);

  if (currentLevel == NUM_LEVELS - 1)
  {
    tft.drawString("LEFT: Replay", tft.width() / 2, tft.height() / 2 + 70);
    tft.drawString("RIGHT: Restart", tft.width() / 2, tft.height() / 2 + 95);
  }

  else
  {
    tft.drawString("LEFT: Replay", tft.width() / 2, tft.height() / 2 + 20);
    tft.drawString("RIGHT: Next",   tft.width() / 2, tft.height() / 2 + 45);
  }
  movesThisLevel = 0;
}

void showLevelIntroScreen() 
{
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);

  String msg = "Level " + String(currentLevel + 1) + "/6";
  tft.drawString(msg, tft.width() / 2, tft.height() / 2 - 20);

  delay(1500);
}

void updateMoveCounter()
{
  tft.setTextDatum(TR_DATUM);
  int x = tft.width() - 5;
  int y = 5;

  int clearWidth = 120;
  int clearHeight = 40;
  tft.fillRect(x - clearWidth, y, clearWidth, clearHeight, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Moves: " + String(movesThisLevel), x, y);
}

void updateTimerDisplay() 
{
  tft.setTextDatum(TL_DATUM);
  int x = 5, y = 5;
  tft.fillRect(x, y, 100, 20, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Time: " + String(timeRemaining), x, y);
}

void triggerGameOver() 
{
  gameOver = true;
  currentLevel = 0;
  totalMoves = 0;
  movesThisLevel = 0;

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_RED);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString("Game Over", tft.width() / 2, tft.height() / 2 - 20);

  tft.setTextSize(2);
  tft.drawString("Returning to Level 1", tft.width() / 2, tft.height() / 2 + 20);

  delay(2000);
  showLevelIntroScreen();
  loadLevel(currentLevel);
}