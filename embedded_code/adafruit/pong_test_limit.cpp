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

// Increase ball and paddle size
const int ballSize = 8;  // Larger balls
const int paddleW = 12;  // Larger paddles
const int paddleH = 60;  // Larger paddles

// Ball positions and velocities
struct Ball {
  int x, y;
  int dx, dy;
};
Ball balls[5];

// Paddle positions and velocities
struct Paddle {
  int x, y;
  int dy;
};
Paddle paddles[6]; // 2 initial paddles, 4 additional paddles

unsigned long lastFpsUpdate = 0;
int frameCount = 0, fps = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  tft.begin(40000000);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);  // Start with a black screen
  
  // Draw a gradient background
  drawBackground();

  // Initialize balls
  for (int i = 0; i < 5; i++) {
    balls[i].x = random(WIDTH / 2);
    balls[i].y = random(HEIGHT / 2);
    balls[i].dx = 3 + random(3);
    balls[i].dy = 2 + random(3);
  }

  // Initialize paddles
  paddles[0].x = 4;  paddles[0].y = 100; paddles[0].dy = 2;   // Left paddle
  paddles[1].x = WIDTH - 4 - paddleW; paddles[1].y = 100; paddles[1].dy = -2; // Right paddle

  // Additional paddles
  for (int i = 2; i < 6; i++) {
    paddles[i].x = random(WIDTH - paddleW);
    paddles[i].y = random(HEIGHT - paddleH);
    paddles[i].dy = random(2) == 0 ? 2 : -2;
  }

  drawAll();
}

void loop() {
  // Save old positions
  Ball oldBalls[5];
  for (int i = 0; i < 5; i++) {
    oldBalls[i] = balls[i];
  }

  Paddle oldPaddles[6];
  for (int i = 0; i < 6; i++) {
    oldPaddles[i] = paddles[i];
  }

  // Update ball positions
  for (int i = 0; i < 5; i++) {
    balls[i].x += balls[i].dx;
    balls[i].y += balls[i].dy;

    // Bounce balls off top and bottom
    if (balls[i].y <= 0 || balls[i].y + ballSize >= HEIGHT) balls[i].dy *= -1;

    // Reset balls if they go out of bounds
    if (balls[i].x < 0 || balls[i].x > WIDTH) {
      balls[i].x = WIDTH / 2;
      balls[i].y = HEIGHT / 2;
      balls[i].dx *= -1;
    }
  }

  // Move paddles
  for (int i = 0; i < 6; i++) {
    paddles[i].y += paddles[i].dy;
    if (paddles[i].y <= 0 || paddles[i].y + paddleH >= HEIGHT) paddles[i].dy *= -1;
  }

  // Check for ball and paddle collisions
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
      if (balls[i].x <= paddles[j].x + paddleW && balls[i].x + ballSize >= paddles[j].x &&
          balls[i].y + ballSize >= paddles[j].y && balls[i].y <= paddles[j].y + paddleH) {
        balls[i].dx *= -1;
        balls[i].x = paddles[j].x + paddleW + 1;
      }
    }
  }

  // Clear old positions by redrawing the background
  for (int i = 0; i < 5; i++) {
    drawBackgroundRegion(oldBalls[i].x, oldBalls[i].y, ballSize, ballSize);
  }

  for (int i = 0; i < 6; i++) {
    drawBackgroundRegion(oldPaddles[i].x, oldPaddles[i].y, paddleW, paddleH);
  }

  // Draw new positions
  for (int i = 0; i < 5; i++) {
    drawBall(balls[i].x, balls[i].y, ILI9341_WHITE);
  }

  for (int i = 0; i < 6; i++) {
    drawPaddle(paddles[i].x, paddles[i].y, ILI9341_GREEN);
  }

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

void drawAll() {
  // Draw all initial elements
  for (int i = 0; i < 5; i++) {
    drawBall(balls[i].x, balls[i].y, ILI9341_WHITE);
  }

  for (int i = 0; i < 6; i++) {
    drawPaddle(paddles[i].x, paddles[i].y, ILI9341_GREEN);
  }
}

void drawBackground() {
  // Create a gradient background for a more detailed look
  for (int i = 0; i < HEIGHT; i++) {
    int r = map(i, 0, HEIGHT, 0, 50);   // Red gradient from top to bottom
    int g = map(i, 0, HEIGHT, 0, 100);  // Green gradient from top to bottom
    int b = map(i, 0, HEIGHT, 255, 0);  // Blue gradient from top to bottom
    tft.drawLine(0, i, WIDTH, i, tft.color565(r, g, b));
  }
}

void drawBackgroundRegion(int x, int y, int w, int h) {
  // Redraw the gradient background over the area to prevent black lines
  for (int i = y; i < y + h; i++) {
    int r = map(i, 0, HEIGHT, 0, 50);   // Red gradient from top to bottom
    int g = map(i, 0, HEIGHT, 0, 100);  // Green gradient from top to bottom
    int b = map(i, 0, HEIGHT, 255, 0);  // Blue gradient from top to bottom
    tft.drawLine(x, i, x + w, i, tft.color565(r, g, b));
  }
}

