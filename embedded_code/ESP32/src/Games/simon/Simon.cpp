#include "Simon.hpp"
#include "EndScreen/EndScreen.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include <vector>

// ========== Drawing ==========
void drawSimonHomeScreen();
void drawSimonGameScreen();
void drawSimonScore();
void highlightSimonButton(int buttonId);
void drawSimonHomeSelection();
void drawSimonTriangleOverlay(int buttonId);
static void drawHighscore();

// ========== Logic ==========
void simonGenerateSequence();
void simonExtendSequence();
void simonPlaySequence();
void simonCheckInput(int buttonPressed);
void simonStartNewGame();
void simonGameOver();
void simonLevelUp();
void simonHandleInput();
void drawPlayerStatusTable();

// =========== AUDIO ============
static void playGameOverSound();

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
SimonState simon_game_state = SIMON_HOMESCREEN;
int sequence[100];      // Sequence storage
int sequenceLength = 0; // Current sequence length
int playerPos = 0;      // Player's current position in the sequence
static int highscore = 0;
int playerScore = 0;
bool multiplayer = false;

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

// Numpad
static NumPad<SimonState> pad(drawSimonHomeScreen, simonStartNewGame,
                              &simon_game_state, SIMON_HOMESCREEN,
                              SIMON_STATE_WATCH);

// Multiplayer
bool simonStateChanged = false;
struct SimonPlayer {
  int id;                 // Unique identifier
  String name = "";       // Display name of the player
  int score = 0;          // Current score
  String status = "idle"; // e.g., "idle", "ready", "playing", "eliminated"
  SimonPlayer(int id_, const String &name_, int score_, const String &status_)
      : id(id_), name(name_), score(score_), status(status_) {}
};

static SimonPlayer currentPlayer =
    SimonPlayer(atoi(generate6DigitCode().c_str()), String(settings.name), 0,
                String("idle"));
std::vector<SimonPlayer> simonPlayers;
static String multiplayerMode = "NONE";
static bool update = false;

// ======================== Game Entry ========================
void runSimon() {

  resetExitFlag(); // Resets flag for Main Menu

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

  // clear sprite and cache
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  // Draw the disk
  drawing.drawSdJpeg(DISK_PATH, diskX, diskY); // First render (first = true)

  // Seed RNG
  randomSeed(analogRead(3));

  // Set initial state
  simon_game_state = SIMON_HOMESCREEN;
  drawSimonHomeScreen();

  currentPlayer = SimonPlayer(atoi(generate6DigitCode().c_str()),
                              String(settings.name), 0, String("idle"));
  update = false;
  // reset multiplayer flag
  multiplayerMode = "NONE";

  // Main game loop
  while (true) {
    // updateAllButtons();
    handleSimonFrame();

    if (getExitFlag())
      return;

    // Return to main menu if B is pressed
    if (simon_game_state == SIMON_HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu.");
      delay(500);
      return;
    }
  }
}

