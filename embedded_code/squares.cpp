#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_MISO 12
#define TFT_LED  21
#define TFT_SCK  14
#define TFT_MOSI 13
#define TFT_DC   2
#define TFT_RESET 4
#define TFT_CS   15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);

#define MAX_SQUARES 10
int squareSize = 20;

struct Square {
  int x, y;
  int dx, dy;
  uint16_t color;
};

Square squares[MAX_SQUARES];

int currentPhase = 0;
int phaseLengths[] = {1, 5, 10}; // Number of squares per phase
int numPhases = sizeof(phaseLengths) / sizeof(phaseLengths[0]);

unsigned long phaseStartTime = 0;

void initSquares(int count) {
  for (int i = 0; i < count; i++) {
    squares[i].x = random(0, tft.width() - squareSize);
    squares[i].y = random(0, tft.height() - squareSize - 30); // leave room for text
    squares[i].dx = 2 + random(3);
    squares[i].dy = 2 + random(3);
    squares[i].color = random(0xFFFF);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.begin(40000000);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  initSquares(phaseLengths[currentPhase]);
  phaseStartTime = millis();
}

void drawSquare(int x, int y, uint16_t color) {
  tft.fillRect(x, y, squareSize, squareSize, color);
}

void eraseSquare(int x, int y) {
  tft.fillRect(x, y, squareSize, squareSize, ILI9341_BLACK);
}

void moveSquares(int count) {
  for (int i = 0; i < count; i++) {
    int oldX = squares[i].x;
    int oldY = squares[i].y;

    squares[i].x += squares[i].dx;
    squares[i].y += squares[i].dy;

    // Collision detection and reversing direction when hitting the edge
    if (squares[i].x < 0 || squares[i].x + squareSize > tft.width()) squares[i].dx *= -1;
    if (squares[i].y < 0 || squares[i].y + squareSize > tft.height() - 30) squares[i].dy *= -1;

    // Erase old position and draw new position
    eraseSquare(oldX, oldY);
    drawSquare(squares[i].x, squares[i].y, squares[i].color);
  }
}

void loop() {
  unsigned long now = millis();
  int count = phaseLengths[currentPhase];

  // Move and update squares on the screen
  moveSquares(count);

  // Advance to next phase after 5 seconds
  if (now - phaseStartTime >= 5000) { // 5 seconds per phase
    currentPhase++;
    if (currentPhase >= numPhases) {
      showFinalResults();
      while (true); // Halt after test
    } else {
      tft.fillScreen(ILI9341_BLACK);
      initSquares(phaseLengths[currentPhase]);
      phaseStartTime = millis();
    }
  }
}

void showFinalResults() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(ILI9341_GREEN);
  tft.print("Benchmark Results:");

  for (int i = 0; i < numPhases; i++) {
    tft.setCursor(20, 80 + i * 30);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Phase ");
    tft.print(i + 1);
    tft.print(" (");
    tft.print(phaseLengths[i]);
    tft.print(" squares)");
  }
}

