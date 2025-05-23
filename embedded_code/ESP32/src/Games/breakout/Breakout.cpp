#include "Breakout.hpp"

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"

#define SCREEN_W 480
#define SCREEN_H 320

#define PADDLE_WIDTH 60 // Smaller paddle
#define PADDLE_HEIGHT 12
#define BALL_RADIUS 6

#define BRICK_COLS 10
#define BRICK_ROWS 7
#define BRICK_WIDTH (SCREEN_W / BRICK_COLS)
#define BRICK_HEIGHT 12 // Smaller bricks

#define BRICK_SPACING_X 4
#define BRICK_SPACING_Y 4

struct Brick {
  int x, y;
  bool active;
  uint16_t color;
};

static Brick bricks[BRICK_ROWS][BRICK_COLS];
static int paddleX;
static float ballXf, ballYf;
static int ballX, ballY;
static int lastBallX = -1, lastBallY = -1;
static int lastPaddleX = -1;
static int ballVX = 2, ballVY = -2;
static bool ballMoving = false;
static int lives = 3;
static int score = 0;
static int lastLives = -1;
static int lastScore = -1;
static float ballSpeed;

unsigned long breakout_lastButtonPressTime = 0;
unsigned long breakout_buttonDebounceDelay = 200;
int breakoutGameOverSelection = 0;

int breakout_selection = 0;
int breakout_subselection = 0;
BreakoutState currentBreakoutState = BREAKOUT_HOMESCREEN;

// Numpad
// Numpad
static NumPad<BreakoutState> pad(
    drawBreakoutHomeScreen, // What to show when exiting pad
    []() { currentBreakoutState = BREAKOUT_PLAYING; }, // What to do on confirm
    &currentBreakoutState, BREAKOUT_HOMESCREEN,
    BREAKOUT_PLAYING // or another post-confirm state
);

const uint16_t rainbow[] = {TFT_RED,  0xFDA0, TFT_YELLOW, TFT_GREEN,
                            TFT_BLUE, 0x8010, 0xF81F};

// ======================== Game Entry ========================
void runBreakout() {
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  int screen_width = tft.width();
  int screen_height = tft.height();

  paddleX = SCREEN_W / 2 - PADDLE_WIDTH / 2;
  lives = 3;
  score = 0;
  initBricks();
  resetBall();

  while (true) {
    handleBreakoutFrame();
    if (currentBreakoutState == BREAKOUT_HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu from Breakout");
      delay(500);
      return;
    }
    delay(16);
  }
}

