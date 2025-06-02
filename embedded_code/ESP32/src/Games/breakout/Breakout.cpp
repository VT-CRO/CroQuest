// src/Games/breakout/Breakout.cpp

#include "Breakout.hpp"
#include "EndScreen/EndScreen.hpp"

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

#define SCREEN_W 480
#define SCREEN_H 320

#define SPEAKER_PIN 21

#define PADDLE_WIDTH 60 // Smaller paddle
#define PADDLE_HEIGHT 12
#define BALL_RADIUS 6

#define BRICK_COLS 10
#define BRICK_ROWS 7
#define BRICK_WIDTH (SCREEN_W / BRICK_COLS)
#define BRICK_HEIGHT 12 // Smaller bricks

#define BRICK_SPACING_X 4
#define BRICK_SPACING_Y 4

static Ball ball;
static Ball prev_ball;
static Paddle paddle;
static Paddle prev_paddle;

static Brick bricks[BRICK_ROWS][BRICK_COLS];

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
static NumPad<BreakoutState> pad(
    drawBreakoutHomeScreen, // What to show when exiting pad
    []() { currentBreakoutState = BREAKOUT_PLAYING; }, // What to do on confirm
    &currentBreakoutState, BREAKOUT_HOMESCREEN,
    BREAKOUT_PLAYING // or another post-confirm state
);

const uint16_t rainbow[] = {TFT_RED,  0xFDA0, TFT_YELLOW, TFT_GREEN,
                            TFT_BLUE, 0x8010, 0xF81F};

// ####################################################################################################
//  Launch Game
// ####################################################################################################

// ========== Run Game ========== //
void runBreakout() {
  resetExitFlag(); // Resets flag for Main Menu

  // === Set initial game state ===
  currentBreakoutState = BREAKOUT_HOMESCREEN;
  breakout_selection = 0;
  breakout_subselection = 0;
  breakoutGameOverSelection = 0;

  //clear sprite and cache
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  // === Initialize game data ===
  lives = 3;
  score = 0;
  lastLives = -1;
  lastScore = -1;
  ballMoving = false;

  // === Set paddle initial position ===
  paddle = {SCREEN_W / 2 - PADDLE_WIDTH / 2, SCREEN_H - 20, PADDLE_WIDTH,
            PADDLE_HEIGHT};
  prev_paddle = paddle;

  // === Ball will be placed above the paddle ===
  resetBall(); // <-- uses `ball` and `prev_ball` structs

  updateAllButtons();

  drawBreakoutHomeScreen();

  while (true) {
    handleBreakoutFrame();

    if (getExitFlag())
      return;

    if (currentBreakoutState == BREAKOUT_HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu");
      delay(500);
      return;
    }

    delay(16); // ~60 FPS
  }
}

// ####################################################################################################
//  Game Logic
// ####################################################################################################

// ========== MANUAL LOOP ========== //
void handleBreakoutFrame() {
  static unsigned long lastFrameTime = 0;
  static int lastSelection = -1;
  static int lastSubselection = -1;

  // check if the Start Button was pressed and goes back to Main Menu
  if (checkStartButtonAndExit(tft))
    return;

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
          paddle.x = SCREEN_W / 2 - PADDLE_WIDTH / 2;
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
      lastFrameTime = millis();
      updateBreakoutGame();
      drawBreakoutFrame();
    }
    break;

  case BREAKOUT_WIN:{
    // ENDSCREEN HANDLING
    std::vector<String> playerNames = {settings.name, "Win"};
    std::vector<int> playerScores = {score, -1};

    EndScreen endScreen(playerNames, playerScores, false, settings.name,
                        score);
    if (endScreen.handleUserInput()) {
          currentBreakoutState = BREAKOUT_PLAYING;
          paddle.x = SCREEN_W / 2 - PADDLE_WIDTH / 2;
          lives = 3;
          score = 0;
          lastLives = -1;
          lastScore = -1;
          initBricks();
          resetBall(); // handleUserInput returns true : game restarts
    } else {
      currentBreakoutState = BREAKOUT_HOMESCREEN;
      drawBreakoutHomeScreen(); // handleUserInput returns false : returns to game
                             // menu
    }
    break;
  }

  case BREAKOUT_GAMEOVER: {
    // ENDSCREEN HANDLING
    std::vector<String> playerNames = {settings.name};
    std::vector<int> playerScores = {score};

    EndScreen endScreen(playerNames, playerScores, false, settings.name,
                        score);
    if (endScreen.handleUserInput()) {
          currentBreakoutState = BREAKOUT_PLAYING;
          paddle.x = SCREEN_W / 2 - PADDLE_WIDTH / 2;
          lives = 3;
          score = 0;
          lastLives = -1;
          lastScore = -1;
          initBricks();
          resetBall(); // handleUserInput returns true : game restarts
    } else {
      currentBreakoutState = BREAKOUT_HOMESCREEN;
      drawBreakoutHomeScreen(); // handleUserInput returns false : returns to game
                             // menu
    }
    break;
  }

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
          paddle = {SCREEN_W / 2 - PADDLE_WIDTH / 2, SCREEN_H - 20,
                    PADDLE_WIDTH, PADDLE_HEIGHT};
          prev_paddle = paddle;
          resetBall();

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

