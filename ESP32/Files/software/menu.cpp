#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Uses setup in User_Setup.h

#define TFT_LED 21
#define BUTTON_RIGHT 34
#define BUTTON_LEFT 35
#define BUTTON_SELECT 22

void drawDecoratedGrid();
void redrawChangedSquares();
void updateSelection();
void handleGameSelection();
void drawSingleSquare(int row, int col, bool isSelected);

TFT_eSPI tft = TFT_eSPI(); // TFT instance

const int screenWidth = 480;
const int screenHeight = 320;

const int gridRows = 3;
const int gridCols = 3;
const int cellWidth = screenWidth / gridCols;
const int cellHeight = screenHeight / gridRows;
const int padding = 10;
const int cornerRadius = 12;

// Game names
const char *gameNames[gridRows][gridCols] = {{"Snake", "Pong", "Mario"},
                                             {"Tetris", "Breakout", "Flappy"},
                                             {"Asteroids", "Racer", "Shooter"}};

int selectedRow = 0;
int selectedCol = 0;
int prevRow = 0;
int prevCol = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  drawDecoratedGrid();
}

void loop() {
  updateSelection();
  handleGameSelection();
}

void drawDecoratedGrid() {
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;

  for (int row = 0; row < gridRows; row++) {
    for (int col = 0; col < gridCols; col++) {
      int x = offsetX + col * cellWidth;
      int y = offsetY + row * cellHeight;
      int squareWidth = cellWidth - 2 * padding;
      int squareHeight = cellHeight - 2 * padding;

      uint16_t fillColor = TFT_BLUE;
      uint16_t borderColor =
          (row == selectedRow && col == selectedCol) ? TFT_RED : TFT_WHITE;

      tft.fillRoundRect(x + padding, y + padding, squareWidth, squareHeight,
                        cornerRadius, fillColor);

      int thickness = (row == selectedRow && col == selectedCol) ? 3 : 1;
      for (int i = 0; i < thickness; i++) {
        tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i,
                          squareHeight + 2 * i, cornerRadius + i, borderColor);
      }

      tft.drawString(gameNames[row][col], x + cellWidth / 2,
                     y + cellHeight / 2);
    }
  }
}

void redrawChangedSquares() {
  drawSingleSquare(prevRow, prevCol, false);
  drawSingleSquare(selectedRow, selectedCol, true);
}

void drawSingleSquare(int row, int col, bool isSelected) {
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;
  int x = offsetX + col * cellWidth;
  int y = offsetY + row * cellHeight;
  int squareWidth = cellWidth - 2 * padding;
  int squareHeight = cellHeight - 2 * padding;

  for (int i = 0; i < 3; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i,
                      squareHeight + 2 * i, cornerRadius + i, TFT_BLUE);
  }

  int thickness = isSelected ? 3 : 1;
  uint16_t borderColor = isSelected ? TFT_RED : TFT_WHITE;
  for (int i = 0; i < thickness; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i,
                      squareHeight + 2 * i, cornerRadius + i, borderColor);
  }

  tft.drawString(gameNames[row][col], x + cellWidth / 2, y + cellHeight / 2);
}

void updateSelection() {
  bool rightPressed = digitalRead(BUTTON_RIGHT) == LOW;
  bool leftPressed = digitalRead(BUTTON_LEFT) == LOW;

  static bool leftLast = false;
  static bool rightLast = false;

  prevRow = selectedRow;
  prevCol = selectedCol;

  if (leftPressed && !leftLast) {
    if (selectedRow == 0 && selectedCol == 0) {
      selectedRow = gridRows - 1;
      selectedCol = gridCols - 1;
    } else if (selectedCol > 0) {
      selectedCol--;
    } else {
      selectedCol = gridCols - 1;
      selectedRow--;
    }
    redrawChangedSquares();
  }

  if (rightPressed && !rightLast) {
    if (selectedRow == gridRows - 1 && selectedCol == gridCols - 1) {
      selectedRow = 0;
      selectedCol = 0;
    } else if (selectedCol < gridCols - 1) {
      selectedCol++;
    } else {
      selectedCol = 0;
      selectedRow++;
    }
    redrawChangedSquares();
  }

  leftLast = leftPressed;
  rightLast = rightPressed;
}

void handleGameSelection() {
  static bool selectLast = false;
  bool selectPressed = digitalRead(BUTTON_SELECT) == LOW;

  if (selectPressed && !selectLast) {
    const char *selectedGame = gameNames[selectedRow][selectedCol];
    Serial.print("Launching: ");
    Serial.println(selectedGame);

    // Game launching logic here
  }

  selectLast = selectPressed;
}