// ========== MANUAL LOOP ==========
void handleBreakoutFrame() {
  static unsigned long lastFrameTime = 0;
  static int lastSelection = -1;
  static int lastSubselection = -1;

  switch (currentBreakoutState) {

  case BREAKOUT_HOMESCREEN:
    if (millis() - lastFrameTime > 150) {
      if (up.wasJustPressed() && breakout_selection == 1) {
        breakout_selection = 0;
        drawBreakoutHomeSelection();
      } else if (down.wasJustPressed() && breakout_selection == 0) {
        breakout_selection = 1;
        drawBreakoutHomeSelection();
      } else if (A.wasJustPressed()) {
        if (breakout_selection == 1) {
          currentBreakoutState = BREAKOUT_MULTIPLAYER_SELECTION;
          drawBreakoutHomeSelection();
        } else {
          currentBreakoutState = BREAKOUT_PLAYING;
          paddleX = SCREEN_W / 2 - PADDLE_WIDTH / 2;
          lives = 3;
          score = 0;
          lastLives = -1;
          lastScore = -1;
          initBricks();
          resetBall();
        }
      }
      lastFrameTime = millis();
    }

    if (breakout_selection != lastSelection ||
        breakout_subselection != lastSubselection) {
      drawBreakoutHomeScreen();
      lastSelection = breakout_selection;
      lastSubselection = breakout_subselection;
    }
    break;

  case BREAKOUT_MULTIPLAYER_SELECTION:
    if (millis() - lastFrameTime > 150) {
      if (left.wasJustPressed() && breakout_subselection == 1) {
        breakout_subselection = 0;
        drawBreakoutHomeSelection();
      } else if (right.wasJustPressed() && breakout_subselection == 0) {
        breakout_subselection = 1;
        drawBreakoutHomeSelection();
      } else if (A.wasJustPressed()) {
        if (breakout_subselection == 0) {

          currentBreakoutState = BREAKOUT_JOIN_SCREEN;
        } else {
          pad.numPadSetup();
          currentBreakoutState = BREAKOUT_BLUETOOTH_NUMPAD;
        }
      } else if (up.wasJustPressed()) {
        currentBreakoutState = BREAKOUT_HOMESCREEN;
        breakout_subselection = 0;
        breakout_selection = 1;
        drawBreakoutHomeSelection();
      }
      lastFrameTime = millis();
    }
    break;

  case BREAKOUT_PLAYING:
    if (millis() - lastFrameTime > 16) {
      updateBreakoutGame();
      drawBreakoutFrame();
      lastFrameTime = millis();
    }
    break;

  case BREAKOUT_WIN:
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(3);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("You Win!", SCREEN_W / 2, SCREEN_H / 2);
    if (Start.wasJustPressed()) {
      currentBreakoutState = BREAKOUT_HOMESCREEN;
      tft.fillScreen(TFT_BLACK);
    }
    break;

  case BREAKOUT_GAMEOVER:
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(3);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Game Over", SCREEN_W / 2, SCREEN_H / 2);
    if (Start.wasJustPressed()) {
      currentBreakoutState = BREAKOUT_HOMESCREEN;
      tft.fillScreen(TFT_BLACK);
    }
    break;

  case BREAKOUT_GAMEOVER_SCREEN:
    if (millis() - breakout_lastButtonPressTime > 150) {
      if (up.wasJustPressed() && breakoutGameOverSelection == 1) {
        breakoutGameOverSelection = 0;
        drawBreakoutGameOverSelect();
      } else if (down.wasJustPressed() && breakoutGameOverSelection == 0) {
        breakoutGameOverSelection = 1;
        drawBreakoutGameOverSelect();
      } else if (A.wasJustPressed()) {
        if (breakoutGameOverSelection == 0) {
          currentBreakoutState = BREAKOUT_HOMESCREEN;
          tft.fillScreen(TFT_BLACK);
          drawBreakoutHomeScreen();
        } else {
          currentBreakoutState = BREAKOUT_PLAYING;
          paddleX = SCREEN_W / 2 - PADDLE_WIDTH / 2;
          lives = 3;
          score = 0;
          lastLives = -1;
          lastScore = -1;
          initBricks();
          resetBall();
        }
        breakout_lastButtonPressTime = millis();
      }
    }
    break;

  case BREAKOUT_BLUETOOTH_NUMPAD:
    pad.handleButtonInput(&breakout_lastButtonPressTime,
                          breakout_buttonDebounceDelay / 2);
    break;
  }
}

void initBricks() {
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      bricks[row][col] = {col * BRICK_WIDTH,
                          40 + row * BRICK_HEIGHT, // Lowered bricks
                          true, rainbow[row]};
    }
  }
  tft.fillScreen(TFT_BLACK);
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &b = bricks[row][col];
      if (b.active) {
        tft.fillRect(b.x + BRICK_SPACING_X / 2, b.y + BRICK_SPACING_Y / 2,
                     BRICK_WIDTH - BRICK_SPACING_X,
                     BRICK_HEIGHT - BRICK_SPACING_Y, b.color);
      }
    }
  }
}

