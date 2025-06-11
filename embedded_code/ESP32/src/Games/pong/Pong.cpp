#include "Pong.hpp"
#include "pong_logic.hpp"

#define orange_color TFT_ORANGE

enum GameState {
  STATE_HOMESCREEN,
  STATE_PLAYING,
  STATE_GAMEOVER,
  MULTIPLAYER_SELECTION,
  MULTIPLAYER_PLAYING,
  SINGLE_PLAYER,
  JOIN_SCREEN,
  BLUETOOTH_SCREEN,
  BLUETOOTH_NUMPAD,
  HOST_SCREEN,
};

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

// Badge
static int consecutiveWins = 0;

// Multiplayer
static bool firstFrame = true;
static bool multiplayerMode = false;
static int player_paddle = 1; // or 0, depending on who the human is

static char localPlayerSide = ' ';
static char localPlayerSymbol = ' ';

// Game assets - ball, and paddles
static Ball ball;
static Ball prev_ball;
static Paddle paddles[2];
static Paddle prev_paddles[2];

// Score and other state elements
static GameState current_state = STATE_HOMESCREEN;
static GameState prev_state = STATE_HOMESCREEN;
static bool game_initialized = false;
static bool first_home_draw = true;
static int level = 1;
static int score0 = 0;
static int score1 = 0;
static int pos_x = 0, pos_y = 0;
static int prev_pos_x = 0, prev_pos_y = 0;

// Frame rate control variables
static unsigned long previousMillis = 0;
static const int targetFPS = 60; // Set your desired frame rate
static const unsigned long frameTime =
    1000 / targetFPS; // Time per frame in milliseconds
static unsigned long currentFPS = 0;
static unsigned long fpsUpdateTime = 0;
static unsigned int frameCount = 0;
static unsigned long lastButtonPressTime = 0;

// button timing
static long moveDelay = 200;
static unsigned long lastMoveTime = 0;

static bool buttonAPressed = false;
static bool buttonBPressed = false;
static int prev_score0, prev_score1 = 0;

static const int SCREEN_HEIGHT = 320;
static const int SCREEN_WIDTH = 480;

// Selection
static int selection = 0;
static int subselection = 0;

// Functions

// Drawing functions
static void drawPaddle(Paddle paddle);
static void drawBall(Ball *ball);
static void drawScore(int score0, int score1);
static void erasePaddle(Paddle paddle);
static void eraseBall(Ball *ball);
static void draw_endscreen(int score0, int score1);
static void init_buttons();
static void erase_score();
static void drawHomeScreen();
static void drawHomeSelection();
static void updateFPS();
static void resetMultiplayerState(bool fullReset);

// Multiplayer Functions
void handleHostLogic();
void handlePeripheralLogic();
void drawGameState();
bool checkWinCondition();

// Numpad
static NumPad<GameState> pad(
    drawHomeScreen,                          // What to show when exiting pad
    []() { current_state = STATE_PLAYING; }, // What to do on confirm
    &current_state, STATE_HOMESCREEN,
    STATE_PLAYING // or another post-confirm state
);

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runPong() {

  resetExitFlag(); // Resets flag for Main Menu

  // Reset state
  current_state = STATE_HOMESCREEN;
  first_home_draw = true;
  game_initialized = false;
  score0 = score1 = 0;
  selection = 0;
  subselection = 0;
  buttonAPressed = false;
  buttonBPressed = false;
  firstFrame = true;

  // clear sprite and cache
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  // Debounce window to ignore early A presses
  unsigned long enterTime = millis();

  // Initialize random number generator
  randomSeed(esp_random());

  // Initialize Screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Initialize frame rate variables
  previousMillis = millis();
  fpsUpdateTime = previousMillis;

  drawHomeScreen();

  // Main pong loop
  while (true) {

    unsigned long now = millis();

    // Debounce: ignore input for first 300ms
    if (now - enterTime < 300) {
      updateAllButtons(); // Still update button states
      delay(10);
      continue;
    }

    handlePongFrame();

    if (getExitFlag()) {
      BluetoothManager::reset();
      return;
    }

    // Keep support for exiting with B from homescreen as well
    if (current_state == STATE_HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu");
      delay(500);
      BluetoothManager::reset();
      return;
    }
  }
}

