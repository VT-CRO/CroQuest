#include "Simon.hpp"

// ======================== Global Definitions ========================

// Assets paths
const char *DISK_PATH = "/simon/assets/disk.jpg";

// Button mapping for Simon game
Button *simonButtons[] = {&A, &B, &left, &right};
NumPad simonPad(tft, drawing, up, down, left, right, A);

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// Game variables
SimonState simonCurrentState = SIMON_HOMESCREEN;
int sequence[100];      // Sequence storage
int sequenceLength = 0; // Current sequence length
int playerPos = 0;      // Player's current position in the sequence
int playerScore = 0;    // Player's score

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

bool first = true;

int simonSelection = 0;
int simonsubselection = 0;
bool start = true;

// ======================== Game Entry ========================
void runSimon() {

  // Initialize display
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  diskCenterY = SCREEN_HEIGHT / 2;

  // Load board asset dimensions
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(DISK_PATH);
  diskSize = dim.width; // Assuming square image
  diskCenterX = diskSize / 2;

  // Seed RNG
  randomSeed(analogRead(3));

  // Set initial state
  simonCurrentState = SIMON_HOMESCREEN;
  drawSimonHomeScreen();

  // Main game loop
  while (true) {
    // updateAllButtons();
    handleSimonFrame();

    // Return to main menu if B is pressed
    if (Start.wasJustPressed()) {
      Serial.println("Returning to menu from Simon");
      delay(500);
      return;
    }
  }
}

void handleSimonFrame() {
  // === Always update button states at the start ===

  // === State machine for Simon ===
  switch (simonCurrentState) {
  case SIMON_HOMESCREEN:
    if (start) {
      drawSimonHomeScreen();
      start = false;
    }

    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (A.wasJustPressed()) {
        if (simonSelection == 1) {
          simonCurrentState = SIMON_MULTIPLAYER_SELECTION;
          drawSimonHomeSelection();
        } else {
          simonStartNewGame();
          simonCurrentState = SIMON_STATE_PLAY;
        }
        lastButtonPressTime = millis();
      } else if (up.wasJustPressed()) {
        if (simonSelection == 1) {
          simonSelection = 0;
          drawSimonHomeSelection();
        }
        lastButtonPressTime = millis();
      } else if (down.wasJustPressed()) {
        if (simonSelection == 0) {
          simonSelection = 1;
          drawSimonHomeSelection();
        }
        lastButtonPressTime = millis();
      }
    }
    break;

  case SIMON_MULTIPLAYER_SELECTION:
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (left.wasJustPressed()) {
        if (simonsubselection == 1) {
          simonsubselection = 0;
          drawSimonHomeSelection();
        }
      } else if (right.wasJustPressed()) {
        if (simonsubselection == 0) {
          simonsubselection = 1;
          drawSimonHomeSelection();
        }
      } else if (A.wasJustPressed()) {
        if (simonsubselection == 0) {
          tft.fillScreen(TFT_BLUE);
          simonCurrentState = SIMON_JOIN_SCREEN;
        } else {
          tft.fillScreen(TFT_BROWN);
          simonCurrentState = SIMON_BLUETOOTH_NUMPAD;
        }
      } else if (up.wasJustPressed()) {
        simonCurrentState = SIMON_HOMESCREEN;
        simonsubselection = 0;
        simonSelection = 1;
        drawSimonHomeSelection();
      }
      lastButtonPressTime = millis();
    }
    break;

  case SIMON_STATE_WATCH:
    if (millis() - lastSequenceTime > sequenceDisplayDelay) {
      simonPlaySequence();
    }
    break;

  case SIMON_STATE_PLAY:
    for (int i = 0; i < 4; i++) {
      if (simonButtons[i]->wasJustPressed()) {
        lastButtonPressTime = millis();
        highlightSimonButton(i);
        simonCheckInput(i);
        break;
      }
    }
    break;

  case SIMON_GAMEOVER_SCREEN:
    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (up.wasJustPressed()) {
        if (simonSelection == 1) {
          simonSelection = 0;
          drawSimonGameOverSelect();
        }
      }
      if (down.wasJustPressed()) {
        if (simonSelection == 0) {
          simonSelection = 1;
          drawSimonGameOverSelect();
        }
      }
      if (A.wasJustPressed()) {
        if (simonSelection == 0) {
          simonCurrentState = SIMON_HOMESCREEN;
          drawSimonHomeScreen();
        } else {
          simonStartNewGame();
          simonCurrentState = SIMON_STATE_PLAY;
        }
        lastButtonPressTime = millis();
      }
    }
    break;

  case SIMON_LEVELUP:
    if (millis() - levelUpTime > 1500) {
      simonCurrentState = SIMON_STATE_WATCH;
      lastSequenceTime = millis();
    }
    break;
  }
}