void handleSimonFrame() {

  if (checkStartButtonAndExit(tft))
    return;

  // === State machine for Simon ===
  switch (simon_game_state) {

  case SIMON_HOMESCREEN:
    if (start) {
      drawSimonHomeScreen();
      start = false;
    }

    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (A.wasJustPressed()) {
        if (simonSelection == 1) {
          simon_game_state = SIMON_MULTIPLAYER_SELECTION;
          drawSimonHomeSelection();
        } else {
          multiplayerMode = "SINGLE";
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

          // HOST = CENTRAL
          BluetoothManager::initCentral(tft);
          BluetoothCentral &central = BluetoothManager::getCentral();

          std::string code = generate6DigitCode();

          // Set the screen for HostGame
          HostGame::init(tft);

          // Now safely show code
          HostGame::showCode(String(code.c_str()));

          central.scanAndConnectLoop(code);

          multiplayerMode = "HOST";

          if (!BluetoothManager::getCentral().getConnectedClients().empty()) {

            // Flush any held buttons to prevent input carryover
            delay(300); // debounce delay
            while (A.isPressed() || up.isPressed() || down.isPressed() ||
                   left.isPressed() || right.isPressed()) {
              delay(10);
            }

            currentPlayer.status = "ready";
            simonPlayers.push_back(currentPlayer);

            simonStartNewGame(); // Start a new game

          } else {
            simon_game_state = SIMON_MULTIPLAYER_SELECTION;
            ConnectionScreen::showMessage("Connection failed.\nTry again.");
            delay(1000);
            drawSimonHomeScreen();
          }
        } else {
          pad.numPadSetup();
          simon_game_state = SIMON_BLUETOOTH_NUMPAD;
          break;
        }
      } else if (up.wasJustPressed()) {
        simon_game_state = SIMON_HOMESCREEN;
        simonsubselection = 0;
        simonSelection = 1;
        drawSimonHomeSelection();
      }
      lastButtonPressTime = millis();
    }
    break;

  case SIMON_STATE_WATCH:
    // update peripheral data
    if (strcmp(multiplayerMode.c_str(), "PERIPHERAL") == 0)
      BluetoothManager::getPeripheral().update();

    if (update) {
      drawSimonGameScreen();   // Redraw disk
      drawSimonScore();        // Show updated score
      drawPlayerStatusTable(); // Draw updated check marks
      update = false;
    }

    if (millis() - lastSequenceTime > sequenceDisplayDelay) {
      simonPlaySequence();
    }
    break;

  case SIMON_STATE_PLAY:
    if (strcmp(multiplayerMode.c_str(), "PERIPHERAL") == 0)
      BluetoothManager::getPeripheral().update();
    if (update) {
      drawSimonGameScreen();   // Redraw disk
      drawSimonScore();        // Show updated score
      drawPlayerStatusTable(); // Draw updated check marks
      update = false;
    }
    for (int i = 0; i < 4; i++) {
      if (simonButtons[i]->wasJustPressed()) {
        lastButtonPressTime = millis();
        highlightSimonButton(i);
        simonCheckInput(i);
        break;
      }
    }
    break;

  case SIMON_GAMEOVER_SCREEN: {
    // ENDSCREEN HANDLING
    std::vector<String> playerNames = {settings.name};
    std::vector<int> playerScores = {playerScore};

    EndScreen endScreen(playerNames, playerScores, multiplayer, settings.name,
                        playerScore);
    if (endScreen.handleUserInput()) {
      simonStartNewGame(); // handleUserInput returns true : game restarts
    } else {
      if (endScreen.exit) { // exit to menu
        return;
      }
      simon_game_state = SIMON_HOMESCREEN;
      drawSimonHomeScreen(); // handleUserInput returns false : returns to game
                             // menu
    }
    break;
  }
  case SIMON_LEVELUP:
    if (millis() - levelUpTime > 200) {
      simon_game_state = SIMON_STATE_WATCH;
      lastSequenceTime = millis();
    }
    break;

  case SIMON_BLUETOOTH_NUMPAD:
    pad.handleButtonInput(&lastButtonPressTime, buttonDebounceDelay / 2);
    std::string enteredCode = pad.getCode();

    if (enteredCode.length() == 6 && pad.wasEnterPressed()) {

      // JOIN = PERIPHERAL
      BluetoothManager::initPeripheral(tft);
      BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
      peripheral.beginAdvertising(enteredCode);

      pad.clearCode();
      delay(1000);

      multiplayerMode = "PERIPHERAL";

      currentPlayer.status = "ready";
      String ready = generateSimonString(String("status"));

      BluetoothManager::getPeripheral().sendAction(ready.c_str());

      simonStartNewGame();
    }
    break;
  }
}

void simonStartNewGame() {
  if (!multiplayer) {
    playerScore = 0;
  }
  playerLevels[0] = 0;
  playerFailed = false;

  sequenceLength = 1;
  playerPos = 0;
  tft.fillScreen(TFT_BLACK);

  simonGenerateSequence();

  first = true; // This makes the sprites not to shift IMPORTANT!!!

  // Draws the disk
  drawSimonGameScreen();

  // Print Score and highscore
  drawSimonScore();
  drawHighscore();

  // Player Status
  drawPlayerStatusTable();

  // Reset sequence playback state
  currentStep = 0;
  showing = false;
  lastStepTime = millis() - 1000;

  simon_game_state = SIMON_STATE_WATCH;
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
    simon_game_state = SIMON_STATE_PLAY;
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
    drawSimonScore(); // Update display

    playerPos++;
    if (playerPos == sequenceLength) {
      simonLevelUp(); // Full pattern matched
    }
  } else {
    // Send eliminated status
    if (strcmp(multiplayerMode.c_str(), "PERIPHERAL") == 0) {
      currentPlayer.status = "eliminated";
      BluetoothManager::getPeripheral().sendAction(
          generateSimonString(String("status")).c_str());
    }

    playerFailed = true;     // Mark failure
    drawPlayerStatusTable(); // Shows table
    delay(400);              // Let player visualize the table

    if (playerScore > highscore) {
      // Add new highscore
      highscore = playerScore;
      File file = SD.open("/simon/highscore.txt", FILE_WRITE);
      if (file) {
        file.println(highscore);
        file.close();
      }
    }

    simonGameOver();
  }
}