void drawBreakoutGameOverSelect() {
  const int textSize = 2;
  const int paddingX = 10;
  const int paddingY = 4;
  const int spacing = 10;

  const char *optionHome = "Press for homescreen";
  const char *optionRestart = "Press to restart";

  int centerX = SCREEN_W / 2;
  int centerY = SCREEN_H / 2;
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(textSize);

  int h = 16 * textSize + paddingY * 2;
  int yHome = centerY + 40;
  int yRestart = yHome + h + spacing;

  int wHome = tft.textWidth(optionHome);
  int wRestart = tft.textWidth(optionRestart);

  // Clear previous areas
  tft.fillRect(centerX - wHome / 2 - paddingX, yHome - h / 2,
               wHome + 2 * paddingX, h, TFT_BLACK);
  tft.fillRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2,
               wRestart + 2 * paddingX, h, TFT_BLACK);

  // Highlight selected
  if (breakout_selection == 0) {
    tft.drawRect(centerX - wHome / 2 - paddingX, yHome - h / 2,
                 wHome + 2 * paddingX, h, TFT_WHITE);
  } else {
    tft.drawRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2,
                 wRestart + 2 * paddingX, h, TFT_WHITE);
  }

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(optionHome, centerX, yHome);
  tft.drawString(optionRestart, centerX, yRestart);
}

void drawBreakoutGameOverScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER", SCREEN_W / 2, SCREEN_H / 2 - 40);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Your Score: " + String(score), SCREEN_W / 2, SCREEN_H / 2);

  drawBreakoutGameOverSelect();
}