void simonStartNewGame() {
  playerScore = 0;
  sequenceLength = 1;
  playerPos = 0;
  tft.fillScreen(TFT_BLACK);
  // Generate first element in sequence
  simonGenerateSequence();

  // Move to WATCH state
  simonCurrentState = SIMON_STATE_WATCH;
  drawSimonGameScreen();
  lastSequenceTime = millis();
}

void simonGenerateSequence() {
  // Generate initial sequence
  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(4); // 0-3 for the 4 buttons
  }
}

void simonExtendSequence() {
  // Add one more element to the sequence
  sequence[sequenceLength] = random(4);
  sequenceLength++;
}

void simonPlaySequence() {
  static int currentStep = 0;
  static unsigned long lastStepTime = 0;
  static bool buttonOn = false;

  if (currentStep >= sequenceLength) {
    // Sequence complete, move to player input
    currentStep = 0;
    playerPos = 0;
    simonCurrentState = SIMON_STATE_PLAY;
    drawSimonGameScreen(); // Reset all buttons to normal state
    return;
  }
  highlightSimonButton(sequence[currentStep]);
  lastStepTime = millis();
  currentStep++;
}

void simonCheckInput(int buttonPressed) {
  if (buttonPressed == sequence[playerPos]) {
    // Correct button pressed
    playerPos++;

    // Check if player completed the sequence
    if (playerPos >= sequenceLength) {
      playerScore++;

      simonLevelUp();
    }
  } else {
    // Wrong button, game over
    simonGameOver();
  }
}

void simonLevelUp() {
  simonCurrentState = SIMON_LEVELUP;
  drawSimonLevelUpScreen();
  levelUpTime = millis();

  // Extend the sequence for the next level
  simonExtendSequence();
  playerPos = 0;
}

void simonGameOver() {
  simonCurrentState = SIMON_GAMEOVER_SCREEN;
  drawSimonGameOverScreen();
  gameOverTime = millis();
}

void drawSimonGameScreen() {

  if (first) {
    // Draw the disk in the center
    int diskX = 0;
    int diskY = diskCenterY - diskSize / 2;
    drawing.drawSdJpeg(DISK_PATH, diskX, diskY);
    first = false;
  }

  // Draw score on the right side
  drawSimonScore();

  drawing.pushSprite(true);
}

void drawSimonGameOverScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER", centerX, centerY - 40);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Your Score: " + String(playerScore), centerX, centerY);

  drawSimonGameOverSelect();
}

void drawSimonGameOverSelect() {
  const int textSize = 2;
  const int paddingX = 10;
  const int paddingY = 4;
  const int spacing = 10; // vertical space between boxes
  const int highlightColor = TFT_WHITE;

  const char *optionHome = "Press for homescreen";
  const char *optionRestart = "Press to restart";

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(textSize);

  // Calculate Y positions
  int h = 16 * textSize + paddingY * 2;
  int yHome = centerY + 40;
  int yRestart = yHome + h + spacing;

  int wHome = tft.textWidth(optionHome);
  int wRestart = tft.textWidth(optionRestart);

  // Clear both option areas
  tft.drawRect(centerX - wHome / 2 - paddingX, yHome - h / 2,
               wHome + 2 * paddingX, h, TFT_BLACK);
  tft.drawRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2,
               wRestart + 2 * paddingX, h, TFT_BLACK);

  // Draw highlight rectangle for current simonSelection
  if (simonSelection == 0) {
    tft.drawRect(centerX - wHome / 2 - paddingX, yHome - h / 2,
                 wHome + 2 * paddingX, h, highlightColor);
  } else if (simonSelection == 1) {
    tft.drawRect(centerX - wRestart / 2 - paddingX, yRestart - h / 2,
                 wRestart + 2 * paddingX, h, highlightColor);
  }

  // Draw option texts
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(optionHome, centerX, yHome);
  tft.drawString(optionRestart, centerX, yRestart);
}

