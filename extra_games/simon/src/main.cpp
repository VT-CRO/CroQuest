#include <Arduino.h>
#include "JpegDrawing.hpp"
TFT_eSPI tft = TFT_eSPI();

JpegDrawing drawing(tft);

// Assets paths
const char* BOARD_PATH = "/simon_assets/disk.jpg";

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// Pin definitions
#define BTN_UP     35
#define BTN_DOWN   34
#define BTN_LEFT   36
#define BTN_RIGHT  39
#define BTN_SELECT 21
#define SD_CS 5

// Game states
enum GameState {
  STATE_INTRO,
  STATE_WATCH,
  STATE_PLAY,
  STATE_GAMEOVER,
  STATE_LEVELUP
};

// Button identifiers
enum ButtonID {
  BUTTON_UP = 0,
  BUTTON_DOWN = 1,
  BUTTON_LEFT = 2,
  BUTTON_RIGHT = 3
};

// Button configurations
const int BUTTON_PINS[4] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT};

// Game variables
GameState currentState = STATE_INTRO;
int sequence[100];         // Sequence storage
int sequenceLength = 0;    // Current sequence length
int playerPos = 0;         // Player's current position in the sequence
int playerScore = 0;       // Player's score

// Button colors
uint16_t buttonColors[4] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_YELLOW};

// Timing variables
unsigned long lastButtonPressTime = 0;
unsigned long buttonDebounceDelay = 200;
unsigned long lastSequenceTime = 0;
unsigned long sequenceDisplayDelay = 800;
unsigned long gameOverTime = 0;
unsigned long levelUpTime = 0;

// Screen positioning 
int buttonSize = 120;
int centerX = SCREEN_WIDTH / 2; 
int centerY = SCREEN_HEIGHT / 2;
int diskCenterX, diskCenterY;
int diskSize;

// Function declarations
void drawIntroScreen();
void drawGameScreen();
void drawGameOverScreen();
void drawLevelUpScreen();
void drawScore();
void drawButton(int buttonId, bool highlight);
void highlightButton(int buttonId);
void generateSequence();
void extendSequence();
void playSequence();
void checkPlayerInput(int buttonPressed);
void startNewGame();
void gameOver();
void levelUp();

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  
  // Initialize buttons
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  
  // Initialize SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  
  diskCenterY = SCREEN_HEIGHT / 2;
  JpegDrawing::ImageInfo dim =  drawing.getJpegDimensions("/simon_assets/disk.jpg");
  
  // disk has same height and width
  diskSize = dim.width;
  
  diskCenterX = diskSize / 2;
  // Set initial state
  currentState = STATE_INTRO;
  drawIntroScreen();
  
  // Initialize random seed
  randomSeed(analogRead(3));
}

void loop() {
  // State machine for game control
  switch (currentState) {
    case STATE_INTRO:
      // Wait for SELECT button to start the game
      if (digitalRead(BTN_SELECT) == LOW && millis() - lastButtonPressTime > buttonDebounceDelay) {
        lastButtonPressTime = millis();
        startNewGame();
      }
      break;
      
    case STATE_WATCH:
      // Display the sequence
      if (millis() - lastSequenceTime > sequenceDisplayDelay) {
        playSequence();
      }
      break;
      
    case STATE_PLAY:
      // Handle player input
      for (int i = 0; i < 4; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW && millis() - lastButtonPressTime > buttonDebounceDelay) {
          lastButtonPressTime = millis();
          highlightButton(i);
          checkPlayerInput(i);
          break;
        }
      }
      break;
      
    case STATE_GAMEOVER:
      if (millis() - gameOverTime > 3000) {
        currentState = STATE_INTRO;
        drawIntroScreen();
      }
      break;
      
    case STATE_LEVELUP:
      if (millis() - levelUpTime > 1500) {
        currentState = STATE_WATCH;
        lastSequenceTime = millis();
      }
      break;
  }
}

void startNewGame() {
  playerScore = 0;
  sequenceLength = 1;
  playerPos = 0;
  tft.fillScreen(TFT_BLACK);
  // Generate first element in sequence
  generateSequence();
  
  // Move to WATCH state
  currentState = STATE_WATCH;
  drawGameScreen();
  lastSequenceTime = millis();
}

void generateSequence() {
  // Generate initial sequence
  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(4); // 0-3 for the 4 buttons
  }
}

void extendSequence() {
  // Add one more element to the sequence
  sequence[sequenceLength] = random(4);
  sequenceLength++;
}

void playSequence() {
  static int currentStep = 0;
  static unsigned long lastStepTime = 0;
  static bool buttonOn = false;
  
  if (currentStep >= sequenceLength) {
    // Sequence complete, move to player input
    currentStep = 0;
    playerPos = 0;
    currentState = STATE_PLAY;
    drawGameScreen(); // Reset all buttons to normal state
    return;
  }
  highlightButton(sequence[currentStep]);
  lastStepTime = millis();
  currentStep++;
}