void simonLevelUp() {
  // UPDATES PLAYER SCORE HERE
  playerScore++; // Count 1 point for this correct input

  // Add to score if peripheral
  if (strcmp(multiplayerMode.c_str(), "PERIPHERAL") == 0) {
    BluetoothManager::getPeripheral().sendAction(
        generateSimonString(String("input")).c_str());
  } else if (strcmp(multiplayerMode.c_str(), "HOST") == 0) {
    for (auto &p : simonPlayers) {
      if (p.id == currentPlayer.id) {
        p.score = playerScore;
        break;
      }
    }
    BluetoothCentral &central = BluetoothManager::getCentral();
    String confirmedState = generateSimonString();
    for (auto *client : central.getConnectedClients()) {
      central.sendToDevice(client, confirmedState.c_str());
    }
  }

  playerLevels[0]++; // Only P1 for now

  simonExtendSequence(); // Add one new triangle to the sequence
  playerPos = 0;

  simon_game_state = SIMON_LEVELUP;
  levelUpTime = millis();

  drawSimonGameScreen();   // Redraw disk
  drawSimonScore();        // Show updated score
  drawPlayerStatusTable(); // Draw updated check marks
}

void simonGameOver() {

  // ================= Badge Unlock Logic =================
  if (multiplayerMode.equals("SINGLE")) {
    if (playerScore >= 2 && !badgeProgress[3]) {
      badgeProgress[3] = true;
      isUnlocked[3] = true;
      saveBadgeProgress();
      checkFinalBadgeUnlock();

      hasPendingNotification = true;
      pendingNotificationMessage = "Simon Badge Unlocked!";
      pendingNotificationDuration = 3000;
    }
  }

  // Gameover audio
  playGameOverSound();

  // draws gameover screen
  simon_game_state = SIMON_GAMEOVER_SCREEN;
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

void highlightSimonButton(int buttonId) {
  // Draw triangle sprite overlay
  drawSimonTriangleOverlay(buttonId);

  // Play tone
  playTone(440 + buttonId * 100, volume);
  // Keep highlight visible briefly
  delay(200);
  playTone(0, 0); // Turn tone off

  // Clear by redrawing full game screen (disk + score, etc.)
  drawSimonGameScreen();

  delay(100); // Short pause before resuming logic
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

    if (simon_game_state == SIMON_MULTIPLAYER_SELECTION) {
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
  if (multiplayerMode.equals("SINGLE") || multiplayerMode.equals("NONE")) {
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
  } else {
    const int startX = diskSize + 60; // Right of the disk
    int startY = 5;                   // Initial vertical position
    const int minBoxWidth = 100;      // Minimum width
    const int boxHeight = 50;
    const int padding = 8;
    const int spacing = 10;

    for (size_t i = 0; i < simonPlayers.size(); ++i) {
      const SimonPlayer &player = simonPlayers[i];

      // Set text settings before measuring
      tft.setTextSize(2);
      tft.setTextDatum(TL_DATUM);

      // Measure text width
      int nameWidth = tft.textWidth(player.name);
      int scoreWidth = tft.textWidth(String(player.score));

      // Calculate dynamic box width
      int contentWidth = max(nameWidth, scoreWidth);
      int boxWidth = max(minBoxWidth, contentWidth + 2 * padding);

      // Draw box
      tft.fillRoundRect(startX, startY, boxWidth, boxHeight, 8, TFT_BLACK);
      tft.drawRoundRect(startX, startY, boxWidth, boxHeight, 8, TFT_YELLOW);

      // Draw name and score
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString(player.name, startX + padding, startY + padding);
      tft.drawString(String(player.score), startX + padding,
                     startY + padding + 22);

      // Move down for the next player
      startY += boxHeight + spacing;
    }
  }
}

// ================== AUDIO ===================== //

static void playGameOverSound() {
  int duration = 200; // milliseconds

  playTone(880, volume); // A5
  delay(duration);
  playTone(660, volume); // E5
  delay(duration);
  playTone(440, volume); // A4
  delay(duration);

  playTone(0, 0); // stop tone
}

// ============ HIGHSCORE and SCORE DRAWING =============== //
static void drawHighscore() {
  File file = SD.open("/simon/highscore.txt", "r");

  if (file) {
    highscore = file.parseInt();
    file.close();
  }

  const int scoreX = 5;                  // Left margin
  const int scoreY = SCREEN_HEIGHT - 30; // Bottom of screen
  const int padding = 4;

  // Set text properties
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM); // Top-left corner

  tft.setTextDatum(TL_DATUM);
  tft.drawString("Highscore: " + String(highscore), scoreX + padding, scoreY);
}

void drawSimonScore() {
  // Set text properties
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM); // Top-left corner

  // Draw score string
  String scoreText = "Score: " + String(playerScore);
  tft.drawString(scoreText, 10, 10);
}

