#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// TFT pins
#define TFT_MISO 12
#define TFT_LED  21
#define TFT_SCK  14
#define TFT_MOSI 13
#define TFT_DC   2
#define TFT_RESET 4
#define TFT_CS   15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);

// Constants
const int WIDTH = 320;
const int HEIGHT = 240;

const int BALL_SIZE = 4;
const int PADDLE_W = 6;
const int PADDLE_H = 30;
const int LEFT_X = 4;
const int RIGHT_X = WIDTH - 4 - PADDLE_W;

// Game objects
int ballX = WIDTH / 2, ballY = HEIGHT / 2;
int ballDX = 3, ballDY = 2;
int leftY = 100, rightY = 100;
int leftDY = 2, rightDY = -2;

// FPS tracking
unsigned long lastFpsUpdate = 0;
int frameCount = 0, fps = 0;

void setup() {
  Serial.begin(115200);

  // Backlight on
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.begin(40000000); // 40 MHz SPI
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
}

void loop() {
  int oldBallX = ballX, oldBallY = ballY;
  int oldLeftY = leftY;
  int oldRightY = rightY;

  // Move ball
  ballX += ballDX;
  ballY += ballDY;

  // Bounce off top/bottom
  if (ballY <= 0 || ballY + BALL_SIZE >= HEIGHT) {
    ballDY *= -1;
  }

  // Bounce off paddles
  if (ballX <= LEFT_X + PADDLE_W &&
      ballY + BALL_SIZE >= leftY &&
      ballY <= leftY + PADDLE_H) {
    ballDX *= -1;
    ballX = LEFT_X + PADDLE_W + 1;
  }

  if (ballX + BALL_SIZE >= RIGHT_X &&
      ballY + BALL_SIZE >= rightY &&
      ballY <= rightY + PADDLE_H) {
    ballDX *= -1;
    ballX = RIGHT_X - BALL_SIZE - 1;
  }

  // Reset if out of bounds
  if (ballX < 0 || ballX > WIDTH) {
    ballX = WIDTH / 2;
    ballY = HEIGHT / 2;
    ballDX *= -1;
  }

  // Move paddles
  leftY += leftDY;
  rightY += rightDY;
  if (leftY <= 0 || leftY + PADDLE_H >= HEIGHT) leftDY *= -1;
  if (rightY <= 0 || rightY + PADDLE_H >= HEIGHT) rightDY *= -1;

  // Clear old objects
  drawBall(oldBallX, oldBallY, ILI9341_BLACK);
  drawPaddle(LEFT_X, oldLeftY, ILI9341_BLACK);
  drawPaddle(RIGHT_X, oldRightY, ILI9341_BLACK);

  // Draw new objects
  drawBall(ballX, ballY, ILI9341_WHITE);
  drawPaddle(LEFT_X, leftY, ILI9341_GREEN);
  drawPaddle(RIGHT_X, rightY, ILI9341_GREEN);

  // FPS counter
  frameCount++;
  unsigned long now = millis();
  if (now - lastFpsUpdate >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFpsUpdate = now;

    // Display FPS
    tft.fillRect(0, 0, 80, 20, ILI9341_BLACK);  // Clear previous text
    tft.setCursor(5, 2);
    tft.print("FPS: ");
    tft.print(fps);

    Serial.print("FPS: ");
    Serial.println(fps);
  }
}

// Drawing helpers
void drawBall(int x, int y, uint16_t color) {
  tft.fillRect(x, y, BALL_SIZE, BALL_SIZE, color);
}

void drawPaddle(int x, int y, uint16_t color) {
  tft.fillRect(x, y, PADDLE_W, PADDLE_H, color);
}

