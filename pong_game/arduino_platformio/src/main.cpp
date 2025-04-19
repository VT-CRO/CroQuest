#include "pong.h"
#include "main.hpp"
#include "draw.hpp"

//Display
static LGFX tft;

//Game assets - ball, and paddles
static Ball ball;
static Ball prev_ball;
static Paddle paddles[2];
static Prev_Paddle prev_paddles[2];

//Score and other state elements
static GameState current_state = STATE_HOME;
static bool game_initialized = false;
static bool first_home_draw = true;
static int level = 1;
static int score0 = 0;
static int score1 = 0;

// Frame rate control variables
static unsigned long previousMillis = 0;
static const int targetFPS = 60;  // Set your desired frame rate
static const unsigned long frameTime = 1000 / targetFPS;  // Time per frame in milliseconds
static unsigned long currentFPS = 0;
static unsigned long fpsUpdateTime = 0;
static unsigned int frameCount = 0;


void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Initialize TFT screen
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Initialize button pins
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);

  //Initialize sd pin
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  digitalWrite(TFT_CS, HIGH);

  SD.end();
  delay(2000);
  // Attempt to initialize the SD card
  if (!SD.begin(SD_CS, SPI, 10000000)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully.");
  
  // Initialize frame rate variables
  previousMillis = millis();
  fpsUpdateTime = previousMillis;
}

void loop() {
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

    // Check if any button is pressed for transitions
    // Button B 
    // - Starts single-player pong
    // - Resets to homescreen when game has ended
    if (digitalRead(B) == LOW) {
      //Starts pong game
      if (current_state == STATE_HOME || current_state == STATE_BLUETOOTH_HOST) {
          current_state = STATE_PLAYING;
          first_home_draw = true;
      }
      //Resets to homescreen after game has ended
      else if(current_state == STATE_GAMEOVER){
        current_state = STATE_HOME;
        first_home_draw = true;  
      }
    }
    //Button A 
    // - Resets game after game has ended
    if (digitalRead(A) == LOW) {
      if (current_state == STATE_GAMEOVER) {
          current_state = STATE_PLAYING;
          score0 = score1 = 0;
          game_initialized = false;
      }
    }

    if (current_state == STATE_HOME) {
      draw_homescreen(tft, &first_home_draw);
    }
    else if (current_state == STATE_PLAYING) {
      //Initializes the game
      if (!game_initialized) {
          initialize_game(&ball, paddles, &level);

          //Initializes prev_ball
          prev_ball.x = ball.x;
          prev_ball.y = ball.y;
          prev_ball.h = ball.h;
          prev_ball.w = ball.w;

          //Initializes prev_paddles
          prev_paddles[0].y = paddles[0].y;
          prev_paddles[1].y = paddles[1].y;
          prev_paddles[0].paddle_mod = false;
          prev_paddles[1].paddle_mod = false;

          //Game initialized is now set to true
          //and screen background is filled
          game_initialized = true;
          tft.fillScreen(TFT_BLACK);
      }

      //Moves the paddle up
      if (digitalRead(UP) == LOW) {
          updatePaddle(true, &paddles[1]);
          // Will make sure the previous position of 
          // the paddle is overwritten
          if(paddles[1].y != prev_paddles[1].y){
            prev_paddles[1].paddle_mod = true;
          }
      } // Moves the paddle down
      else if (digitalRead(DOWN) == LOW) {
          updatePaddle(false, &paddles[1]);
          // Will make sure the previous position of 
          // the paddle is overwritten
          if(paddles[1].y != prev_paddles[1].y){
            prev_paddles[1].paddle_mod = true;
          }
      }

      //Ai paddle logic
      ai_paddle(&paddles[0], &ball, level);
      if(paddles[0].y != prev_paddles[0].y){
        prev_paddles[0].paddle_mod = true;
      }

      //Updates the position of the ball
      updateBall(&ball, paddles, &level, &score0, &score1);

      //Checks if the game has been won by either player and
      // modified the gamestate accordingly, otherwise
      // draws all game assets
      if(score0 >= GAME_WON || score1 >= GAME_WON)
      {
          current_state = STATE_GAMEOVER;
          tft.fillScreen(TFT_BLACK);
      }else{
        drawPaddle(tft, paddles[0], &prev_paddles[0]);
        drawPaddle(tft, paddles[1], &prev_paddles[1]);
        drawBall(tft, &ball, &prev_ball);
        drawScore(tft, score0, score1);
      }
    }
    //Draws the endscreen/gameover screen
    else if (current_state == STATE_GAMEOVER) {
      draw_endscreen(tft, score0, score1);
    }
  }
}