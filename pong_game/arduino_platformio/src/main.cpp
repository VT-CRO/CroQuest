#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

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


// Bluetooth definitions
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static bool isHost = true; // Starts as a host
static String hostCode = "quest-"; //start of host code
BLEAdvertisedDevice* myDevice;
BLECharacteristic* pCharacteristic;
BLEAdvertising *pAdvertising;

void setupHost(){
  //Create random host code
  hostCode += String(random(100000, 999999));

  BLEDevice::init(hostCode.c_str());
  BLEServer *pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID,
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_WRITE |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
}

void setup() {
  Serial.begin(115200);

  //Initialize rnadom number generator
  randomSeed(analogRead(0));

  //Each device initially setup as a host
  setupHost();

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
  if (!SD.begin(SD_CS, SPI, 100000000)) {
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
      if(!buttonBPressed){
        //Starts pong game
        if (current_state == STATE_HOME) {
            current_state = STATE_PLAYING;
            first_home_draw = true;
        }
        //Resets to homescreen after game has ended
        else if(current_state == STATE_GAMEOVER){
          current_state = STATE_HOME;
          first_home_draw = true;  
          score0 = score1 = 0;
          game_initialized = false;
        }
        else if(current_state == STATE_MULTIPLAYER){
          current_state = STATE_BLUETOOTH_CLIENT;
          tft.fillScreen(TFT_BLACK);
          init_buttons();
          draw_all_buttons(tft);
        }
      }
      buttonBPressed = true;
    }else{
      buttonBPressed = false;
    }
    //Button A 
    // - Resets game after game has ended
    if (digitalRead(A) == LOW) {
      // If button A was not previously pressed, transition to the next state
      if (!buttonAPressed) {
        if (current_state == STATE_HOME) {
          current_state = STATE_MULTIPLAYER;
          draw_multiplayer_screen(tft);
        }
        else if (current_state == STATE_MULTIPLAYER) {
          current_state = STATE_BLUETOOTH_START;
          tft.fillScreen(TFT_BLUE); // Set the blue screen only if transitioning to Bluetooth start
        }
        else if (current_state == STATE_BLUETOOTH_CLIENT){
          int button = draw_button_pressed(tft, pos_x, pos_y);
          if(button == -2){
            code.remove(code.length() - 1);
          }else if(button == -1){
            // do something with the enter
          }
          else{
            if(code.length() < 6){
              code += String(button);
            }
          }
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
    else if (current_state == STATE_BLUETOOTH_CLIENT){
      
      if (currentMillis - lastButtonPressTime > debounceDelay) {
        if (digitalRead(UP) == LOW) {
          pos_y = pos_y > 0 ? pos_y - 1 : NUM_PAD_LENGTH - 1;
          lastButtonPressTime = currentMillis;
        } else if (digitalRead(DOWN) == LOW) {
          pos_y = pos_y < NUM_PAD_LENGTH - 1 ? pos_y + 1 : 0;
          lastButtonPressTime = currentMillis;
        } else if (digitalRead(LEFT) == LOW) {
          pos_x = pos_x > 0 ? pos_x - 1 : NUM_PAD_WIDTH - 1;
          lastButtonPressTime = currentMillis;
        } else if (digitalRead(RIGHT) == LOW) {
          pos_x = pos_x < NUM_PAD_WIDTH - 1 ? pos_x + 1 : 0;
          lastButtonPressTime = currentMillis;
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
  }
}