void checkPlayerInput(int buttonPressed) {
  if (buttonPressed == sequence[playerPos]) {
    // Correct button pressed
    playerPos++;
    
    // Check if player completed the sequence
    if (playerPos >= sequenceLength) {
      playerScore++;
      
      levelUp();
    }
  } else {
    // Wrong button, game over
    gameOver();
  }
}

void levelUp() {
  currentState = STATE_LEVELUP;
  drawLevelUpScreen();
  levelUpTime = millis();
  
  // Extend the sequence for the next level
  extendSequence();
  playerPos = 0;
}

void gameOver() {
  currentState = STATE_GAMEOVER;
  drawGameOverScreen();
  gameOverTime = millis();
}

// Drawing functions
void drawIntroScreen() {
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SIMON", centerX, centerY - 40);
  
  tft.setTextSize(2);
  tft.drawString("MEMORY GAME", centerX, centerY);
  tft.drawString("Press SELECT to start", centerX, centerY + 70);
}

void drawGameScreen() {
  
  // Draw the disk in the center
  int diskX = 0;
  int diskY = diskCenterY - diskSize/2;
  drawing.drawSdJpeg(BOARD_PATH, diskX, diskY);
  
  // Draw score on the right side
  drawScore();
  
  drawing.pushSprite();
}

void drawGameOverScreen() {
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER", centerX, centerY - 30);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Your Score: " + String(playerScore), centerX, centerY + 10);
}

void drawLevelUpScreen() {
  // Overlay a message on the game screen
  int boxWidth = 140;
  int boxHeight = 60;
  tft.fillRect(diskCenterX - boxWidth/2, diskCenterY - boxHeight/2, boxWidth, boxHeight, TFT_BLACK);
  tft.drawRect(diskCenterX - boxWidth/2, diskCenterY - boxHeight/2, boxWidth, boxHeight, TFT_GREEN);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("LEVEL UP!", diskCenterX, diskCenterY - 10);
  tft.drawString("Score: " + String(playerScore), diskCenterX, diskCenterY + 10);

  delay(200);
  drawGameScreen();
}

void drawScore() {
  // Draw scores on the right side
  int scoreX = SCREEN_WIDTH / 2;
  int scoreY = 80;
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM); // Top-left alignment
  
  tft.drawString("P1", scoreX, scoreY);

  tft.drawLine(scoreX, scoreY + 20, scoreX + tft.textWidth("P1"), scoreY + 20, TFT_WHITE);
  
  tft.drawString(String(playerScore), scoreX + 10, scoreY + 30);
}

void drawButton(int buttonId, bool highlight) {
  // Calculate positions around the disk for triangle indicators
  int diskX = diskCenterX - diskSize/2;
  int diskY = diskCenterY - diskSize/2;
  int arrowSize = 30; // Size of the triangle
  
  if (highlight) {
    
    switch(buttonId) {
      case BUTTON_UP: {
        // Draw upward-pointing triangle at top of disk
        int16_t x0 = diskCenterX;
        int16_t y0 = diskY + 20;
        int16_t x1 = x0 - arrowSize/2;
        int16_t y1 = y0 + arrowSize;
        int16_t x2 = x0 + arrowSize/2;
        int16_t y2 = y0 + arrowSize;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_DOWN: {
        // Draw downward-pointing triangle at bottom of disk
        int16_t x0 = diskCenterX;
        int16_t y0 = diskY + diskSize - 20;
        int16_t x1 = x0 - arrowSize/2;
        int16_t y1 = y0 - arrowSize;
        int16_t x2 = x0 + arrowSize/2;
        int16_t y2 = y0 - arrowSize;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_LEFT: {
        // Draw left-pointing triangle on left side of disk
        int16_t x0 = diskX + 20;
        int16_t y0 = centerY;
        int16_t x1 = x0 + arrowSize;
        int16_t y1 = y0 - arrowSize/2;
        int16_t x2 = x0 + arrowSize;
        int16_t y2 = y0 + arrowSize/2;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_RIGHT: {
        // Draw right-pointing triangle on right side of disk
        int16_t x0 = diskX + diskSize - 20;
        int16_t y0 = centerY;
        int16_t x1 = x0 - arrowSize;
        int16_t y1 = y0 - arrowSize/2;
        int16_t x2 = x0 - arrowSize;
        int16_t y2 = y0 + arrowSize/2;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
    }
  } else {
    // Redraw the game screen to remove the triangle
    drawGameScreen();
  }
}

void highlightButton(int buttonId) {
  // Highlight the button
  drawButton(buttonId, true);
  // Could add sound here if hardware supports it
  
  // Delay to keep the button visibly highlighted
  delay(300);
  
  // Return button to normal state
  drawButton(buttonId, false);

  delay(200);
}