// ========== Initialize Bricks Logic ========== //
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

// ========== Resets Ball Position ========== //
void resetBall() {
  ballSpeed = 8.0f;

  ball.w = ball.h = 2 * BALL_RADIUS;
  ball.x = paddle.x + paddle.w / 2 - ball.w / 2;
  ball.y = paddle.y - ball.h - 2;

  float angle = -PI / 4;
  ball.vx = cos(angle) * ballSpeed;
  ball.vy = sin(angle) * ballSpeed;

  prev_ball = ball;
  ballMoving = false;
}

// ========== Update Game Status ========== //
void updateBreakoutGame() {

  // Handle paddle movement
  updatePaddle(&paddle);

  // Move ball with paddle before game starts
  if (!ballMoving) {

    // Erase previous ball position
    eraseBall(&prev_ball);

    // Position ball above paddle and move it
    ball.x = paddle.x + (paddle.w - ball.w) / 2;
    ball.y = paddle.y - ball.h - 1;

    // Save current position for next erase
    prev_ball = ball;

    // Draw updated ball position
    drawBall(&ball);

    // Start game with A
    if (A.wasJustPressed()) {
      ballMoving = true;
      playStartSound();
    }

    return; // Skip updateBall and collision logic
  }

  // Ball is moving
  updateBall(&ball, &paddle);

  // Check if ball is lost
  if (ball.y > SCREEN_H) {
    eraseBall(&prev_ball); // Clear ball before resetting

    playLoseLifeSound(); // Sound of Losing a life
    lives--;

    if (lives <= 0) {
      currentBreakoutState = BREAKOUT_GAMEOVER;
    } else {
      resetBall();      // Place ball back on paddle
      prev_ball = ball; // Prevent redraw artifacts
    }
    return;
  }

  // === Brick collisions ===
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &b = bricks[row][col];
      if (!b.active)
        continue;

      if (ball.x + ball.w > b.x && ball.x < b.x + BRICK_WIDTH &&
          ball.y + ball.h > b.y && ball.y < b.y + BRICK_HEIGHT) {

        b.active = false;
        tft.fillRect(b.x + BRICK_SPACING_X / 2, b.y + BRICK_SPACING_Y / 2,
                     BRICK_WIDTH - BRICK_SPACING_X,
                     BRICK_HEIGHT - BRICK_SPACING_Y, TFT_BLACK);
        score += 10;

        playBreakSound();

        // Bounce logic
        int overlapLeft = (ball.x + ball.w) - b.x;
        int overlapRight = (b.x + BRICK_WIDTH) - ball.x;
        int overlapTop = (ball.y + ball.h) - b.y;
        int overlapBottom = (b.y + BRICK_HEIGHT) - ball.y;

        bool fromLeftRight =
            min(overlapLeft, overlapRight) < min(overlapTop, overlapBottom);

        if (fromLeftRight)
          ball.vx *= -1;
        else
          ball.vy *= -1;

        break; // only one brick per frame
      }
    }
  }

  // === Win check ===
  bool allCleared = true;
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      if (bricks[row][col].active) {
        allCleared = false;
        break;
      }
    }
    if (!allCleared)
      break;
  }

  if (allCleared)
    currentBreakoutState = BREAKOUT_WIN;
}

