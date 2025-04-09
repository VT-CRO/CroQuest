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

const int WIDTH = 320;
const int HEIGHT = 240;

int ballX = 160, ballY = 120;
int ballDX = 3, ballDY = 2;
const int ballSize = 4;

const int paddleW = 6;
const int paddleH = 30;
const int leftX = 4;
const int rightX = WIDTH - 4 - paddleW;
int leftY = 100, rightY = 100;
int leftDY = 2, rightDY = -2;

unsigned long lastFpsUpdate = 0;
int frameCount = 0, fps = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  tft.begin(40000000);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  drawBall(ballX, ballY, ILI9341_WHITE);
  drawPaddle(leftX, leftY, ILI9341_GREEN);
  drawPaddle(rightX, rightY, ILI9341_GREEN);
}

void loop() {
  // Save old positions
  int oldBallX = ballX, oldBallY = ballY;
  int oldLeftY = leftY;
  int oldRightY = rightY;

  // Update ball position
  ballX += ballDX;
  ballY += ballDY;

  // Bounce
  if (ballY <= 0 || ballY + ballSize >= HEIGHT) ballDY *= -1;

  // Left paddle collision
  if (ballX <= leftX + paddleW &&
      ballY + ballSize >= leftY &&
      ballY <= leftY + paddleH) {
    ballDX *= -1;
    ballX = leftX + paddleW + 1;
  }

  // Right paddle collision
  if (ballX + ballSize >= rightX &&
      ballY + ballSize >= rightY &&
      ballY <= rightY + paddleH) {
    ballDX *= -1;
    ballX = rightX - ballSize - 1;
  }

  // Reset if ball goes out
  if (ballX < 0 || ballX > WIDTH) {
    ballX = WIDTH / 2;
    ballY = HEIGHT / 2;
    ballDX *= -1;
  }

  // Move paddles
  leftY += leftDY;
  rightY += rightDY;
  if (leftY <= 0 || leftY + paddleH >= HEIGHT) leftDY *= -1;
  if (rightY <= 0 || rightY + paddleH >= HEIGHT) rightDY *= -1;

  // Clear only old positions
  drawBall(oldBallX, oldBallY, ILI9341_BLACK);
  drawPaddle(leftX, oldLeftY, ILI9341_BLACK);
  drawPaddle(rightX, oldRightY, ILI9341_BLACK);

  // Draw new positions
  drawBall(ballX, ballY, ILI9341_WHITE);
  drawPaddle(leftX, leftY, ILI9341_GREEN);
  drawPaddle(rightX, rightY, ILI9341_GREEN);

  // FPS tracking
  frameCount++;
  unsigned long now = millis();
  if (now - lastFpsUpdate >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFpsUpdate = now;

    // Display to serial only
    Serial.print("FPS: ");
    Serial.println(fps);
  }
}

void drawBall(int x, int y, uint16_t color) {
  tft.fillRect(x, y, ballSize, ballSize, color);
}

void drawPaddle(int x, int y, uint16_t color) {
  tft.fillRect(x, y, paddleW, paddleH, color);
}