void drawSimonLevelUpScreen() {
  // Overlay a message on the game screen
  int boxWidth = 140;
  int boxHeight = 60;
  tft.fillRect(diskCenterX - boxWidth / 2, diskCenterY - boxHeight / 2,
               boxWidth, boxHeight, TFT_BLACK);
  tft.drawRect(diskCenterX - boxWidth / 2, diskCenterY - boxHeight / 2,
               boxWidth, boxHeight, TFT_YELLOW);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("LEVEL UP!", diskCenterX, diskCenterY - 10);
  tft.drawString("Score: " + String(playerScore), diskCenterX,
                 diskCenterY + 10);

  delay(200);
  drawSimonGameScreen();
}

void drawSimonScore() {
  const int scoreX = diskSize + 20; // Positioned right of the disk
  int scoreY = 5;                   // Top position
  const int boxWidth = 100;
  const int boxHeight = 50;
  const int padding = 8;

  // Draw background box for the score
  tft.fillRoundRect(scoreX, scoreY, boxWidth, boxHeight, 8, TFT_BLACK);
  tft.drawRoundRect(scoreX, scoreY, boxWidth, boxHeight, 8, TFT_YELLOW);

  // Set text properties
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);

  // Player label
  tft.drawString("P1", scoreX + padding, scoreY + padding);

  // Score value
  tft.setTextSize(2);
  tft.drawString(String(playerScore), scoreX + padding, scoreY + 28);
}

void drawSimonButton(int index, bool highlight) {
  int arrowSize = 45;

  int diskX = diskCenterX - diskSize / 2;
  int diskY = diskCenterY - diskSize / 2;

  // Set arrow coordinates based on direction
  if (highlight) {
    switch (index) {
    case 0: { // Up
      int16_t x0 = diskCenterX;
      int16_t y0 = diskY + 30;
      int16_t x1 = x0 - arrowSize / 2;
      int16_t y1 = y0 + arrowSize;
      int16_t x2 = x0 + arrowSize / 2;
      int16_t y2 = y0 + arrowSize;
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
      break;
    }
    case 1: { // Down
      int16_t x0 = diskCenterX;
      int16_t y0 = diskY + diskSize - 30;
      int16_t x1 = x0 - arrowSize / 2;
      int16_t y1 = y0 - arrowSize;
      int16_t x2 = x0 + arrowSize / 2;
      int16_t y2 = y0 - arrowSize;
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
      break;
    }
    case 2: { // Left
      int16_t x0 = diskX + 30;
      int16_t y0 = diskCenterY;
      int16_t x1 = x0 + arrowSize;
      int16_t y1 = y0 - arrowSize / 2;
      int16_t x2 = x0 + arrowSize;
      int16_t y2 = y0 + arrowSize / 2;
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_WHITE);
      break;
    }
    case 3: { // Right
      int16_t x0 = diskX + diskSize - 30;
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
    drawSimonGameScreen();
  }
}

void highlightSimonButton(int buttonId) {
  // Highlight the button
  drawSimonButton(buttonId, true);
  // Could add sound here

  // Optional: play a tone or add feedback here
  // tone(BUZZER_PIN, 440 + buttonId * 100, 150);

  // Delay to keep the button visibly highlighted
  delay(200);

  // Return button to normal state
  drawSimonButton(buttonId, false);

  if (simonCurrentState == SIMON_STATE_WATCH) {
    delay(300);
  }
}

void drawSimonHomeScreen() {
  // Clear the screen
  uint16_t bgColor = TFT_BLACK;
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
  tft.fillRect(0, y_single - 15, SCREEN_WIDTH, 35, TFT_BLACK);
  tft.fillRect(0, y_multi - 15, SCREEN_WIDTH, 80, TFT_BLACK);

  if (simonSelection == 0) {
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

    if (simonCurrentState == SIMON_MULTIPLAYER_SELECTION) {
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

      // Highlight rectangle
      if (simonsubselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (simonsubselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw sub-option labels
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}