// ========== Update Ball Status ========== //
void updateBall(Ball *b, Paddle *paddle) {
  prev_ball = *b;

  // Apply motion
  b->x += b->vx;
  b->y += b->vy;

  // Wall collision //
  if(b->x <= 0){ // Left Wall
    b->x = 0;
    b->vx = abs(b->vx);
    playBounceSound();
  }else if(b->x + b->w >= SCREEN_W){ // Right Wall
    b->x = b->x - b->w;
    b->vx = -abs(b->vx);
    playBounceSound();
  }

  const int topMargin = 24; // Protect UI area
  if (b->y <= topMargin) {
    b->y = topMargin; // Prevent overshooting
    b->vy *= -1;
    playBounceSound();
  }

  // Paddle collision
  const int leftMargin = 1;
  const int rightMargin = 1;

  int px = paddle->x + leftMargin;
  int pw = paddle->w - leftMargin - rightMargin;

  if (b->vy > 0 && b->y + b->h >= paddle->y && b->y < paddle->y + paddle->h &&
      b->x + b->w > px && b->x < px + pw) {

    playBounceSound();

    float hitRatio = ((b->x + b->w / 2.0f) - (paddle->x + paddle->w / 2.0f)) /
                     (paddle->w / 2.0f);
    hitRatio =
        constrain(hitRatio, -0.9f, 0.9f); // Prevent flat horizontal bounces
    
    // Prevent center hits from going straight up/down
    if (abs(hitRatio) < 0.1f) {
      // Add small random offset to break center deadlock
      hitRatio = (random(0, 2) == 0) ? -0.15f : 0.15f;
    }

    b->vx = hitRatio * ballSpeed;
    b->vy = -sqrt(ballSpeed * ballSpeed - b->vx * b->vx);

    // Safety net: force minimum vy
    if (abs(b->vy) < 2.0f)
      b->vy = -2.0f;

    b->y = paddle->y - b->h - 1;
  }
}

// ========== Update Paddle Status ========== //
void updatePaddle(Paddle *p) {
  prev_paddle = *p;
  if (left.isPressed())
    p->x -= 10;
  if (right.isPressed())
    p->x += 10;
  if (p->x < 0)
    p->x = 0;
  if (p->x + p->w > SCREEN_W)
    p->x = SCREEN_W - p->w;
}

// ####################################################################################################
//  Game Drawing
// ####################################################################################################

// ========== Draw GameOver Selection Buttons ========== //
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

// ========== Draw GameOver Screen ========== //
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

// ========== Draw HomeScreen Selection Buttons ========== //
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

// ========== Draw HomeScreen Screen ========== //
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

// ========== Draw Game Frame (Ball and Paddle logic) ========== //
void drawBreakoutFrame() {

  drawPaddle(&paddle);
  eraseBall(&prev_ball);
  drawBall(&ball);

  if (paddle.x != prev_paddle.x || paddle.y != prev_paddle.y) {
    erasePaddle(&prev_paddle);
    drawPaddle(&paddle);
    prev_paddle = paddle;
  }

  if (lives != lastLives || score != lastScore) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2); // Same font size as before

    // Clear top bar (slightly taller to avoid overlap)
    tft.fillRect(0, 0, SCREEN_W, 32, TFT_BLACK);

    // New larger radius
    const int iconRadius = BALL_RADIUS + 2; // make ball bigger
    const int iconX = 20;
    const int iconY = 12; // lower a bit to center vertically

    // Draw larger ball
    tft.fillCircle(iconX + iconRadius, iconY + iconRadius, iconRadius,
                   TFT_WHITE);

    // Keep same distance from ball to text
    const int textX = iconX + iconRadius * 2 + 30;
    const int textY =
        iconY + iconRadius - 8 + 10; // aligns text vertically with circle
    tft.drawString("x " + String(lives), textX, textY);

    // Draw score
    tft.drawString("Score: " + String(score), SCREEN_W - 80, 20); // Top right

    lastLives = lives;
    lastScore = score;
  }
}

// ========== Draw Ball ========== //
void drawBall(const Ball *b) {
  tft.fillCircle(b->x + b->w / 2, b->y + b->h / 2, b->w / 2, TFT_WHITE);
}

// ========== Remove Ball ========== //
void eraseBall(const Ball *b) {
  if (b->y + b->h / 2 < 20)
    return;
  tft.fillCircle(b->x + b->w / 2, b->y + b->h / 2, b->w / 2, TFT_BLACK);
}

// ========== Draw Paddle ========== //
void drawPaddle(const Paddle *p) {
  tft.fillRect(p->x, p->y, p->w, p->h, TFT_WHITE);
}

// ========== Remove Paddle ========== //
void erasePaddle(const Paddle *p) {
  tft.fillRect(p->x, p->y, p->w, p->h, TFT_BLACK);
}

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Start Game Sound ========== //
void playStartSound() {
  playTone(1000, volume);
  delay(80);
  playTone(1400, volume);
  delay(80);
  playTone(0, 0); // stop sound
}

// ========== Bouncing Sound ========== //
void playBounceSound() {
  playTone(1200, volume); // Quick bounce blip
  delay(15);
  playTone(0, 0);
}

// ========== Life Lost Sound ========== //
void playLoseLifeSound() {
  playTone(200, volume); // Deep drop tone
  delay(30);
  playTone(0, 0);
}

// ========== Breaking Brick Sound ========== //
void playBreakSound() {
  playTone(1200, volume); // Higher pitch for break
  delay(30);
  playTone(0, 0);
}