void drawBreakoutHomeSelection() {
  int y_single = 180;
  int y_multi = 230;
  int y_sub = y_multi + 40;

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, SCREEN_W, 35, TFT_BLACK);
  tft.fillRect(0, y_multi - 15, SCREEN_W, 80, TFT_BLACK);

  if (breakout_selection == 0) {
    tft.setTextSize(3);
    tft.drawString("Press for Single-Player", SCREEN_W / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Press for Multiplayer", SCREEN_W / 2, y_multi);
  } else {
    tft.setTextSize(2);
    tft.drawString("Press for Single-Player", SCREEN_W / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Press for Multiplayer", SCREEN_W / 2, y_multi);

    if (currentBreakoutState == BREAKOUT_MULTIPLAYER_SELECTION) {
      const char *sub1 = "Host a Game";
      const char *sub2 = "Join a Game";

      tft.setTextSize(2);
      int padding_x = 10;
      int padding_y = 4;
      int boxHeight = 20 + padding_y * 2;

      int sub1Width = tft.textWidth(sub1);
      int sub2Width = tft.textWidth(sub2);
      int sub1BoxWidth = sub1Width + padding_x * 2;
      int sub2BoxWidth = sub2Width + padding_x * 2;

      int x_sub1 = SCREEN_W / 4;
      int x_sub2 = 3 * SCREEN_W / 4;

      if (breakout_subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (breakout_subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}

void drawBreakoutHomeScreen() {
  tft.fillScreen(TFT_BLACK);

  // Title
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(4);
  tft.drawString("BREAKOUT", SCREEN_W / 2, 40);

  // Tagline
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Brick Buster Challenge", SCREEN_W / 2, 90);

  // Options
  tft.setTextSize(2);
  tft.drawString("Press for Single-Player", SCREEN_W / 2, 180);
  tft.drawString("Press for Multiplayer", SCREEN_W / 2, 230);

  drawBreakoutHomeSelection();
}

void resetBall() {
  ballSpeed = 8.0f; // Reasonable fast starting speed

  ballX = paddleX + PADDLE_WIDTH / 2;
  ballY = SCREEN_H - 30;
  ballXf = ballX;
  ballYf = ballY;

  // Set direction and scale to match ballSpeed
  float angle = -PI / 4; // 45° upward
  ballVX = cos(angle) * ballSpeed;
  ballVY = sin(angle) * ballSpeed;

  ballMoving = false;
}

void eraseOldBall() {
  if (lastBallX != -1 && lastBallY != -1) {
    tft.fillCircle(lastBallX, lastBallY, BALL_RADIUS, TFT_BLACK);
  }
}

void drawBall() {
  eraseOldBall();
  tft.fillCircle(ballX, ballY, BALL_RADIUS, TFT_WHITE);
  lastBallX = ballX;
  lastBallY = ballY;
}

void eraseOldPaddle() {
  if (lastPaddleX != -1) {
    tft.fillRect(lastPaddleX, SCREEN_H - 20, PADDLE_WIDTH, PADDLE_HEIGHT,
                 TFT_BLACK);
  }
}

void drawPaddle() {
  if (paddleX != lastPaddleX) {
    eraseOldPaddle();
    tft.fillRect(paddleX, SCREEN_H - 20, PADDLE_WIDTH, PADDLE_HEIGHT,
                 TFT_LIGHTGREY);
    lastPaddleX = paddleX;
  }
}

void drawHUD() {
  if (lives != lastLives || score != lastScore) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.fillRect(0, 0, SCREEN_W, 20, TFT_BLACK); // Clear top bar

    // Draw a single ball icon
    int ballX = 20;
    int ballY = 20;
    tft.fillCircle(ballX, ballY, BALL_RADIUS, TFT_WHITE);

    // Draw " x N" next to it
    tft.drawString(" x " + String(lives), ballX + BALL_RADIUS * 2 + 20, 20);

    // Draw score
    tft.drawString("Score: " + String(score), SCREEN_W - 80, 20); // Top right
    lastLives = lives;
    lastScore = score;
  }
}

void updateBreakoutGame() {
  if (left.wasJustPressed())
    paddleX -= 10;
  if (right.wasJustPressed())
    paddleX += 10;
  if (paddleX < 0)
    paddleX = 0;
  if (paddleX + PADDLE_WIDTH > SCREEN_W)
    paddleX = SCREEN_W - PADDLE_WIDTH;

  if (!ballMoving && A.wasJustPressed())
    ballMoving = true;

  if (ballMoving) {
    moveBallSafely();

    if (ballX <= 0 || ballX >= SCREEN_W)
      ballVX *= -1;
    if (ballY <= 0)
      ballVY *= -1;

    if (ballY + BALL_RADIUS >= SCREEN_H - 20 && ballX >= paddleX &&
        ballX <= paddleX + PADDLE_WIDTH) {
      ballVY *= -1;
    }

    if (ballY > SCREEN_H) {
      lives--;
      if (lives <= 0) {
        currentBreakoutState = BREAKOUT_GAMEOVER;
      } else {
        resetBall();
      }
    }

    for (int row = 0; row < BRICK_ROWS; ++row) {
      for (int col = 0; col < BRICK_COLS; ++col) {
        Brick &b = bricks[row][col];
        if (!b.active)
          continue;

        if (ballX > b.x && ballX < b.x + BRICK_WIDTH && ballY > b.y &&
            ballY < b.y + BRICK_HEIGHT) {

          b.active = false;
          tft.fillRect(b.x + BRICK_SPACING_X / 2, b.y + BRICK_SPACING_Y / 2,
                       BRICK_WIDTH - BRICK_SPACING_X,
                       BRICK_HEIGHT - BRICK_SPACING_Y, TFT_BLACK);
          score += 10;

          // Determine side of impact
          int ballCenterX = ballX;
          int ballCenterY = ballY;

          bool hitSide =
              (ballCenterX <= b.x || ballCenterX >= b.x + BRICK_WIDTH);
          bool hitTopBottom =
              (ballCenterY <= b.y || ballCenterY >= b.y + BRICK_HEIGHT);

          if (hitSide && !hitTopBottom) {
            ballVX *= -1;
          } else {
            ballVY *= -1;
          }

          return; // prevent multiple brick hits in one frame
        }
      }
    }

    bool allCleared = true;
    for (int row = 0; row < BRICK_ROWS; ++row)
      for (int col = 0; col < BRICK_COLS; ++col)
        if (bricks[row][col].active)
          allCleared = false;
    if (allCleared)
      currentBreakoutState = BREAKOUT_WIN;
  }
}

void drawBreakoutFrame() {
  drawPaddle();
  drawBall();
  drawHUD();
}

void moveBallSafely() {
  int steps = ceil(ballSpeed);
  float stepX = ballVX / (float)steps;
  float stepY = ballVY / (float)steps;

  for (int i = 0; i < steps; ++i) {
    ballXf += stepX;
    ballYf += stepY;

    ballX = round(ballXf);
    ballY = round(ballYf);

    if (checkBallCollisions())
      break;
  }
}

bool checkBallCollisions() {
  // Bounce off left wall
  if (ballXf - BALL_RADIUS <= 0) {
    ballXf = BALL_RADIUS + 1;
    ballVX = abs(ballVX);

    float mag = sqrt(ballVX * ballVX + ballVY * ballVY);
    ballVX = (ballVX / mag) * ballSpeed;
    ballVY = (ballVY / mag) * ballSpeed;

    ballX = round(ballXf);
    return true;
  }

  // Bounce off right wall
  if (ballXf + BALL_RADIUS >= SCREEN_W) {
    ballXf = SCREEN_W - BALL_RADIUS - 1;
    ballVX = -abs(ballVX);

    float mag = sqrt(ballVX * ballVX + ballVY * ballVY);
    ballVX = (ballVX / mag) * ballSpeed;
    ballVY = (ballVY / mag) * ballSpeed;

    ballX = round(ballXf);
    return true;
  }

  // Bounce off top wall
  if (ballYf - BALL_RADIUS <= 0) {
    ballYf = BALL_RADIUS + 1;
    ballVY = abs(ballVY);

    float mag = sqrt(ballVX * ballVX + ballVY * ballVY);
    ballVX = (ballVX / mag) * ballSpeed;
    ballVY = (ballVY / mag) * ballSpeed;

    ballY = round(ballYf);
    return true;
  }

  // Paddle
  if (ballVY > 0 && ballYf + BALL_RADIUS >= SCREEN_H - 20 &&
      ballYf - ballVY + BALL_RADIUS <= SCREEN_H - 20 && ballX >= paddleX &&
      ballX <= paddleX + PADDLE_WIDTH) {

    ballYf = SCREEN_H - 20 - BALL_RADIUS - 1;
    ballY = round(ballYf);

    ballVY = -abs(ballVY);

    // Avoid getting stuck with very small horizontal motion
    if (abs(ballVX) < 0.5f)
      ballVX = (ballVX < 0) ? -0.5f : 0.5f;

    float mag = sqrt(ballVX * ballVX + ballVY * ballVY);
    ballVX = (ballVX / mag) * ballSpeed;
    ballVY = (ballVY / mag) * ballSpeed;

    return true;
  }

  // Bricks
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &b = bricks[row][col];
      if (!b.active)
        continue;

      if (ballX > b.x && ballX < b.x + BRICK_WIDTH && ballY > b.y &&
          ballY < b.y + BRICK_HEIGHT) {

        b.active = false;
        tft.fillRect(b.x + BRICK_SPACING_X / 2, b.y + BRICK_SPACING_Y / 2,
                     BRICK_WIDTH - BRICK_SPACING_X,
                     BRICK_HEIGHT - BRICK_SPACING_Y, TFT_BLACK);

        // Bounce
        ballVY *= -1;

        float currentSpeed = sqrt(ballVX * ballVX + ballVY * ballVY);
        float newSpeed = min(currentSpeed + 0.5f, 12.0f);

        float directionX = ballVX / currentSpeed;
        float directionY = ballVY / currentSpeed;

        ballVX = directionX * newSpeed;
        ballVY = directionY * newSpeed;

        ballSpeed = newSpeed;
        score += 10;

        return true;
      }
    }
  }

  return false;
}