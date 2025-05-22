#include <BLEDevice.h>
#include <SD.h>
#include <TFT_eSPI.h>  // Changed from LGFX to TFT_eSPI
#include "pong_logic.hpp"
#include "Pong.hpp"

typedef struct {
int y;
bool paddle_mod;
} Prev_Paddle;

// Game assets - ball, and paddles
static Ball ball;
static Ball prev_ball;
static Paddle paddles[2]; 
static Paddle prev_paddles[2];

// Score and other state elements
static GameState current_state = STATE_HOMESCREEN;
static bool game_initialized = false;
static bool first_home_draw = true;
static int level = 1;
static int score0 = 0;
static int score1 = 0;
static int pos_x = 0, pos_y = 0;
static int prev_pos_x = 0, prev_pos_y = 0;

// Frame rate control variables
static unsigned long previousMillis = 0;
static const int targetFPS = 60;  // Set your desired frame rate
static const unsigned long frameTime = 1000 / targetFPS;  // Time per frame in milliseconds
static unsigned long currentFPS = 0;
static unsigned long fpsUpdateTime = 0;
static unsigned int frameCount = 0;
static unsigned long lastButtonPressTime = 0;

//button timing
static long moveDelay = 200;
static unsigned long lastMoveTime = 0;


static bool buttonAPressed = false;
static bool buttonBPressed = false;
static int player_paddle = 1;
static int prev_score0, prev_score1 = 0;


static const int SCREEN_HEIGHT = 320;
static const int SCREEN_WIDTH = 480;

//Selection
static int selection = 0;
static int subselection = 0;


// Functions 
static void pongLoop();
static void handle_A_B_input();

//Drawing functions
static void drawPaddle(Paddle paddle);
static void drawBall(Ball* ball);
static void drawScore(int score0, int score1);
static void erasePaddle(Paddle paddle);
static void eraseBall(Ball* ball);
static void draw_endscreen(int score0, int score1);
static void init_buttons();
static void erase_score();
static void drawHomeScreen();
static void drawHomeSelection();


//Numpad
static NumPad<GameState> pad(
    drawHomeScreen, // What to show when exiting pad
    []() { current_state = STATE_PLAYING; }, // What to do on confirm
    &current_state, STATE_HOMESCREEN,
    STATE_PLAYING // or another post-confirm state
);


// Initializes pong
void runPong() {

  // Initialize random number generator
  randomSeed(esp_random());

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Initialize frame rate variables
  previousMillis = millis();
  fpsUpdateTime = previousMillis;
  
  //Main pong loop
  while(1){
    pongLoop();
  }
}

// Main game logic / game loop
static void pongLoop() {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillis;
  
  // Only update the game if enough time has passed for the next frame
  if (elapsedMillis >= frameTime) {
    // Record the time for this frame
    previousMillis = currentMillis;
    
    // FPS calculation (updates once per second)
    frameCount++;
    if (currentMillis - fpsUpdateTime >= 1000) {
      currentFPS = frameCount;
      frameCount = 0;
      fpsUpdateTime = currentMillis;
    }

    // ================= A and B input handling ============= //
    handle_A_B_input();


    // ============= START OF THE GAME STATES ================= //

    //GAME MENU STATE
    if (current_state == STATE_HOMESCREEN) {
      drawHomeScreen();
      if (up.wasJustPressed() && selection == 1) {
        selection = 0;
        drawHomeSelection();
      } else if (down.wasJustPressed() && selection == 0) {
        selection = 1;
        drawHomeSelection();
      } 
    }
    else if(current_state == MULTIPLAYER_SELECTION){
      if (left.wasJustPressed() && subselection == 1) {
        subselection = 0;
        drawHomeSelection();
      } else if (right.wasJustPressed() && subselection == 0) {
        subselection = 1;
        drawHomeSelection();
      }
      else if(up.wasJustPressed()){
        subselection = 0;
        selection = 1;
        current_state = STATE_HOMESCREEN;
        drawHomeSelection();
      }
    }
    // Handles the Numpad
    else if(current_state == BLUETOOTH_NUMPAD){
      pad.handleButtonInput(&lastMoveTime, moveDelay);
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
          if(paddles[player_paddle].y != prev_paddles[player_paddle].y){
            prev_paddles[player_paddle].paddle_mod = true;
          }
      } // Moves the paddle down
      else if (down.isPressed()) {
          updatePaddle(false, &paddles[player_paddle]);
          // Will make sure the previous position of 
          // the paddle is overwritten
          if(paddles[player_paddle].y != prev_paddles[player_paddle].y){
            prev_paddles[player_paddle].paddle_mod = true;
          }
      }

      // ============== SINGLE PLAYER ================= // 
      // Single player mode
      // AI paddle logic
      ai_paddle(&paddles[0], &ball, level);

      //Check if paddle position changed
      prev_ball.x = ball.x;
      prev_ball.y = ball.y;

      // Update ball position
      updateBall(&ball, paddles, &level, &score0, &score1);

      //Checks if the paddle positions have been modified
      if(paddles[0].y != prev_paddles[0].y){
        prev_paddles[0].paddle_mod = true;
      }
      if(paddles[1].y != prev_paddles[1].y){
        prev_paddles[1].paddle_mod = true;
      }

      // Checks if the game has been won by either player and
      // modified the gamestate accordingly, otherwise
      // draws all game assets
      if(score0 >= GAME_WON || score1 >= GAME_WON) {
          // Changes the game state if either the players or AI has won
          current_state = STATE_GAMEOVER;
          tft.fillScreen(TFT_BLACK);
          draw_endscreen(score0, score1);
      } else {
        // Checks if the paddles have been moved/modified 
        // and erases the old paddle if that is the case
        if(prev_paddles[0].paddle_mod){
          erasePaddle(prev_paddles[0]);
          prev_paddles[0].paddle_mod = false;
          prev_paddles[0].y = paddles[0].y;
        }
        if(prev_paddles[1].paddle_mod){
          erasePaddle(prev_paddles[1]);
          prev_paddles[1].paddle_mod = false;
          prev_paddles[1].y = paddles[1].y;
        }
        eraseBall(&prev_ball);

        //will erase the scores if either of them have changed
        if(prev_score0 != score0 || prev_score1 != score1){
          erase_score();
          prev_score0 = score0;
          prev_score1 = score1;
        }

        //Draws all assets
        drawPaddle(paddles[0]);
        drawPaddle(paddles[1]);
        drawBall(&ball);
        drawScore(score0, score1);
      }
    }
  }
}

