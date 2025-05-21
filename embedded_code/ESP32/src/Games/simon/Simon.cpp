#include "Simon.hpp"

// ======================== Global Definitions ========================

// Assets paths
const char *DISK_PATH = "/simon/assets/disk.jpg";
const char *UP_TRIANGLE = "/simon/assets/triangle_up.jpg";
const char *DOWN_TRIANGLE = "/simon/assets/triangle_down.jpg";
const char *LEFT_TRIANGLE = "/simon/assets/triangle_left.jpg";
const char *RIGHT_TRIANGLE = "/simon/assets/triangle_right.jpg";

// Button mapping for Simon game
Button *simonButtons[] = {&up, &down, &left, &right};

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define MAX_PLAYERS 6
bool playerFailed = false;
int playerLevels[MAX_PLAYERS] = {0}; // Track how many levels each player passed

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
int diskX = 0;
int diskY = 0;
int centerX = SCREEN_WIDTH / 2;
int centerY = SCREEN_HEIGHT / 2;
int diskCenterX, diskCenterY;
int diskSize;

bool first = true;

int currentStep = 0;
unsigned long lastStepTime = 0;
bool showing = false;

int simonSelection = 0;
int simonsubselection = 0;
bool start = true;

// ======================== Game Entry ========================
void runSimon() {

  // Initialize display
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Load board asset dimensions
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(DISK_PATH);
  diskSize = dim.width; // Still assuming square

  // Center of the disk: left third of screen, vertical middle
  // Compute center positions
  diskCenterX = SCREEN_WIDTH / 3;
  diskCenterY = SCREEN_HEIGHT / 2;

  // Save position for reuse
  diskX = diskCenterX - 120; // Change if you want to move the disk
  diskY = diskCenterY - 120;

  // Draw the disk
  drawing.drawSdJpeg(DISK_PATH, diskX, diskY); // First render (first = true)

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
  playerLevels[0] = 0;
  playerFailed = false;

  sequenceLength = 1;
  playerPos = 0;
  tft.fillScreen(TFT_BLACK);

  simonGenerateSequence();

  first = true; // This makes the sprites not to shift IMPORTANT!!!

  // Draws the disk
  drawSimonGameScreen();

  // Print Score
  drawSimonScore();

  // Player Status
  drawPlayerStatusTable();

  // Reset sequence playback state
  currentStep = 0;
  showing = false;
  lastStepTime = millis() - 600;

  simonCurrentState = SIMON_STATE_WATCH;
}

void simonGenerateSequence() {
  // Generate initial sequence
  for (int i = 0; i < 100; i++) {
    sequence[i] = random(0, 4); // 0-3 for the 4 buttons
  }
}

void simonExtendSequence() {
  // Add one more element to the sequence
  sequence[sequenceLength] = random(4);
  sequenceLength++;
}

void simonPlaySequence() {
  unsigned long now = millis();

  if (currentStep >= sequenceLength) {
    currentStep = 0;
    simonCurrentState = SIMON_STATE_PLAY;
    drawSimonGameScreen();
    return;
  }

  if (!showing && now - lastStepTime > 400) {
    highlightSimonButton(sequence[currentStep]);
    showing = true;
    lastStepTime = now;
  }

  if (showing && now - lastStepTime > 600) {
    drawSimonGameScreen();
    showing = false;
    currentStep++;
    lastStepTime = now;
  }
}

void simonCheckInput(int buttonPressed) {
  if (buttonPressed == sequence[playerPos]) {
    playerScore++;    // Count 1 point for this correct input
    drawSimonScore(); // Update display

    playerPos++;
    if (playerPos == sequenceLength) {
      simonLevelUp(); // Full pattern matched
    }
  } else {
    playerFailed = true;     // Mark failure
    drawPlayerStatusTable(); // Shows table
    delay(800);              // Let player visualize the table
    simonGameOver();
  }
}

void simonLevelUp() {

  playerLevels[0]++; // Only P1 for now

  simonExtendSequence(); // Add one new triangle to the sequence
  playerPos = 0;

  simonCurrentState = SIMON_LEVELUP;
  levelUpTime = millis();

  drawSimonGameScreen();   // Redraw disk
  drawSimonScore();        // Show updated score
  drawPlayerStatusTable(); // Draw updated check marks
}

void simonGameOver() {
  simonCurrentState = SIMON_GAMEOVER_SCREEN;
  drawSimonGameOverScreen();
  gameOverTime = millis();
}

