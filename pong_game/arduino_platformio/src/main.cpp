#include <BLEDevice.h>
#include <SD.h>
#include <TFT_eSPI.h>  // Changed from LGFX to TFT_eSPI
#include "pong.h"
#include "main.hpp"
#include "draw.hpp"
#include "bluetooth.hpp"

// Display
static TFT_eSPI tft;  // Changed from LGFX to TFT_eSPI

// Game assets - ball, and paddles
static Ball ball;
static Ball prev_ball;
static Paddle paddles[2]; 
static Paddle prev_paddles[2];

// Score and other state elements
static GameState current_state = STATE_HOME;
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
static const unsigned long debounceDelay = 150;
static unsigned long lastButtonPressTime = 0;

static bool buttonAPressed = false;
static bool buttonBPressed = false;
static String code;
static bool hosting = false;
static int player_paddle = 1;
static int waiting_select = 1;
static int multiplayer_select = 1;
static int client_select = 1;
static int prev_score0, prev_score1 = 0;

// Callback functions for Bluetooth data
void onPaddleUpdate(int player, int y) {
  // Update the appropriate paddle
  if (player >= 0 && player < 2) {
    paddles[player].y = y;
    // Make sure y is within the required range
    if (paddles[player].y < 0) {
      paddles[player].y = 0;
    } else if (paddles[player].y > SCREEN_HEIGHT - PADDLE_HEIGHT) {
      paddles[player].y = SCREEN_HEIGHT - PADDLE_HEIGHT;
    }
    // Mark that the paddle has been modified
    prev_paddles[player].paddle_mod = true;
  }
}

void onBallUpdate(double x, double y, double speedX, double speedY) {
  // Update the ball position and speed
  prev_ball.x = ball.x;
  prev_ball.y = ball.y;
  
  ball.x = x;
  ball.y = y;
  ball.dx = speedX;
  ball.dy = speedY;
}

void onScoreUpdate(int player) {
  // Update the score for the appropriate player
  if (player == 1) {
    score0++;
  } else if (player == 2) {
    score1++;
  }
}

static void A_button_send_ready();
static void A_button_client_connection_input();

