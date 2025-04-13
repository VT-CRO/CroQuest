#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Define all TFT pins
#define TFT_MISO 12
#define TFT_LED  21
#define TFT_SCK  14
#define TFT_MOSI 13
#define TFT_DC   2
#define TFT_RESET 4
#define TFT_CS   15

// Buttons
#define BUTTON_RIGHT 36
#define BUTTON_LEFT 35


// Initialize the TFT display with all pins
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);

// Screen dimensions
const int screenWidth = 320;
const int screenHeight = 240;

// Grid dimensions
const int gridRows = 3;
const int gridCols = 3;
const int cellWidth = screenWidth / gridCols;
const int cellHeight = screenHeight / gridRows;
const int padding = 10;  // Padding inside each square
const int cornerRadius = 12;  // Radius for rounded corners

// Menu Items
int selectedRow = 0;
int selectedCol = 0;

// Squares Switching
int prevRow = 0;
int prevCol = 0;


// Border colors
uint16_t selectedBorderColor = ILI9341_YELLOW;
uint16_t unselectedBorderColor = ILI9341_WHITE;


void setup() {
  Serial.begin(115200);

  // Turn on screen backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);


  // Start Pins Buttons
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);  // Button move right
  pinMode(BUTTON_LEFT, INPUT_PULLUP);   // Button move left

  // Initialize TFT screen
  tft.begin(40000000);  // 40 MHz SPI
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);


  // Draw the 3x3 grid with decoration
  drawDecoratedGrid();
}

void loop() {
  updateSelection();
}



void drawDecoratedGrid() {
  // Calculate offset to center the grid
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;

  // Draw each square with rounded corners and padding
  for (int row = 0; row < gridRows; row++) {
    for (int col = 0; col < gridCols; col++) {
      // Calculate the top-left corner of the square
      int x = offsetX + col * cellWidth;
      int y = offsetY + row * cellHeight;

      // Adjust the size of each square to account for padding
      int squareWidth = cellWidth - 2 * padding;
      int squareHeight = cellHeight - 2 * padding;


      uint16_t borderColor = (row == selectedRow && col == selectedCol) ? ILI9341_RED : ILI9341_WHITE;

      // Draw the filled square with rounded corners
      tft.fillRoundRect(x + padding, y + padding, squareWidth, squareHeight, cornerRadius, ILI9341_BLUE);
      
      // Draw a thicker border for the selected square
      if (row == selectedRow && col == selectedCol) {
        for (int i = 0; i < 3; i++) { // thickness of 3
          tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2*i, squareHeight + 2*i, cornerRadius + i, ILI9341_RED);
        }
      } else {
        tft.drawRoundRect(x + padding, y + padding, squareWidth, squareHeight, cornerRadius, ILI9341_WHITE);
      }
        }
      }
}

void redrawChangedSquares() {
  drawSingleSquare(prevRow, prevCol, false);          // Unselect previous
  drawSingleSquare(selectedRow, selectedCol, true);   // Select new
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
      // If we're at col1, row1, go to col3, row3
      selectedRow = gridRows - 1;
      selectedCol = gridCols - 1;
    } else if (selectedCol > 0) {
      // Move left within the current row
      selectedCol--;
    } else {
      // Wrap to the last column of the previous row
      selectedCol = gridCols - 1;
      selectedRow--;
    }
    redrawChangedSquares();
  }

  if (rightPressed && !rightLast) {
    if (selectedRow == gridRows - 1 && selectedCol == gridCols - 1) {
      // If we're at col3, row3, go to col1, row1
      selectedRow = 0;
      selectedCol = 0;
    } else if (selectedCol < gridCols - 1) {
      // Move right within the current row
      selectedCol++;
    } else {
      // Wrap to the first column of the next row
      selectedCol = 0;
      selectedRow++;
    }
    redrawChangedSquares();
  }

  leftLast = leftPressed;
  rightLast = rightPressed;
}


void drawSingleSquare(int row, int col, bool isSelected) {
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;

  int x = offsetX + col * cellWidth;
  int y = offsetY + row * cellHeight;
  int squareWidth = cellWidth - 2 * padding;
  int squareHeight = cellHeight - 2 * padding;

  // First, "erase" the previous border (draw over it with square's fill color)
  for (int i = 0; i < 3; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i,
                      squareWidth + 2 * i, squareHeight + 2 * i, cornerRadius + i, ILI9341_BLUE);
  }

  // Then, draw the current border in the correct color
  uint16_t borderColor = isSelected ? ILI9341_RED : ILI9341_WHITE;
  int borderThickness = isSelected ? 3 : 1;

  for (int i = 0; i < borderThickness; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i,
                      squareWidth + 2 * i, squareHeight + 2 * i, cornerRadius + i, borderColor);
  }
}