// ========== Generate Simon State String ========== //
String generateSimonString(String mode) {
  if (mode == "full") {
    String state = "s@state:";
    for (size_t i = 0; i < simonPlayers.size(); ++i) {
      const SimonPlayer &p = simonPlayers[i];
      state +=
          String(p.id) + ":" + p.name + ":" + String(p.score) + ":" + p.status;
      if (i < simonPlayers.size() - 1)
        state += ",";
    }
    Serial.println("📤 Generated Simon State: " + state);
    return state;
  } else if (mode == "input") {
    String state = "s@input:" + String(currentPlayer.id);
    Serial.println("📤 Generated Simon Input: " + state);
    return state;
  } else if (mode == "status") {
    String state = "s@status:" + String(currentPlayer.id) + ":" +
                   currentPlayer.name + ":" + currentPlayer.status;
    Serial.println("📤 Generated Simon Status: " + state);
    return state;
  } else {
    Serial.println("⚠️ Invalid mode for generateSimonString(): " + mode);
    return "";
  }
}

// ========== Parse Simon State String ========== //
void readSimonString(String oldState, const char *data) {
  if (data == nullptr) {
    Serial.println("⚠️ Null data received in readSimonString");
    return;
  }

  String input = String(data);
  input.trim();

  if (input.length() == 0) {
    Serial.println("⚠️ Empty input received in readSimonString");
    return;
  }

  if (input.startsWith("s@state:")) {
    input.replace("s@state:", "");
    std::vector<SimonPlayer> newPlayers;

    if (input.length() == 0) {
      // Empty state - clear all players
      simonPlayers = newPlayers;
      Serial.println("✅ Cleared all players (empty state).");
      return;
    }

    int start = 0;
    while (start < (int)input.length()) {
      int end = input.indexOf(',', start);
      if (end == -1)
        end = input.length();

      String part = input.substring(start, end);
      part.trim(); // Remove any whitespace

      if (part.length() == 0) {
        start = end + 1;
        continue;
      }

      int first = part.indexOf(':');
      int second = part.indexOf(':', first + 1);
      int third = part.indexOf(':', second + 1);

      if (first > 0 && second > first && third > second) {
        String idStr = part.substring(0, first);
        String name = part.substring(first + 1, second);
        String scoreStr = part.substring(second + 1, third);
        String status = part.substring(third + 1);

        // Validate numeric conversions
        int id = idStr.toInt();
        int score = scoreStr.toInt();

        // Basic validation
        if (id > 0 && name.length() > 0) {
          newPlayers.push_back(SimonPlayer(id, name, score, status));
        } else {
          Serial.println("⚠️ Invalid player data: " + part);
        }
      } else {
        Serial.println("⚠️ Malformed player data: " + part);
      }
      start = end + 1;
    }

    simonPlayers = newPlayers;
    Serial.println("✅ Processed full state update with " +
                   String(newPlayers.size()) + " players.");
    update = true;
  } else if (input.startsWith("s@input:")) {
    input.replace("s@input:", "");
    int id = input.toInt();

    if (id <= 0) {
      Serial.println("⚠️ Invalid player ID in input: " + input);
      return;
    }

    bool found = false;
    for (auto &p : simonPlayers) {
      if (p.id == id) {
        p.score += 1;
        Serial.printf("✅ Player %d input processed: new score = %d\n", id,
                      p.score);
        found = true;
        break;
      }
    }

    if (!found) {
      Serial.printf("⚠️ Player %d not found for input processing\n", id);
    }
    update = true;
  } else if (input.startsWith("s@status:")) {
    input.replace("s@status:", "");
    int first = input.indexOf(':');
    int second = input.indexOf(':', first + 1);

    if (first != -1 && second != -1) {
      String idStr = input.substring(0, first);
      String name = input.substring(first + 1, second);
      String status = input.substring(second + 1);

      int id = idStr.toInt();

      if (id <= 0 || name.length() == 0) {
        Serial.println("⚠️ Invalid status data: " + input);
        return;
      }

      bool found = false;
      for (auto &p : simonPlayers) {
        if (p.id == id) {
          p.status = status;
          p.name = name;
          Serial.printf("✅ Player %d (%s) status updated to %s\n", id,
                        name.c_str(), status.c_str());
          found = true;
          break;
        }
      }

      if (!found) {
        // New player joining
        SimonPlayer newPlayer(id, name, 0, status);
        simonPlayers.push_back(newPlayer);
        Serial.printf("👤 New player added: ID=%d, Name=%s, Status=%s\n", id,
                      name.c_str(), status.c_str());
      }
      update = true;
    } else {
      Serial.println("⚠️ Malformed status string: " + input);
    }
  } else {
    Serial.println("⚠️ Unrecognized Simon string format: " + input);
  }
}