// ========== Manual Loop ========== //
void handlePongFrame() {

  consecutiveWins = 0;

  static unsigned long lastMoveTime = 0;

  // Check if the Start Button was pressed and goes back to Main Menu
  if (checkStartButtonAndExit(tft))
    return;

  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillis;

  // Only update the game if enough time has passed for the next frame
  if (elapsedMillis >= frameTime) {
    // Record the time for this frame
    previousMillis = currentMillis;

    updateFPS();

    // FPS calculation (updates once per second)
    frameCount++;
    if (currentMillis - fpsUpdateTime >= 1000) {
      currentFPS = frameCount;
      frameCount = 0;
      fpsUpdateTime = currentMillis;
    }

    // ================== HOMESCREEN State =================== //
    if (current_state == STATE_HOMESCREEN) {
      if (millis() - lastMoveTime > moveDelay / 2) {
        if (A.wasJustPressed()) {

          if (selection == 0) {

            // SINGLE PLAYER
            resetMultiplayerState(true);
            multiplayerMode = false;
            player_paddle = 1; // Player on the right
            current_state = STATE_PLAYING;
            game_initialized = false;
            tft.fillScreen(TFT_BLACK);

          } else if (selection == 1) {

            // MULTIPLAYER MENU
            resetMultiplayerState(true);
            multiplayerMode = true;
            current_state = MULTIPLAYER_SELECTION;
            drawHomeSelection();
          }
        }

        // Selection logic
        if (up.wasJustPressed()) {
          selection = 0;
          drawHomeSelection();
        } else if (down.wasJustPressed()) {
          selection = 1;
          drawHomeSelection();
        }

        lastMoveTime = millis();
      }
    }

    // PLAYING STATE - MULTIPLAYER AND SINGLE-PLAYER
    else if (current_state == STATE_PLAYING) {

      // Initializes the game
      if (!game_initialized) {
        initialize_game(&ball, paddles, &level);

        // Initializes prev_ball
        prev_ball.x = ball.x;
        prev_ball.y = ball.y;
        prev_ball.h = ball.h;
        prev_ball.w = ball.w;

        // Initializes prev_paddles
        prev_paddles[0].y = paddles[0].y;
        prev_paddles[0].x = paddles[0].x;
        prev_paddles[1].y = paddles[1].y;
        prev_paddles[1].x = paddles[1].x;

        prev_paddles[0].w = paddles[0].w;
        prev_paddles[0].h = paddles[0].h;

        prev_paddles[1].w = paddles[1].w;
        prev_paddles[1].h = paddles[1].h;

        prev_paddles[0].paddle_mod = false;
        prev_paddles[1].paddle_mod = false;

        // Game initialized is now set to true
        // and screen background is filled
        game_initialized = true;
        tft.fillScreen(TFT_BLACK);
        prev_score0 = 0;
        prev_score1 = 0;
      }

      // Moves the paddle up
      if (up.isPressed()) {
        updatePaddle(true, &paddles[player_paddle]);
        // Will make sure the previous position of
        // the paddle is overwritten
        if (paddles[player_paddle].y != prev_paddles[player_paddle].y) {
          prev_paddles[player_paddle].paddle_mod = true;
        }
      } // Moves the paddle down
      else if (down.isPressed()) {
        updatePaddle(false, &paddles[player_paddle]);
        // Will make sure the previous position of
        // the paddle is overwritten
        if (paddles[player_paddle].y != prev_paddles[player_paddle].y) {
          prev_paddles[player_paddle].paddle_mod = true;
        }
      }

      // ============== SINGLE PLAYER ================= //
      // AI logic
      if (!multiplayerMode) {
        ai_paddle(&paddles[0], &ball, level); // AI controls left paddle
      }

      // Check if paddle position changed
      prev_ball.x = ball.x;
      prev_ball.y = ball.y;

      // Update ball position
      updateBall(&ball, paddles, &level, &score0, &score1);

      // Checks if the paddle positions have been modified
      if (paddles[0].y != prev_paddles[0].y) {
        prev_paddles[0].paddle_mod = true;
      }
      if (paddles[1].y != prev_paddles[1].y) {
        prev_paddles[1].paddle_mod = true;
      }

      // Checks if the game has been won by either player and
      // modified the gamestate accordingly, otherwise
      // draws all game assets
      if (score0 >= GAME_WON || score1 >= GAME_WON) {

        // ================= Badge Unlock Logic =================
        if (!multiplayerMode) {
          bool playerWon =
              score1 >= GAME_WON; // assuming player is on the right
          bool aiWon = score0 >= GAME_WON;

          if (playerWon && !aiWon) {
            consecutiveWins++;
          } else {
            consecutiveWins = 0;
          }

          if (consecutiveWins >= 1 && !badgeProgress[1]) {
            badgeProgress[1] = true;
            isUnlocked[1] = true;
            saveBadgeProgress();
            checkFinalBadgeUnlock();

            hasPendingNotification = true;
            pendingNotificationMessage = "Pong Badge Unlocked!";
            pendingNotificationDuration = 3000;
          }
        }

        // Changes the game state if either the players or AI has won
        current_state = STATE_GAMEOVER;
        firstFrame = true;
        tft.fillScreen(TFT_BLACK);
        draw_endscreen(score0, score1);
        return;
      } else {
        // Checks if the paddles have been moved/modified
        // and erases the old paddle if that is the case
        if (prev_paddles[0].paddle_mod) {
          erasePaddle(prev_paddles[0]);
          prev_paddles[0].paddle_mod = false;
          prev_paddles[0].y = paddles[0].y;
        }
        if (prev_paddles[1].paddle_mod) {
          erasePaddle(prev_paddles[1]);
          prev_paddles[1].paddle_mod = false;
          prev_paddles[1].y = paddles[1].y;
        }
        eraseBall(&prev_ball);

        // will erase the scores if either of them have changed
        if (prev_score0 != score0 || prev_score1 != score1) {
          erase_score();
          prev_score0 = score0;
          prev_score1 = score1;
        }

        // Draws all assets
        drawPaddle(paddles[0]);
        drawPaddle(paddles[1]);
        drawBall(&ball);
        drawScore(score0, score1);
      }
    }

    // ================== MULTIPLAYER_SELECTION State =================== //
    else if (current_state == MULTIPLAYER_SELECTION) {
      if (millis() - lastMoveTime > moveDelay) {

        if (A.wasJustPressed()) {
          if (subselection == 0) {
            // === HOST FLOW === //
            BluetoothManager::initCentral(tft);
            BluetoothCentral &central = BluetoothManager::getCentral();

            std::string code = generate6DigitCode();

            // Display the host connection code (create a simple PongHostScreen
            // if needed)
            HostGame::init(tft);
            HostGame::showCode(String(code.c_str()));

            if (getExitFlag()) {
              resetExitFlag();
              current_state = STATE_HOMESCREEN;
              return;
            }

            central.scanAndConnectLoop(code);

            multiplayerMode = true;

            if (!central.getConnectedClients().empty()) {
              initialize_game(&ball, paddles, &level);
              localPlayerSide = 'X'; // or 0/1 if you want paddle 0
              current_state = HOST_SCREEN;
              firstFrame = true;
              tft.fillScreen(TFT_BLACK); // or your orange_color
            } else {
              current_state = MULTIPLAYER_SELECTION;
              tft.fillScreen(TFT_BLACK);
              ConnectionScreen::showMessage("Connection failed.\nTry again.");
            }

            // Button debounce flush
            delay(300);
            while (A.isPressed() || up.isPressed() || down.isPressed() ||
                   left.isPressed() || right.isPressed()) {
              delay(10);
            }

          } else {
            // === JOIN FLOW === //
            current_state = BLUETOOTH_NUMPAD;
            pad.numPadSetup();
          }
        }

        // Directional navigation
        if (up.wasJustPressed()) {
          current_state = STATE_HOMESCREEN;
          drawHomeSelection();
        } else if (left.wasJustPressed() && subselection == 1) {
          subselection = 0;
          drawHomeSelection();
        } else if (right.wasJustPressed() && subselection == 0) {
          subselection = 1;
          drawHomeSelection();
        }

        lastMoveTime = millis();
      }
    }

    else if (current_state == MULTIPLAYER_PLAYING ||
             current_state == HOST_SCREEN) {
      if (firstFrame) {
        // Initializes the game
        initialize_game(&ball, paddles, &level);
        // Initializes prev_ball
        prev_ball.x = ball.x;
        prev_ball.y = ball.y;
        prev_ball.h = ball.h;
        prev_ball.w = ball.w;

        // Initializes prev_paddles
        prev_paddles[0].y = paddles[0].y;
        prev_paddles[0].x = paddles[0].x;
        prev_paddles[1].y = paddles[1].y;
        prev_paddles[1].x = paddles[1].x;

        prev_paddles[0].w = paddles[0].w;
        prev_paddles[0].h = paddles[0].h;

        prev_paddles[1].w = paddles[1].w;
        prev_paddles[1].h = paddles[1].h;

        prev_paddles[0].paddle_mod = false;
        prev_paddles[1].paddle_mod = false;

        tft.fillScreen(TFT_BLACK);
        prev_score0 = 0;
        prev_score1 = 0;
        firstFrame = false;
      }

      if (current_state == HOST_SCREEN) {
        handleHostLogic();
      } else {
        handlePeripheralLogic();
      }

      if (checkWinCondition()) {
        current_state = STATE_GAMEOVER;
        prev_state = current_state;
        tft.fillScreen(TFT_BLACK);
        draw_endscreen(score0, score1);
        firstFrame = true;
        return;
      }

      drawGameState();
    }

    // ================== BLUETOOTH_NUMPAD State =================== //
    else if (current_state == BLUETOOTH_NUMPAD) {
      pad.handleButtonInput(&lastMoveTime, moveDelay);

      std::string enteredCode = pad.getCode();
      if (enteredCode.length() == 6 && pad.wasEnterPressed()) {
        current_state = MULTIPLAYER_PLAYING;

        // JOIN = PERIPHERAL
        BluetoothManager::initPeripheral(tft);
        BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
        peripheral.beginAdvertising(enteredCode);
        localPlayerSymbol = 'O';

        initialize_game(&ball, paddles, &level);

        pad.clearCode();
      }
    } else if (current_state == STATE_GAMEOVER) {
      if (A.wasJustPressed()) {
        // current_state = prev_state; // Need some other checks before letting
        // users restart probably need to send a "ready" string, etc.
      } else if (B.wasJustPressed()) {
        current_state = STATE_HOMESCREEN;
        first_home_draw = true;
        drawHomeScreen();
      }
    }
  }
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Update Frames Per Second ========== //
void updateFPS() {
  frameCount++;
  if (millis() - fpsUpdateTime >= 1000) {
    currentFPS = frameCount;
    frameCount = 0;
    fpsUpdateTime = millis();
  }
}

// ========== Reset Multiplayer Values ========== //
void resetMultiplayerState(bool fullReset) {
  multiplayerMode = false;
  localPlayerSide = ' '; // or 0/1 if using int instead of char
  player_paddle = 1;     // Default paddle side for single-player
  // if (fullReset) {
  //   // BluetoothManager::end(); // Optional if you're cleaning up connections
  // }
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// Function to draw the paddle on the screen
static void
drawPaddle(Paddle paddle) { // Changed from LGFX_Sprite to TFT_eSprite
  // Draw new paddle position
  tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_WHITE);
}

// Function to draw the ball on the screen
static void drawBall(Ball *ball) { // Changed from LGFX_Sprite to TFT_eSprite
  // Draw new ball position
  tft.fillCircle(ball->x + (ball->w / 2), ball->y + (ball->w / 2), ball->w / 2,
                 TFT_WHITE);
}

// Function to erase the paddle on the screen
static void
erasePaddle(Paddle paddle) { // Changed from LGFX_Sprite to TFT_eSprite
  // Draw new paddle position
  tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_BLACK);
}