void drawSimonGameScreen() {

  // Print Score
  drawSimonScore();

  // Draw the entire disk sprite.
  drawing.clearSprite();
  drawing.drawSdJpeg(DISK_PATH, diskX, diskY);
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
  const int scoreX = 5;                  // Left margin
  const int scoreY = SCREEN_HEIGHT - 30; // Bottom of screen
  const int padding = 4;

  // Set text properties
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM); // Top-left corner

  // Draw score string
  String scoreText = "Score: " + String(playerScore);
  tft.drawString(scoreText, scoreX + padding, scoreY);
}

void highlightSimonButton(int buttonId) {
  // Draw triangle sprite overlay
  drawSimonTriangleOverlay(buttonId);

  // Play tone
  tone(21, 440 + buttonId * 100, 150);

  // Keep highlight visible briefly
  delay(200);

  // Clear by redrawing full game screen (disk + score, etc.)
  drawSimonGameScreen();

  delay(150); // Short pause before resuming logic
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

void drawCenteredOverlay(const char *imagePath) {
  File imageFile = SD.open(imagePath);
  if (!imageFile) {
    Serial.print("❌ Failed to open: ");
    Serial.println(imagePath);
    return;
  }

  tft.setSwapBytes(true); // for TFT_eSPI JPEG decoder

  if (JpegDec.decodeSdFile(imageFile)) {
    int imgW = JpegDec.width;
    int imgH = JpegDec.height;

    int xpos = diskCenterX - imgW / 2;
    int ypos = diskCenterY - imgH / 2;

    while (JpegDec.read()) {
      int x = JpegDec.MCUx * JpegDec.MCUWidth + xpos;
      int y = JpegDec.MCUy * JpegDec.MCUHeight + ypos;

      if (x < tft.width() && y < tft.height()) {
        tft.pushImage(x, y, JpegDec.MCUWidth, JpegDec.MCUHeight,
                      JpegDec.pImage);
      }
    }
  }

  imageFile.close();
}

void drawSimonTriangleOverlay(int buttonId) {
  const char *path;
  int x = 0, y = 0;

  switch (buttonId) {
  case 0:
    path = "/simon/assets/triangle_up.jpg";
    x = diskX + 30;
    y = diskY - 15; // DON'T TOUCH
    break;
  case 1:
    path = "/simon/assets/triangle_down.jpg";
    x = diskX + 30; // DON'T TOUCH
    y = diskY + 120;
    break;
  case 2:
    path = "/simon/assets/triangle_left.jpg";
    x = diskX - 15; // DON'T TOUCH
    y = diskY + 30;
    break;
  case 3:
    path = "/simon/assets/triangle_right.jpg";
    x = diskX + 120; // DON'T TOUCH
    y = diskY + 30;
    break;
  default:
    return;
  }

  drawing.deleteSprite();

  drawing.drawSdJpeg(path, x, y);
  drawing.pushSprite(false, true, 0xFFFF); // white treated as transparen
}

void drawPlayerStatusTable() {
  const int startX = SCREEN_WIDTH - 160; // "-" Move more to the left
  const int startY = 20;
  const int nameHeight = 20;
  const int checkSize = 12;
  const int checkSpacing = 16;
  const int maxPerRow = 6;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  for (int i = 0; i < 1; i++) { // 1 for now (P1)
    int nameX = startX;
    int nameY = startY + i * (nameHeight + 50);
    String name = "P" + String(i + 1);
    tft.drawString(name, nameX, nameY);

    // Draw green checks for levels passed
    for (int lvl = 0; lvl < playerLevels[i]; lvl++) {
      int row = lvl / maxPerRow;
      int col = lvl % maxPerRow;

      int checkX = nameX + col * (checkSize + checkSpacing);
      int checkY = nameY + nameHeight + 5 + row * (checkSize + 10);

      tft.fillCircle(checkX, checkY, checkSize / 2, TFT_GREEN);
      tft.drawCircle(checkX, checkY, checkSize / 2, TFT_WHITE);
    }

    // Red X if the player failed
    if (playerFailed) {
      int failIndex = playerLevels[i];
      int row = failIndex / maxPerRow;
      int col = failIndex % maxPerRow;

      int failX = nameX + col * (checkSize + checkSpacing);
      int failY = nameY + nameHeight + 5 + row * (checkSize + 10);

      tft.fillCircle(failX, failY, checkSize / 2, TFT_RED);
      tft.drawCircle(failX, failY, checkSize / 2, TFT_WHITE);
      tft.drawLine(failX - 3, failY - 3, failX + 3, failY + 3, TFT_WHITE);
      tft.drawLine(failX - 3, failY + 3, failX + 3, failY - 3, TFT_WHITE);
    }
  }
}
