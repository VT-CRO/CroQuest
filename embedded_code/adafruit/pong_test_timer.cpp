#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Screen
#define TFT_MISO 12
#define TFT_LED  21
#define TFT_SCK  14
#define TFT_MOSI 13
#define TFT_DC   2
#define TFT_RESET 4
#define TFT_CS   15

// Buttons
#define BUTTON_UP    35
#define BUTTON_DOWN  36


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

int leftScore = 0;
int rightScore = 0;

unsigned long lastTimeUpdate = 0;  // Timer update interval
unsigned long startTime = 0;       // Store the start time
int elapsedTime = 0;               // Elapsed time in seconds
unsigned long previousMillis = 0;  // For non-blocking timer updates
const long interval = 1000;        // 1 second interval for timer update

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
  drawScores();  // Initial drawing of the scores
  
  pinMode(BUTTON_UP, INPUT_PULLUP);     // Button move up
  pinMode(BUTTON_DOWN, INPUT_PULLUP);   // Button move down 


  startTime = millis();  // Start the timer
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
  if (ballX < 0) {
    // Right player scores
    rightScore++;
    resetBall();
    drawScores();  // Update the score display only when a player scores
  }
  if (ballX > WIDTH) {
    // Left player scores
    leftScore++;
    resetBall();
    drawScores();  // Update the score display only when a player scores
  }

  // Move paddles
  rightY += rightDY;
  if (rightY <= 0 || rightY + paddleH >= HEIGHT) rightDY *= -1;

  // leftY += leftDY;   // Automatic Movement
  // if (leftY <= 0 || leftY + paddleH >= HEIGHT) leftDY *= -1;   // Automatic Movement
  

  // Read button states (LOW means pressed due to INPUT_PULLUP)
  bool upPressed = digitalRead(BUTTON_UP) == LOW;
  bool downPressed = digitalRead(BUTTON_DOWN) == LOW;

  // Move left paddle manually with buttons
  if (upPressed && leftY > 0) {
    leftY -= 3;
  }
  if (downPressed && leftY + paddleH < HEIGHT) {
    leftY += 3;
  }


  // Clear only old positions
  drawBall(oldBallX, oldBallY, ILI9341_BLACK);
  drawPaddle(leftX, oldLeftY, ILI9341_BLACK);
  drawPaddle(rightX, oldRightY, ILI9341_BLACK);

  // Draw new positions
  drawBall(ballX, ballY, ILI9341_WHITE);
  drawPaddle(leftX, leftY, ILI9341_GREEN);
  drawPaddle(rightX, rightY, ILI9341_GREEN);

  // Non-blocking timer update every second
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time we updated the timer
    previousMillis = currentMillis;
    
    // Increment elapsed time every second
    elapsedTime = (currentMillis - startTime) / 1000;

    // Only update the timer text, no need to redraw everything
    // updateTimer();
  }
}

void drawBall(int x, int y, uint16_t color) {
  tft.fillRect(x, y, ballSize, ballSize, color);
}

void drawPaddle(int x, int y, uint16_t color) {
  tft.fillRect(x, y, paddleW, paddleH, color);
}

void drawScores() {
  // Draw the scores at the beginning or after a score update
  tft.setCursor(10, 10);
  tft.print("L: ");
  tft.print(leftScore);
  
  tft.setCursor(WIDTH - 60, 10);
  tft.print("R: ");
  tft.print(rightScore);
}

// void updateTimer() {
//   // Clear only the area where the timer is displayed (no full screen redraw)
//   tft.fillRect(WIDTH / 2 - 50, 10, 100, 30, ILI9341_BLACK);

//   // Draw updated timer
//   tft.setCursor(WIDTH / 2 - 40, 10);
//   tft.print("Time: ");
//   tft.print(elapsedTime);
//   tft.print("s");
// }

void resetBall() {
  ballX = WIDTH / 2;
  ballY = HEIGHT / 2;
  ballDX *= -1;
}


