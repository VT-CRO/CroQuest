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
  STATE_LEVELUP,
  MULTIPLAYER_SELECTION,
  BLUETOOTH_NUMPAD,
  JOIN_SCREEN,
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

int selection = 0;
int subselection = 0;
bool start = true;

// Function declarations
void drawHomeScreenSimon();
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
void drawSimonHomeSelection();
void drawGameOverSelect();

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(3);
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
  
  // Initialize random seed
  randomSeed(analogRead(3));
  
  // Set initial state
  currentState = STATE_INTRO;
  drawHomeScreenSimon();
}

void loop() {
  // State machine for game control
  switch (currentState) {
    case STATE_INTRO:
      if(start){
        drawHomeScreenSimon();
        start = false;
      }
      // Wait for SELECT button to start the game
      if(millis() - lastButtonPressTime > buttonDebounceDelay){
        if (digitalRead(BTN_SELECT) == LOW) {
          if(selection == 1){
            currentState = MULTIPLAYER_SELECTION;
            drawSimonHomeSelection();
          }
          else{
            currentState = STATE_PLAY;
            startNewGame();
          }
          lastButtonPressTime = millis();
        }
        else if(digitalRead(BTN_UP) == LOW){
          if(selection == 1){
            selection = 0;
            drawSimonHomeSelection();
          }
          lastButtonPressTime = millis();
        }
        else if(digitalRead(BTN_DOWN) == LOW){
          if(selection == 0){
            selection = 1;
            drawSimonHomeSelection();
          }
          lastButtonPressTime = millis();
        }
      }
      break;
    case MULTIPLAYER_SELECTION:
      if(millis() - lastButtonPressTime > buttonDebounceDelay/2){
        if(digitalRead(BTN_LEFT) == LOW){
          if(subselection == 1){
            subselection = 0;
            drawSimonHomeSelection();
          }
        }
        else if(digitalRead(BTN_RIGHT) == LOW){
          if(subselection == 0){
            subselection = 1;
            drawSimonHomeSelection();
          }
        }
        else if(digitalRead(BTN_SELECT) == LOW){
          if(subselection == 0){
            tft.fillScreen(TFT_BLUE);
            currentState = JOIN_SCREEN;
          }else{
            tft.fillScreen(TFT_BROWN);
            currentState = BLUETOOTH_NUMPAD;
          }
        }
        else if(digitalRead(BTN_UP) == LOW){
          currentState = STATE_INTRO;
          subselection = 0;
          selection = 1;
          drawSimonHomeSelection();
        }
        lastButtonPressTime = millis();
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
      if(millis() - lastButtonPressTime > buttonDebounceDelay){
        if(digitalRead(BTN_UP) == LOW){
          if(selection == 1){
            selection = 0;
            drawGameOverSelect();
          }
        }
        if(digitalRead(BTN_DOWN) == LOW){
          if(selection == 0){
            selection = 1;
            drawGameOverSelect();
          }
        }
        if (digitalRead(BTN_SELECT) == LOW) {
          if(selection == 0){
            lastButtonPressTime = millis();
            currentState = STATE_INTRO;
            drawHomeScreenSimon();
          }else{
            currentState = STATE_PLAY;
            startNewGame();
          }
        }
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
  tft.drawString("GAME OVER", centerX, centerY - 40);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Your Score: " + String(playerScore), centerX, centerY);

  drawGameOverSelect();
}

void drawGameOverSelect() {
  const int textSize = 2;
  const int paddingX = 10;
  const int paddingY = 4;
  const int spacing = 10; // vertical space between boxes
  const int highlightColor = TFT_WHITE;

  const char* optionHome = "Press for homescreen";
  const char* optionRestart = "Press to restart";

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(textSize);

  // Calculate Y positions
  int h = 16 * textSize + paddingY * 2;
  int yHome = centerY + 40;
  int yRestart = yHome + h + spacing;

  int wHome = tft.textWidth(optionHome);
  int wRestart = tft.textWidth(optionRestart);

  // Clear both option areas
  tft.drawRect(centerX - wHome / 2 - paddingX, yHome - h / 2, wHome + 2 * paddingX, h, TFT_BLACK);
  tft.drawRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2, wRestart + 2 * paddingX, h, TFT_BLACK);

  // Draw highlight rectangle for current selection
  if (selection == 0) {
    tft.drawRect(centerX - wHome / 2 - paddingX, yHome - h / 2, wHome + 2 * paddingX, h, highlightColor);
  } else if (selection == 1) {
    tft.drawRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2, wRestart + 2 * paddingX, h, highlightColor);
  }

  // Draw option texts
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(optionHome, centerX, yHome);
  tft.drawString(optionRestart, centerX, yRestart);
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
  const int scoreX = diskSize + 20;  // Positioned right of the disk
  int scoreY = 5;             // Top position
  const int boxWidth = 100;
  const int boxHeight = 50;
  const int padding = 8;

    // Draw background box for the score
    tft.fillRoundRect(scoreX, scoreY, boxWidth, boxHeight, 8, TFT_BLACK);
    tft.drawRoundRect(scoreX, scoreY, boxWidth, boxHeight, 8, TFT_GREEN);
  
    // Set text properties
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
  
    // Player label
    tft.drawString("P1", scoreX + padding, scoreY + padding);
  
    // Score value
    tft.setTextSize(2);
    tft.drawString(String(playerScore), scoreX + padding, scoreY + 28);
}


void drawButton(int buttonId, bool highlight) {
  // Larger arrows, closer to center
  int arrowSize = 45;

  int diskX = diskCenterX - diskSize / 2;
  int diskY = diskCenterY - diskSize / 2;

  if (highlight) {
    switch (buttonId) {
      case BUTTON_UP: {
        int16_t x0 = diskCenterX;
        int16_t y0 = diskY + 30; // closer to center
        int16_t x1 = x0 - arrowSize / 2;
        int16_t y1 = y0 + arrowSize;
        int16_t x2 = x0 + arrowSize / 2;
        int16_t y2 = y0 + arrowSize;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_DOWN: {
        int16_t x0 = diskCenterX;
        int16_t y0 = diskY + diskSize - 30; // closer to center
        int16_t x1 = x0 - arrowSize / 2;
        int16_t y1 = y0 - arrowSize;
        int16_t x2 = x0 + arrowSize / 2;
        int16_t y2 = y0 - arrowSize;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_LEFT: {
        int16_t x0 = diskX + 30; // closer to center
        int16_t y0 = diskCenterY;
        int16_t x1 = x0 + arrowSize;
        int16_t y1 = y0 - arrowSize / 2;
        int16_t x2 = x0 + arrowSize;
        int16_t y2 = y0 + arrowSize / 2;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
      case BUTTON_RIGHT: {
        int16_t x0 = diskX + diskSize - 30; // closer to center
        int16_t y0 = diskCenterY;
        int16_t x1 = x0 - arrowSize;
        int16_t y1 = y0 - arrowSize / 2;
        int16_t x2 = x0 - arrowSize;
        int16_t y2 = y0 + arrowSize / 2;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
        break;
      }
    }
  } else {
    drawGameScreen(); // Redraw to clear highlight
  }
}

void highlightButton(int buttonId) {
  // Highlight the button
  drawButton(buttonId, true);
  // Could add sound here
  
  // Delay to keep the button visibly highlighted
  if(currentState == STATE_WATCH){
    delay(200);
  }
  
  // Return button to normal state
  drawButton(buttonId, false);

  if(currentState == STATE_WATCH){
    delay(200);
  }
}

void drawHomeScreenSimon() {
  // Clear the screen with a distinct color (e.g., dark blue)
  uint16_t bgColor = TFT_DARKGREY;
  tft.fillScreen(bgColor);

  // Set title properties
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(4);
  tft.drawString("SIMON", SCREEN_WIDTH / 2, 40);

  // Tagline
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Memory Challenge", SCREEN_WIDTH / 2, 90);

  // Options
  tft.setTextSize(2);
  tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, 180);
  tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, 230);

  drawSimonHomeSelection();
}

void drawSimonHomeSelection() {
  int y_single = 180;
  int y_multi = 230;
  int y_sub = y_multi + 40;

  // Clear option areas
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, SCREEN_WIDTH, 35, TFT_DARKGREY);
  tft.fillRect(0, y_multi - 15, SCREEN_WIDTH, 80, TFT_DARKGREY);

  if (selection == 0) {
    // Single-player selected
    tft.setTextSize(3);
    tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);
  } else {
    // Multiplayer selected
    tft.setTextSize(2);
    tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);

    if (currentState == MULTIPLAYER_SELECTION) {
      const char* sub1 = "Host a Game";
      const char* sub2 = "Join a Game";

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

      // Highlight rectangle
      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2, sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2, sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw sub-option labels
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}