// Function to erase the ball on the screen
static void eraseBall(Ball *ball) { // Changed from LGFX_Sprite to TFT_eSprite
  // Draw new ball position
  tft.fillCircle(ball->x + (ball->w / 2), ball->y + (ball->w / 2), ball->w / 2,
                 TFT_BLACK);
}

// Function to draw the score on the screen
static void drawScore(int score0,
                      int score1) { // Changed from LGFX_Sprite to TFT_eSprite
  tft.setTextSize(2);

  tft.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, TFT_WHITE);

  // Create score strings
  String scoreText1 = "P1: " + String(score0);
  String scoreText2 = "P2: " + String(score1);

  // Get text widths
  int width1 = tft.textWidth(scoreText1);
  int width2 = tft.textWidth(scoreText2);

  // Calculate positions so each string is centered in its half
  int leftX = (SCREEN_WIDTH / 2 - width1) / 2;
  int rightX = SCREEN_WIDTH / 2 + (SCREEN_WIDTH / 2 - width2) / 2;

  // Draw the text
  tft.setCursor(leftX, 5);
  tft.print(scoreText1);

  tft.setCursor(rightX, 5);
  tft.print(scoreText2);
}

static void erase_score() {
  tft.setTextSize(2); // Make sure the same text size is set
  int textHeight = tft.fontHeight();
  int y = 5; // Same Y position you use for scores

  // Clear left half where P1 score is
  tft.fillRect(0, y, SCREEN_WIDTH / 2, textHeight, TFT_BLACK);

  // Clear right half where P2 score is
  tft.fillRect(SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 2, textHeight, TFT_BLACK);
}