// Handles user input to the A and B buttons
static void handle_A_B_input(){
  
  // ================== BUTTON B ===================== //

  // Check if any button is pressed for transitions
    // Button B 
    // - Starts single-player pong
    // - Resets to homescreen when game has ended
    if (B.wasJustPressed()) {
      if(!buttonBPressed){
        if (current_state == STATE_HOMESCREEN) {
          // MAYBE GOES BACK TO MAIN MENU
        }
        // Resets to homescreen after game has ended
        else if(current_state == STATE_GAMEOVER){
          current_state = STATE_HOMESCREEN;
          first_home_draw = true;  
          score0 = score1 = 0;
          game_initialized = false;
        }
      }
      buttonBPressed = true;
    }else{
      buttonBPressed = false;
    }


    // ================== BUTTON A ===================== //

    // Button A 
    // - Resets game after game has ended
    if (A.wasJustPressed()) {
      // If button A was not previously pressed, transition to the next state
      if (!buttonAPressed) {
        // Starts pong game
        if (current_state == STATE_HOMESCREEN) {
          if (selection == 1) {
            current_state = MULTIPLAYER_SELECTION;
            drawHomeSelection();
          }else{
            current_state = STATE_PLAYING;
          }
        }
        else if(current_state == STATE_GAMEOVER){
          current_state = STATE_PLAYING; 
          score0 = score1 = 0;
          game_initialized = false;
        }
        else if(current_state == MULTIPLAYER_SELECTION){
          if(subselection == 0){
            current_state = JOIN_SCREEN;
            tft.fillScreen(TFT_BLUE);
          }else{
            pad.numPadSetup();
            first_home_draw = true;
            current_state = BLUETOOTH_NUMPAD;
          }
        }
      }
      // Set buttonAPressed flag to true to indicate button is pressed
      buttonAPressed = true;
    } else {
      // Reset buttonAPressed when button A is released
      buttonAPressed = false;
    }
}

// ======================== DRAWING FUNCTIONS ======================

// Function to draw the paddle on the screen
static void drawPaddle(Paddle paddle) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new paddle position
    tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_WHITE);
}

// Function to draw the ball on the screen
static void drawBall(Ball* ball) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new ball position
    tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_WHITE);
}

// Function to erase the paddle on the screen
static void erasePaddle(Paddle paddle) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new paddle position
    tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_BLACK);
}

// Function to erase the ball on the screen
static void eraseBall(Ball* ball) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new ball position
    tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_BLACK);
}

// Function to draw the score on the screen
static void drawScore(int score0, int score1) {  // Changed from LGFX_Sprite to TFT_eSprite
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
  //Clears homescreen and redraws it
  if(first_home_draw){
    tft.fillScreen(TFT_BLACK);
    first_home_draw = false;

    // Draw big "PONG" title
    String title = "PONG";
    tft.setTextSize(8);
    // Main title with depth effect
    int titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2;
    int titleY = 30;
    tft.setTextColor(TFT_WHITE);  // Main text color
    tft.setCursor(titleX, titleY);
    tft.print(title);

    // Highlights (simulate lighting)
    titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2 - 2;
    titleY = 30 - 2;
    tft.setTextColor(TFT_LIGHTGREY);  // Lighter color for highlight
    tft.setCursor(titleX, titleY);
    tft.print(title);

    // Options
    tft.setTextSize(2);
    tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, 180);
    tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, 230);

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
    tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);
  } else {
    tft.setTextSize(2);
    tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);

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



//Draws the end screen
static void draw_endscreen(int score0, int score1) {
    // Clear screen
    tft.fillScreen(TFT_BLACK);

    // Title (Winner message) with depth effect
    String player = score0 >= GAME_WON ? "Player 1 Wins!" : "Player 2 Wins!";
    tft.setTextSize(5);  // Larger size for the winner message
    int titleX = (SCREEN_WIDTH - tft.textWidth(player)) / 2;
    int titleY = 30;

    // Shadow effect
    tft.setTextColor(TFT_DARKGREY);  // Shadow color
    tft.setCursor(titleX + 4, titleY + 4);
    tft.print(player);

    // Main title
    tft.setTextColor(TFT_WHITE);  // Main text color
    tft.setCursor(titleX, titleY);
    tft.print(player);

    // Score display with highlights and shadows
    tft.setTextSize(3);
    String scorestr0 = "P1: " + String(score0) + "  ";
    String scorestr1 = "P2: " + String(score1);
    int scoreX = (SCREEN_WIDTH - tft.textWidth(scorestr0) - tft.textWidth(scorestr1)) / 2;
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(scoreX + 4, 120 + 4);  // Slight shadow offset
    tft.print(scorestr0);
    tft.print(scorestr1);

    tft.setTextColor(TFT_WHITE);  // Reset main text color
    tft.setCursor(scoreX, 120);   // Main position
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
    tft.setCursor(pressB_x - 40, pressB_y);  // Left-shifted
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