void setup() {
  Serial.begin(115200);

  // Initialize random number generator
  randomSeed(esp_random());

  // Initialize Bluetooth
  initializeBluetooth();
  
  // Register callback functions for receiving data
  registerPaddleUpdateCallback(onPaddleUpdate);
  registerBallUpdateCallback(onBallUpdate);
  registerScoreUpdateCallback(onScoreUpdate);
  
  // Each device initially setup as a host
  setupHost();

  // Initialize TFT screen
  tft.begin();  // Changed from init() to begin()
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
      if(!buttonBPressed){
        // Starts pong game
        if (current_state == STATE_HOME) {
            current_state = STATE_PLAYING;
            first_home_draw = true;
        }
        // Resets to homescreen after game has ended
        else if(current_state == STATE_GAMEOVER){
          current_state = STATE_HOME;
          first_home_draw = true;  
          score0 = score1 = 0;
          resetReadyStatus();
          disableMultiplayer();
          game_initialized = false;
        }
        else if(current_state == STATE_MULTIPLAYER){
          current_state = STATE_BLUETOOTH_CLIENT;
          pos_x = 0, pos_y = 0;
          prev_pos_x = 0, prev_pos_y = 0;
          tft.fillScreen(TFT_BLACK);
          init_buttons();
          draw_all_buttons(tft);
        }
      }
      buttonBPressed = true;
    }else{
      buttonBPressed = false;
    }
    // Button A 
    // - Resets game after game has ended
    if (digitalRead(A) == LOW) {
      // If button A was not previously pressed, transition to the next state
      if (!buttonAPressed) {
        if (current_state == STATE_HOME) {
          current_state = STATE_MULTIPLAYER;
          draw_multiplayer_screen(tft);
          draw_back_multiplayer(tft, multiplayer_select);

        }
        else if (current_state == STATE_MULTIPLAYER) {
          if(multiplayer_select == 0){
            current_state = STATE_HOME;
            first_home_draw = true;
            draw_homescreen(tft, &first_home_draw);
          }else{
            current_state = STATE_BLUETOOTH_WAIT;
            draw_waiting_screen(tft, isHostReady(), isClientReady(), getHostCode());
            draw_back_waiting(tft, waiting_select);
            hosting = true;
          }
        }
        else if(current_state == STATE_BLUETOOTH_WAIT){
          if(waiting_select == 0){
            current_state = STATE_MULTIPLAYER;
            draw_multiplayer_screen(tft);
            draw_back_multiplayer(tft, multiplayer_select);
          }else{
            A_button_send_ready();
          }
        }
        else if (current_state == STATE_BLUETOOTH_CLIENT){
          if(client_select == 0){
            current_state = STATE_MULTIPLAYER;
            draw_multiplayer_screen(tft);
            draw_back_multiplayer(tft, multiplayer_select);
            client_select = 1;
          }else{
            A_button_client_connection_input();
          }
        }
        else if(current_state == STATE_GAMEOVER){
          current_state = STATE_PLAYING; 
          score0 = score1 = 0;
          game_initialized = false;
        }
      }
      // Set buttonAPressed flag to true to indicate button is pressed
      buttonAPressed = true;
    } else {
      // Reset buttonAPressed when button A is released
      buttonAPressed = false;
    }


    if (current_state == STATE_HOME) {
      draw_homescreen(tft, &first_home_draw);
    }
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
      if (digitalRead(UP) == LOW) {
          updatePaddle(true, &paddles[player_paddle]);
          // Will make sure the previous position of 
          // the paddle is overwritten
          if(paddles[player_paddle].y != prev_paddles[player_paddle].y){
            prev_paddles[player_paddle].paddle_mod = true;
          }
      } // Moves the paddle down
      else if (digitalRead(DOWN) == LOW) {
          updatePaddle(false, &paddles[player_paddle]);
          // Will make sure the previous position of 
          // the paddle is overwritten
          if(paddles[player_paddle].y != prev_paddles[player_paddle].y){
            prev_paddles[player_paddle].paddle_mod = true;
          }
      }
      if(isMultiplayerEnabled()){
        if(prev_paddles[player_paddle].paddle_mod == true){
          sendPaddlePosition(player_paddle, paddles[player_paddle].y);
          prev_paddles[player_paddle].paddle_mod = false;
        }

        // If host, also need to send ball updates (only host controls the ball logic)
        if(isInHostMode()){

          prev_ball.x = ball.x;
          prev_ball.y = ball.y;
          
          // Update ball position
          updateBall(&ball, paddles, &level, &score0, &score1);

          // Send ball position to client
          sendBallPosition(ball.x, ball.y, ball.dx, ball.dy);
          
          // Check if any player scored and send the score update
          if (ball.x <= 0) {
            // Player 2 scored
            sendScore(2);
          } else if (ball.x >= SCREEN_WIDTH - ball.w) {
            // Player 1 scored
            sendScore(1);
          }

          if(paddles[0].y != prev_paddles[0].y){
            prev_paddles[0].paddle_mod = true;
          }
          if(paddles[1].y != prev_paddles[1].y){
            prev_paddles[1].paddle_mod = true;
          }
        }
      } else {
        // Single player mode
        // AI paddle logic
        ai_paddle(&paddles[0], &ball, level);
        //Check if paddle position changed
        prev_ball.x = ball.x;
        prev_ball.y = ball.y;
        // Update ball position
        updateBall(&ball, paddles, &level, &score0, &score1);

        if(paddles[0].y != prev_paddles[0].y){
          prev_paddles[0].paddle_mod = true;
        }
        if(paddles[1].y != prev_paddles[1].y){
          prev_paddles[1].paddle_mod = true;
        }

      }

      // Checks if the game has been won by either player and
      // modified the gamestate accordingly, otherwise
      // draws all game assets
      if(score0 >= GAME_WON || score1 >= GAME_WON) {
          current_state = STATE_GAMEOVER;
          tft.fillScreen(TFT_BLACK);
          draw_endscreen(tft, score0, score1);
      } else {
        if(prev_paddles[0].paddle_mod){
          erasePaddle(tft, prev_paddles[0]);
          prev_paddles[0].paddle_mod = false;
          prev_paddles[0].y = paddles[0].y;
        }
        if(prev_paddles[1].paddle_mod){
          erasePaddle(tft, prev_paddles[1]);
          prev_paddles[1].paddle_mod = false;
          prev_paddles[1].y = paddles[1].y;
        }
        eraseBall(tft, &prev_ball);

        if(prev_score0 != score0 || prev_score1 != score1){
          erase_score(tft);
          prev_score0 = score0;
          prev_score1 = score1;
        }

        drawPaddle(tft, paddles[0]);
        drawPaddle(tft, paddles[1]);
        drawBall(tft, &ball);
        drawScore(tft, score0, score1);
      }
    }
    else if(current_state == STATE_MULTIPLAYER){
      if(currentMillis - lastButtonPressTime > debounceDelay){
        if(digitalRead(UP) == LOW || digitalRead(DOWN) == LOW){
          multiplayer_select = 1 - multiplayer_select;
          lastButtonPressTime = currentMillis;
          draw_back_multiplayer(tft, multiplayer_select);
        }
      }
    }
    else if (current_state == STATE_BLUETOOTH_CLIENT){
      
      if (currentMillis - lastButtonPressTime > debounceDelay) {
        if(client_select == 1){
          if (digitalRead(UP) == LOW) {
            pos_y--;
          } else if (digitalRead(DOWN) == LOW) {
            pos_y = pos_y < NUM_PAD_LENGTH - 1 ? pos_y + 1 : NUM_PAD_LENGTH - 1;
          } else if (digitalRead(LEFT) == LOW) {
            pos_x = pos_x > 0 ? pos_x - 1 : 0;
          } else if (digitalRead(RIGHT) == LOW) {
            pos_x = pos_x < NUM_PAD_WIDTH - 1 ? pos_x + 1 : NUM_PAD_WIDTH - 1;
          }
          if(pos_y <= -1){
            client_select = 0;
            pos_y = 0;
            draw_client_input_back(tft, client_select);
          }
          lastButtonPressTime = currentMillis;
        }else{
          if(digitalRead(DOWN) == LOW){
            client_select = 1;
            lastButtonPressTime = currentMillis;
            draw_client_input_back(tft, client_select);
          }
        }
      }
      draw_button_hover(tft, pos_x, pos_y);
      if(prev_pos_x != pos_x || prev_pos_y != pos_y){
        draw_button_default(tft, prev_pos_x, prev_pos_y);
      }
      prev_pos_x = pos_x;
      prev_pos_y = pos_y;
      draw_numbers(tft, code);
    }
    else if(current_state == STATE_BLUETOOTH_WAIT){
      if(isHostReady() && isClientReady()){
        draw_waiting_screen(tft, isHostReady(), isClientReady(), getHostCode());
        delay(1000);
        current_state = STATE_PLAYING;
        first_home_draw = true;
        resetReadyStatus();
        enableMultiplayer();

        if(!isInHostMode()){
          player_paddle = 0;
        }
      }
      if(currentMillis - lastButtonPressTime > debounceDelay){
        if(digitalRead(UP) == LOW || digitalRead(DOWN) == LOW){
          waiting_select = 1 - waiting_select;
          lastButtonPressTime = currentMillis;
          draw_back_waiting(tft, waiting_select);
        }
      }
    }
    if(hosting){
      if (!isDeviceConnected()) {
        if(isInHostMode()){
          startAdvertising();
        }else{
          connectToHost();
        }
      }
    }
  }
}