static void drawHomeScreen() {
  // Clears homescreen and redraws it
  if (first_home_draw) {
    tft.fillScreen(TFT_BLACK);
    first_home_draw = false;

    // Draw big "PONG" title
    String title = "PONG";
    tft.setTextSize(8);
    // Main title with depth effect
    int titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2;
    int titleY = 30;
    tft.setTextColor(TFT_WHITE); // Main text color
    tft.setCursor(titleX, titleY);
    tft.print(title);

    // Highlights (simulate lighting)
    titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2 - 2;
    titleY = 30 - 2;
    tft.setTextColor(TFT_LIGHTGREY); // Lighter color for highlight
    tft.setCursor(titleX, titleY);
    tft.print(title);

    // Options
    tft.setTextSize(2);
    tft.drawString("Start Single-Player", SCREEN_WIDTH / 2, 180);
    tft.drawString("Start Multiplayer", SCREEN_WIDTH / 2, 230);

    drawHomeSelection();
  }
}

static void drawHomeSelection() {
  int y_single = 180;
  int y_multi = 230;
  int y_sub = y_multi + 40;

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, SCREEN_WIDTH, 35, TFT_BLACK);
  tft.fillRect(0, y_multi - 15, SCREEN_WIDTH, 80, TFT_BLACK);

  if (selection == 0) {
    tft.setTextSize(3);
    tft.drawString("Start Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Start Multiplayer", SCREEN_WIDTH / 2, y_multi);
  } else {
    tft.setTextSize(2);
    tft.drawString("Start Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Start Multiplayer", SCREEN_WIDTH / 2, y_multi);

    if (current_state == MULTIPLAYER_SELECTION) {
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

      int x_sub1 = SCREEN_WIDTH / 4;
      int x_sub2 = 3 * SCREEN_WIDTH / 4;

      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}

// Draws the end screen
static void draw_endscreen(int score0, int score1) {

  bool playerWon = score1 >= GAME_WON; // Assuming player is on the right
  bool aiWon = score0 >= GAME_WON;

  // Clear screen
  tft.fillScreen(TFT_BLACK);

  // Title (Winner message) with depth effect
  String player = score0 >= GAME_WON ? "Player 1 Wins!" : "Player 2 Wins!";
  tft.setTextSize(5); // Larger size for the winner message
  int titleX = (SCREEN_WIDTH - tft.textWidth(player)) / 2;
  int titleY = 30;

  // Shadow effect
  tft.setTextColor(TFT_DARKGREY); // Shadow color
  tft.setCursor(titleX + 4, titleY + 4);
  tft.print(player);

  // Main title
  tft.setTextColor(TFT_WHITE); // Main text color
  tft.setCursor(titleX, titleY);
  tft.print(player);

  // Score display with highlights and shadows
  tft.setTextSize(3);
  String scorestr0 = "P1: " + String(score0) + "  ";
  String scorestr1 = "P2: " + String(score1);
  int scoreX =
      (SCREEN_WIDTH - tft.textWidth(scorestr0) - tft.textWidth(scorestr1)) / 2;
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(scoreX + 4, 120 + 4); // Slight shadow offset
  tft.print(scorestr0);
  tft.print(scorestr1);

  tft.setTextColor(TFT_WHITE); // Reset main text color
  tft.setCursor(scoreX, 120);  // Main position
  tft.print(scorestr0);
  tft.print(scorestr1);

  // Instructions for restart and home screen with buttons

  // ----- Restart Button (A) -----
  int pressA_y = 200;
  int pressA_x = (SCREEN_WIDTH / 2) - 110;

  // "Press" text
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(pressA_x, pressA_y);
  tft.print("Press");

  // A Button (circle)
  int a_circle_x = SCREEN_WIDTH / 2;
  int a_circle_y = pressA_y + 15;
  uint16_t LIGHTER_RED = tft.color565(255, 120, 120); // Brighter red
  uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);  // Darker shadow

  // Shadow effect for button
  tft.fillCircle(a_circle_x + 4, a_circle_y + 4, 22, DARKER_SHADOW);
  // Red button
  tft.fillCircle(a_circle_x, a_circle_y, 22, TFT_RED);
  // Light red highlight
  tft.fillEllipse(a_circle_x - 7, a_circle_y - 7, 7, 5, LIGHTER_RED);

  // "A" letter inside the button
  tft.setTextSize(2);
  tft.setCursor(a_circle_x - 6, a_circle_y - 8);
  tft.setTextColor(TFT_BLACK);
  tft.print("A");

  // Restart instruction text
  String restartText = "to restart";
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  int restartText_x = (SCREEN_WIDTH / 2) + 40;
  tft.setCursor(restartText_x, pressA_y);
  tft.print(restartText);

  // ----- Home Button (B) -----
  int pressB_y = 260;
  int pressB_x = (SCREEN_WIDTH / 2) - 110;

  // "Press" text
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(pressB_x - 40, pressB_y); // Left-shifted
  tft.print("Press");

  // B Button (circle)
  int b_circle_x = SCREEN_WIDTH / 2 - 45;
  int b_circle_y = pressB_y + 15;
  uint16_t LIGHTER_BLUE = tft.color565(120, 120, 255); // Brighter blue

  // Shadow effect for button
  tft.fillCircle(b_circle_x + 4, b_circle_y + 4, 22, DARKER_SHADOW);
  // Blue button
  tft.fillCircle(b_circle_x, b_circle_y, 22, TFT_BLUE);
  // Light blue highlight
  tft.fillEllipse(b_circle_x - 7, b_circle_y - 7, 7, 5, LIGHTER_BLUE);

  // "B" letter inside the button
  tft.setTextSize(2);
  tft.setCursor(b_circle_x - 6, b_circle_y - 8);
  tft.setTextColor(TFT_BLACK);
  tft.print("B");

  // Home instruction text
  String homeText = "for home screen";
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  int homeText_x = pressB_x - 40 + 150; // Adjusted position for left shift
  tft.setCursor(homeText_x, pressB_y);
  tft.print(homeText);
}

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ####################################################################################################
//  Bluetooth Logic
// ####################################################################################################

