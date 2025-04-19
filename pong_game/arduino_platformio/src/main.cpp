#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <LovyanGFX.hpp>
#include "pong.h"
// Pin definitions and constants
#define UP 8
#define DOWN 18
#define LEFT 3
#define RIGHT 46
#define A 16
#define B 17
#define HOST_CODE_SIZE 6

// TFT display pins - using SPI2_HOST (previously known as VSPI)
#define TFT_MISO  12
#define TFT_LED   21
#define TFT_SCK   14
#define TFT_MOSI  13
#define TFT_DC     2
#define TFT_RESET  4
#define TFT_CS    15

#define SD_CS     5  // SD Card CS pin

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;  // Reduced SPI frequency for increased stability
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = TFT_SCK;
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = TFT_MISO;
      cfg.pin_dc   = TFT_DC;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    {
      auto cfg = _panel.config();
      cfg.pin_cs   = TFT_CS;
      cfg.pin_rst  = TFT_RESET;
      cfg.pin_busy = -1;

      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable      = true;
      cfg.invert        = false;
      cfg.rgb_order    = false;
      cfg.dlen_16bit  = false;
      cfg.bus_shared  = true;

      _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

LGFX tft;

GameState current_state = STATE_HOME;
Ball ball;
Ball prev_ball;
Paddle paddles[2];

typedef struct {
  int y;
  bool paddle_mod;
} Prev_Paddle;

void drawPaddle(LGFX& tft, Paddle paddle, Prev_Paddle* prev_paddle);
void drawBall(LGFX& tft, Ball* ball, Ball* prev_ball);
void drawScore(LGFX& tft, int score0, int score1);

Prev_Paddle prev_paddles[2];
int level = 1;
int score0 = 0;
int score1 = 0;
bool game_initialized = false;

// Frame rate control variables
unsigned long previousMillis = 0;
const int targetFPS = 60;  // Set your desired frame rate
const unsigned long frameTime = 1000 / targetFPS;  // Time per frame in milliseconds
unsigned long currentFPS = 0;
unsigned long fpsUpdateTime = 0;
unsigned int frameCount = 0;

// Function to draw the paddle on the screen
void drawPaddle(LGFX& tft, Paddle paddle, Prev_Paddle* prev_paddle) {
  // Clear previous paddle position (if any)
  if(prev_paddle->y != paddle.y || prev_paddle->paddle_mod){
    tft.fillRect(paddle.x, prev_paddle->y, paddle.w, paddle.h, TFT_BLACK);
    prev_paddle->y = paddle.y;
    prev_paddle->paddle_mod = false;
  }

  // Draw new paddle position
  tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_WHITE);
}

// Function to draw the ball on the screen
void drawBall(LGFX& tft, Ball* ball, Ball* prev_ball) {
  if(prev_ball->x != ball->x || prev_ball->y != ball->y){
    tft.fillCircle(prev_ball->x + (ball->w/2), prev_ball->y + (ball->w/2), ball->w/2, TFT_BLACK);
    prev_ball->x = ball->x;
    prev_ball->y = ball->y;
  }

  // Draw new ball position
  tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_WHITE);
}

// Function to draw the score on the screen
void drawScore(LGFX& tft, int score0, int score1) {
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
  
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  digitalWrite(TFT_CS, HIGH);

  // digitalWrite(TFT_CS, HIGH);
  // digitalWrite(SD_CS, LOW);
  SD.end();
  delay(2000);
  // Attempt to initialize the SD card
  if (!SD.begin(SD_CS, SPI, 4000000)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully.");
  
  // Initialize frame rate variables
  previousMillis = millis();
  fpsUpdateTime = previousMillis;
}


bool first_home_draw = true;
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
    if (digitalRead(B) == LOW) {
      if (current_state == STATE_HOME || current_state == STATE_BLUETOOTH_HOST) {
          current_state = STATE_PLAYING;
          first_home_draw = true;
      }
    }

    if (digitalRead(A) == LOW) {
      if (current_state == STATE_GAMEOVER) {
          current_state = STATE_PLAYING;
          score0 = score1 = 0;
          game_initialized = false;
      }
    }

    // delay(2000);  // Display image for 2 seconds

    if (current_state == STATE_HOME) {
      // Check if the BMP file exists on the SD card
      if(first_home_draw){
        if (SD.exists("/home_screen.bmp")) {
          Serial.println("Drawing BMP...");
          tft.drawBmpFile(SD, "/home_screen.bmp", 0, 0);
        } else {
          Serial.println("File not found!");
        }
        first_home_draw = false;
      }
    }
    else if (current_state == STATE_PLAYING) {
      if (!game_initialized) {
          initialize_game(&ball, paddles, &level);
          prev_ball.x = ball.x;
          prev_ball.y = ball.y;
          prev_ball.h = ball.h;
          prev_ball.w = ball.w;

          prev_paddles[0].y = paddles[0].y;
          prev_paddles[1].y = paddles[1].y;
          prev_paddles[0].paddle_mod = false;
          prev_paddles[1].paddle_mod = false;
          game_initialized = true;
          tft.fillScreen(TFT_BLACK);
      }

      if (digitalRead(UP) == LOW) {
          updatePaddle(true, &paddles[1]);
          if(paddles[1].y != prev_paddles[1].y){
            prev_paddles[1].paddle_mod = true;
          }
      } else if (digitalRead(DOWN) == LOW) {
          updatePaddle(false, &paddles[1]);
          if(paddles[1].y != prev_paddles[1].y){
            prev_paddles[1].paddle_mod = true;
          }
      }

      ai_paddle(&paddles[0], &ball, level);
      if(paddles[0].y != prev_paddles[0].y){
        prev_paddles[0].paddle_mod = true;
      }
      updateBall(&ball, paddles, &level, &score0, &score1);

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
    else if (current_state == STATE_GAMEOVER) {
      
      //Player that won the game
      tft.setTextSize(3);
      String player = score0 >= GAME_WON ? "Player 1 Wins!" : "Player 2 Wins!";
      tft.setCursor((SCREEN_WIDTH - tft.textWidth(player)) / 2, 60);
      tft.print(player);

      //The score for both players
      tft.setTextSize(2);
      String scorestr0 = "P1: " + String(score0) + "  ";
      String scorestr1 = "P2: " + String(score1);
      tft.setCursor((SCREEN_WIDTH - tft.textWidth(scorestr0) - tft.textWidth(scorestr1))/2, 120);
      tft.print(scorestr0);
      tft.print(scorestr1);
  
     String restart = "Press (A) to restart";
     tft.fillRect((SCREEN_WIDTH - tft.textWidth(restart)) /2, 180, tft.textWidth(restart), tft.fontHeight(), TFT_BLACK);
     tft.setCursor((SCREEN_WIDTH - tft.textWidth(restart)) /2, 180);
     tft.print(restart);
  }
  }
}