//////////// A Button ////////////////////////

// Used to notify host and client if either are ready to connect
static void A_button_send_ready()
{
  sendReadyStatus(isInHostMode());
  draw_waiting_screen(tft, isHostReady(), isClientReady(), getHostCode());
}

// Logic for button presses when connecting to host and logic to connect to host
static void A_button_client_connection_input()
{
  int button = draw_button_pressed(tft, pos_x, pos_y);
  if(button == -2){
    code.remove(code.length() - 1);
  }
  else if(button == -1){
    if (code.length() == 6) {
      Serial.println("Attempting to connect to host with code: quest-" + code);
      
      setConnectionCode(code);
      setHostMode(false);
      stopAdvertising();
      startScan();
  
      unsigned long startTime = millis();
      bool connected = false;
      while (millis() - startTime < 6000 && !connected) {
        connected = connectToHost();
        delay(100);  // Wait for scan results
      }
  
      if (isDeviceConnected()) {
        // Transition to multiplayer gameplay or waiting state
        current_state = STATE_BLUETOOTH_WAIT; // Or some other multiplayer state
      } else {
        Serial.println("Host not found or connection failed.");
        tft.fillScreen(TFT_WHITE);
        tft.setCursor(20, 20);
        tft.setTextColor(TFT_BLACK);
        tft.print("Failed to connect");
      }
    }
  }
  else{
    if(code.length() < 6){
      code += String(button);
    }
  }
}