void handleHostLogic() {
  BluetoothCentral &central = BluetoothManager::getCentral();

  std::string msg = central.readMessage();
  if (!msg.empty() && msg.rfind("@pong@move@", 0) == 0) {
    std::string yStr = msg.substr(11);
    int yPos = std::stoi(yStr);
    paddles[1].y = yPos;
    prev_paddles[1].paddle_mod = true;
  }

  if (up.isPressed()) {
    updatePaddle(true, &paddles[0]);
    prev_paddles[0].paddle_mod = true;
  } else if (down.isPressed()) {
    updatePaddle(false, &paddles[0]);
    prev_paddles[0].paddle_mod = true;
  }

  prev_ball.x = ball.x;
  prev_ball.y = ball.y;
  updateBall(&ball, paddles, &level, &score0, &score1);

  // Checks if the paddle positions have been modified
  if (paddles[0].y != prev_paddles[0].y) {
    prev_paddles[0].paddle_mod = true;
  }
  if (paddles[1].y != prev_paddles[1].y) {
    prev_paddles[1].paddle_mod = true;
  }

  char state[64];
  sprintf(state, "@pong@state@%d,%d,%d,%.1f,%.1f", paddles[0].y, score0, score1,
          ball.x, ball.y);
  central.sendMessage(state);
}

void handlePeripheralLogic() {
  BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();

  // read state
  std::string state = peripheral.readMessage();

  // update paddle
  if (up.isPressed()) {
    updatePaddle(true, &paddles[1]);
    prev_paddles[1].paddle_mod = true;

    char buf[32];
    sprintf(buf, "@pong@move@%d", paddles[1].y);
    peripheral.sendMessage(buf);
  } else if (down.isPressed()) {
    updatePaddle(false, &paddles[1]);
    prev_paddles[1].paddle_mod = true;

    char buf[32];
    sprintf(buf, "@pong@move@%d", paddles[1].y);
    peripheral.sendMessage(buf);
  }

  if (!state.empty() && state.rfind("@pong@state@", 0) == 0) {
    int y0, s0, s1;
    float bx, by;
    sscanf(state.c_str() + 13, "%d,%d,%d,%f,%f", &y0, &s0, &s1, &bx, &by);

    prev_ball.x = ball.x;
    prev_ball.y = ball.y;

    if (score0 != s0 || score1 != s1) {
      initialize_game(&ball, paddles, &level);
    }

    paddles[0].y = y0;
    score0 = s0;
    score1 = s1;
    ball.x = bx;
    ball.y = by;

    prev_paddles[0].paddle_mod = true;
    prev_paddles[1].paddle_mod = true;
  }
}

void drawGameState() {
  if (prev_paddles[0].paddle_mod) {
    erasePaddle(prev_paddles[0]);
    prev_paddles[0].paddle_mod = false;
    prev_paddles[0].y = paddles[0].y;
  }

  if (prev_paddles[1].paddle_mod) {
    erasePaddle(prev_paddles[1]);
    prev_paddles[1].paddle_mod = false;
    prev_paddles[1].y = paddles[1].y;
  }

  eraseBall(&prev_ball);

  if (prev_score0 != score0 || prev_score1 != score1) {
    erase_score();
    prev_score0 = score0;
    prev_score1 = score1;
  }

  drawPaddle(paddles[0]);
  drawPaddle(paddles[1]);
  drawBall(&ball);
  drawScore(score0, score1);
}

bool checkWinCondition() { return score0 >= GAME_WON || score1 >= GAME_WON; }
