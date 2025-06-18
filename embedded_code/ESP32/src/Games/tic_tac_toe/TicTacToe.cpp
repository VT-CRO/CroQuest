// src/Games/tic_tac_toe/TicTacToe.cpp

#include "TicTacToe.hpp"
#include <unordered_map>

// ####################################################################################################
//  Functions Declarations
// ####################################################################################################

// ========== Drawing ========== //
void drawScoreboard();
void drawWinnerMessage();
void drawWinLine();
void drawGrid();
void drawAllPlaying();
void drawHomeScreen();
void drawHomescreenSelect();

// ========== Sound ========== //
void playMoveSound();
void playWinSound();
void playErrorSound();

// ========== Logic ========== //
void checkWinner();
void clearCursor(int index);
void highlightCursor(int index);
int findBestMove(char aiSymbol, char playerSymbol);
void resetBoardState(bool clearScreen);
void resetToSinglePlayerDefaults();
void resetMultiplayerState(bool clearScreen);

// Handle ready message
void handleReadyMessage(const std::string& message); 


// ========== Game States ========== //
enum State {
  HOMESCREEN,
  MULTIPLAYER,
  SINGLE_PLAYER,
  GAMEOVER_SCREEN,
  BLUETOOTH_NUMPAD,
  MULTIPLAYER_SELECTION,
  MULTIPLAYER_PLAYING,
  JOIN_SCREEN,
  HOST_SCREEN,
};

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

// Speaker Pin
#define SPEAKER_PIN 21

// Badge
TicTacToeSession session;

// Logic
bool multiplayerMode = false;
static bool firstFrame = true;
bool ticTacToeStateChanged = false;

int lastCursor = -1;

// Bluetooth Turns
char localPlayerSymbol = 'X'; // default for host

static String remotePlayerName = "Player 2"; // default fallback

// Assets
const char *BOARD_PATH = "/tic_tac_toe_assets/board.jpg";
const char *X_PATH = "/tic_tac_toe_assets/x.jpg";
const char *O_PATH = "/tic_tac_toe_assets/o.jpg";
const char *DIS_O_PATH = "/tic_tac_toe_assets/disappearing_o.jpg";
const char *DIS_X_PATH = "/tic_tac_toe_assets/disappearing_x.jpg";

// Game Board
String board[9] = {"**", "**", "**", "**", "**", "**", "**", "**", "**"};
Move moveQueue[6];
static int moveCount = 0;
static int cursorIndex = 0;
static char currentPlayer = 'X';
static char winner = 'N';
int winCombo[3] = {-1, -1, -1};
bool roundEnded = false;
unsigned long winTime = 0;

// Game State
State game_state = HOMESCREEN;
State prev_game_state = HOMESCREEN;
int selection = 0;
int subselection = 0;
const unsigned long moveDelay = 100;
bool buttonPreviouslyPressed = false;

// Game Menu
static int prevSelection = -1;
static int prevSubselection = -1;
static State prevGameState = HOMESCREEN;

// Screen
int screen_width, screen_height;
const int cell_size = 80;
int x_start, y_start;
uint16_t orange_color = tft.color565(0xFF, 0x70, 0x00);

// Score
int xWins = 0;
int oWins = 0;

std::unordered_map<std::string, bool> ready = {{"host", false}, {"periph", false}};

// Numpad Setup
static NumPad<State> pad(drawHomeScreen, drawAllPlaying, &game_state,
                         HOMESCREEN, SINGLE_PLAYER);

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runTicTacToe() {

  resetExitFlag(); // Resets flag for Main Menu
  game_state = HOMESCREEN;
  prev_game_state = HOMESCREEN;

  tft.fillScreen(orange_color);

  pinMode(SPEAKER_PIN, OUTPUT);

  screen_width = tft.width();
  screen_height = tft.height();

  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(BOARD_PATH);
  x_start = (screen_width - dim.width) / 2;
  y_start = (screen_height - dim.height) / 2;

  // delete drawing sprite
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  // reset game
  for (int i = 0; i < 9; i++)
    board[i] = "**";
  currentPlayer = 'X';
  cursorIndex = 0;
  winner = 'N';
  winCombo[0] = winCombo[1] = winCombo[2] = -1;
  roundEnded = false;
  moveCount = 0;

  // Reset UI tracking
  firstFrame = true;
  selection = 0;
  subselection = 0;
  prevSelection = -1;
  prevSubselection = -1;
  prevGameState = static_cast<State>(-1);

  drawHomeScreen();
  ready["periph"] = false;
  ready["host"] = false;


  while (true) {
    handleTicTacToeFrame();

    if (getExitFlag()) {
      BluetoothManager::reset();
      return;
    }

    // keep support for exiting with B from homescreen
    if (game_state == HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu");
      delay(500);
      BluetoothManager::reset();
      return;
    }
  }
}

// ========== Manual Loop ========== //
void handleTicTacToeFrame() {

  static int lastCursor = -1;
  static unsigned long lastMoveTime = 0;

  // Check if the Start Button was pressed and goes back to Main Menu
  if (checkStartButtonAndExit(tft))
    return;

  // ================== HOMESCREEN State =================== //
  if (game_state == HOMESCREEN) {
    if (millis() - lastMoveTime > moveDelay / 2) {
      if (A.wasJustPressed()) {
        if (selection == 0) {
          resetMultiplayerState(true);

          game_state = SINGLE_PLAYER;
          firstFrame = true;

        } else if (selection == 1) {
          resetMultiplayerState(true);
          firstFrame = true;

          game_state = MULTIPLAYER_SELECTION;
          drawHomescreenSelect();
        }
      }
      // Selection logic
      if (up.isPressed()) {
        selection = 0;
        drawHomescreenSelect();

      } else if (down.isPressed()) {
        selection = 1;
        drawHomescreenSelect();
      }
      lastMoveTime = millis();
    }
  }

  // ================== MULTIPLAYER_SELECTION State =================== //
  else if (game_state == MULTIPLAYER_SELECTION) {
    if (!roundEnded && millis() - lastMoveTime > moveDelay) {
      if (A.wasJustPressed()) {
        //clear ready status
        ready["periph"] = false;
        ready["host"] = false;
        if (subselection == 0) {

          // HOST = CENTRAL
          BluetoothManager::initCentral(tft);
          BluetoothCentral &central = BluetoothManager::getCentral();

          std::string code = generate6DigitCode();

          // Set the screen for HostGame
          HostGame::init(tft);

          // Now safely show code
          HostGame::showCode(String(code.c_str()));

          // Check if user exited
          if (getExitFlag()) {
            resetExitFlag();
            game_state = HOMESCREEN; // TODO: This does not work completely well
            return;
          }

          central.scanAndConnectLoop(code);

          multiplayerMode = true;

          if (!BluetoothManager::getCentral().getConnectedClients().empty()) {
            localPlayerSymbol = 'X';

            game_state = HOST_SCREEN;

            // Flush any held buttons to prevent input carryover
            delay(300); // debounce delay
            while (A.isPressed() || up.isPressed() || down.isPressed() ||
                   left.isPressed() || right.isPressed()) {
              delay(10);
            }

            //send ready string
            ready["host"] = true;
            BluetoothManager::getCentral().sendMessage("ready@host,true");

          } else {
            game_state = MULTIPLAYER_SELECTION;
            tft.fillScreen(orange_color);
            drawHomeScreen();
            ConnectionScreen::showMessage("Connection failed.\nTry again.");
          }

        } else {
          game_state = BLUETOOTH_NUMPAD;
          pad.numPadSetup();
        }
      }

      if (up.isPressed()) {
        game_state = HOMESCREEN;
        drawHomescreenSelect();
      } else if (left.isPressed()) {
        if (subselection == 1) {
          subselection = 0;
          drawHomescreenSelect();
        }
      } else if (right.isPressed()) {
        if (subselection == 0) {
          subselection = 1;
          drawHomescreenSelect();
        }
      }

      lastMoveTime = millis();
    }
  }

  // ================== HOST_SCREEN State =================== //
  else if (game_state == HOST_SCREEN) {
    // Don't continue if the host or peripheral aren't ready yet
    if(!ready["host"] || !ready["periph"])
      return;

    BluetoothCentral &central = BluetoothManager::getCentral();
  }

  // ================== MULTIPLAYER_PLAYING State =================== //
  else if (game_state == MULTIPLAYER_PLAYING) {
    // Don't continue if the host or peripheral aren't ready yet
    if(!ready["host"] || !ready["periph"])
      return;

    BluetoothManager::getPeripheral().update();
  }

  // ========== Multiplayer Logic for Host/Peripheral ========== //
  if (game_state == HOST_SCREEN || game_state == MULTIPLAYER_PLAYING) {

    if (firstFrame) {
      tft.fillScreen(orange_color);
      drawAllPlaying();
      firstFrame = false;
    }

    // Draw if Bluetooth state just changed
    if (ticTacToeStateChanged) {
      drawAllPlaying();
      drawWinLine();
      if (roundEnded) {
        playWinSound();
        drawWinnerMessage();
      }
      lastCursor = cursorIndex;
      ticTacToeStateChanged = false;
    }

    // Turn Indicator
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, orange_color);

    if (currentPlayer == 'O' && game_state == HOST_SCREEN ||
        currentPlayer == 'X' && game_state == MULTIPLAYER_PLAYING) {
      tft.drawString("Waiting...", tft.width() / 2, tft.height() - 10);
    } else if (currentPlayer == 'X' && game_state == HOST_SCREEN) {
      tft.drawString(" X's Turn ", tft.width() / 2, tft.height() - 10);
    } else if (currentPlayer == 'O' && game_state == MULTIPLAYER_PLAYING) {
      tft.drawString(" O's Turn  ", tft.width() / 2, tft.height() - 10);
    }

    // Handle movement
    if (!roundEnded && millis() - lastMoveTime > moveDelay / 2) {
      if (up.isPressed() && cursorIndex >= 3) {
        cursorIndex -= 3;
        lastMoveTime = millis();
      } else if (down.isPressed() && cursorIndex <= 5) {
        cursorIndex += 3;
        lastMoveTime = millis();
      } else if (left.isPressed() && cursorIndex % 3 != 0) {
        cursorIndex -= 1;
        lastMoveTime = millis();
      } else if (right.isPressed() && cursorIndex % 3 != 2) {
        cursorIndex += 1;
        lastMoveTime = millis();
      }
    }

    // Handle Selection
    bool selectPressed = A.wasJustPressed();

    if (localPlayerSymbol == currentPlayer) {
      Serial.printf("⏳ Not your turn. You are '%c' and it's '%c'\n",
                    localPlayerSymbol, currentPlayer);

      // All move/selection logic
      if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
          board[cursorIndex] == "**") {
        if (moveCount >= 6) {
          int oldIndex = moveQueue[0].index;
          board[oldIndex] = "**";
          for (int i = 1; i < 6; i++)
            moveQueue[i - 1] = moveQueue[i];
          moveCount = 5;
        }

        playMoveSound();

        board[cursorIndex] = String(currentPlayer);
        moveQueue[moveCount].index = cursorIndex;
        moveQueue[moveCount].symbol = currentPlayer;
        moveCount++;

        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
        checkWinner();

        drawAllPlaying();
        drawWinLine();
        if (roundEnded) {
          playWinSound();
          drawWinnerMessage();
        }

        // After move, send state
        String newState = generateTicTacToeStateString();

        Serial.println("HOST SENDING: ");
        Serial.println(newState);

        if (game_state == HOST_SCREEN) {
          BluetoothCentral &central = BluetoothManager::getCentral();
          for (auto *client : central.getConnectedClients()) {
            if (!central.sendToDevice(client, newState.c_str())) {
              Serial.println("❌ Failed to notify client. Disconnecting...");
              ConnectionScreen::showMessage("Peripheral disconnected");
              shouldExitToMenu = true;
              return;
            }
          }
        } else if (game_state == MULTIPLAYER_PLAYING) {
          BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
          if (!peripheral.sendAction(newState.c_str())) {
            ConnectionScreen::showMessage("Disconnected (Send Failed)");
            shouldExitToMenu = true;
            return;
          }
        }

        delay(400);
      } else if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
                 board[cursorIndex] != "**") {
        playErrorSound();
      }

      buttonPreviouslyPressed = selectPressed;

      // Redraw when moving or selecting
      if (cursorIndex != lastCursor || selectPressed) {
        drawAllPlaying();
        drawWinLine();
        if (roundEnded) {
          playWinSound();
          drawWinnerMessage();
        }
        lastCursor = cursorIndex;
      }
    }

    // Auto Restart with Bluetooth sync
    if (roundEnded && millis() - winTime >= 5000 && xWins < 2 && oWins < 2) {

      resetBoardState(true);
      drawAllPlaying();

    } else if (xWins >= 2 || oWins >= 2) {
      prev_game_state = game_state;
      game_state = GAMEOVER_SCREEN;
      // clear ready status
      ready["host"] = false;
      ready["periph"] = false;
      resetBoardState(true);
    }
  }

  // ================== SINGLE_PLAYER State =================== //
  else if (game_state == SINGLE_PLAYER) {

    if (firstFrame) {
      resetToSinglePlayerDefaults();
      resetBoardState(true);
      drawAllPlaying();
      firstFrame = false;
    }

    if (!roundEnded && millis() - lastMoveTime > moveDelay / 2) {
      if (up.isPressed() && cursorIndex >= 3) {
        cursorIndex -= 3;
        lastMoveTime = millis();
      } else if (down.isPressed() && cursorIndex <= 5) {
        cursorIndex += 3;
        lastMoveTime = millis();
      } else if (left.isPressed() && cursorIndex % 3 != 0) {
        cursorIndex -= 1;
        lastMoveTime = millis();
      } else if (right.isPressed() && cursorIndex % 3 != 2) {
        cursorIndex += 1;
        lastMoveTime = millis();
      }
    }

    // Piece Placement
    bool selectPressed = A.wasJustPressed();

    if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
        board[cursorIndex] == "**") {
      if (moveCount >= 6) {
        int oldIndex = moveQueue[0].index;
        board[oldIndex] = "**";
        for (int i = 1; i < 6; i++)
          moveQueue[i - 1] = moveQueue[i];
        moveCount = 5;
      }

      playMoveSound();

      board[cursorIndex] = String(currentPlayer);
      moveQueue[moveCount].index = cursorIndex;
      moveQueue[moveCount].symbol = currentPlayer;
      moveCount++;

      currentPlayer = 'O';
      checkWinner();

      // Player's move render before AI thinks
      drawAllPlaying();
      drawWinLine();

      if (roundEnded) {
        playWinSound();
        drawWinnerMessage();
      }
      delay(100);
    } else if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
               board[cursorIndex] != "**") {
      playErrorSound(); // Tried to press an occupied tile
    }

    if (currentPlayer == 'O' && !roundEnded) {
      delay(50); // Optional: makes AI feel more human

      int aiMove = findBestMove('O', 'X');
      if (aiMove != -1) {
        if (moveCount >= 6) {
          int oldIndex = moveQueue[0].index;
          board[oldIndex] = "**";
          for (int i = 1; i < 6; i++)
            moveQueue[i - 1] = moveQueue[i];
          moveCount = 5;
        }

        board[aiMove] = "O";
        moveQueue[moveCount].index = aiMove;
        moveQueue[moveCount].symbol = 'O';
        moveCount++;

        currentPlayer = 'X';
        checkWinner();

        drawAllPlaying();
        drawWinLine();
        playWinSound();

        if (roundEnded) {
          playWinSound();
          drawWinnerMessage();
        }
      }
    }

    buttonPreviouslyPressed = selectPressed;

    // Redraw
    if (cursorIndex != lastCursor || selectPressed) {
      drawAllPlaying();
      drawWinLine();

      if (roundEnded) {
        playWinSound();
        drawWinnerMessage();
      }
      lastCursor = cursorIndex;
    }

    // Auto Restart
    if (roundEnded && millis() - winTime >= 3000 && xWins < 2 && oWins < 2) {
      resetBoardState(true);
      drawAllPlaying();

    } else if (xWins >= 2 || oWins >= 2) {
      game_state = GAMEOVER_SCREEN;
      resetBoardState(true);
      // Clear the screen with orange background
      tft.fillScreen(orange_color);
    }
  }

  // ================== BLUETOOTH_NUMPAD State =================== //
  else if (game_state == BLUETOOTH_NUMPAD) {
    pad.handleButtonInput(&lastMoveTime, moveDelay);
    Serial.println("Testing Crash");

    std::string enteredCode = pad.getCode();
    if (enteredCode.length() == 6 && pad.wasEnterPressed()) {
      // JOIN = PERIPHERAL
      BluetoothManager::initPeripheral(tft);
      BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
      peripheral.beginAdvertising(enteredCode);
      localPlayerSymbol = 'O';

      pad.clearCode();

      ready["periph"] = true;
    }

    // Only start the game when both the host and peripheral are ready
    if(ready["periph"] && ready["host"]){
      BluetoothManager::getPeripheral().sendMessage("ready@periph,true"); // send ready message
      game_state = MULTIPLAYER_PLAYING;
    }
  }

  // ================== GAMEOVER_SCREEN State =================== //
  else if (game_state == GAMEOVER_SCREEN) {
    std::vector<String> playerNames;
    std::vector<int> playerScores;

    bool multiplayer = multiplayerMode;

    if (multiplayer) {
      playerNames = {settings.name};
      playerScores = {(localPlayerSymbol == 'X') ? xWins : oWins};

      String opponentName = remotePlayerName;
      int opponentScore = (localPlayerSymbol == 'X') ? oWins : xWins;

      playerNames.push_back(opponentName);
      playerScores.push_back(opponentScore);

    } else {
      playerNames = {"AI", settings.name};
      playerScores = {oWins, xWins};
    }

    char localNameBuffer[6];
    strncpy(localNameBuffer, settings.name, sizeof(localNameBuffer) - 1);
    localNameBuffer[sizeof(localNameBuffer) - 1] = '\0';

    int playerScore = (localPlayerSymbol == 'X') ? xWins : oWins;

    EndScreen endScreen(playerNames, playerScores, multiplayer, localNameBuffer,
                        playerScore);

    if (endScreen.handleUserInput()) {

      // ==== Reset score ==== //
      xWins = 0;
      oWins = 0;

      // ---- Restart game ---- //
      if (multiplayer) {
        resetMultiplayerState(false); // false = don't clear screen again

      } else {
        resetToSinglePlayerDefaults(); // Clear AI/game state
      }

      resetBoardState(true);
      roundEnded = false;
      winner = 'N';
      winCombo[0] = winCombo[1] = winCombo[2] = -1;

      if (multiplayer) {
        game_state = prev_game_state;
      } else {
        game_state = SINGLE_PLAYER;
      }

      // set ready status
      if(game_state == MULTIPLAYER_PLAYING){
        ready["periph"] = true;
        BluetoothManager::getPeripheral().sendMessage("ready@periph,true");
      }else if(game_state == HOST_SCREEN){
        ready["host"] = true;
        BluetoothManager::getCentral().sendMessage("ready@host,true");
      }

      // Show intermediary waiting screen
      if(game_state == MULTIPLAYER_PLAYING || game_state == HOST_SCREEN){
        if(!ready["periph"] || !ready["host"]){
            tft.fillScreen(TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("WAITING FOR OTHER PLAYER...", tft.width()/2, tft.height()/2);
            tft.setTextDatum(TL_DATUM);
        }
      }

      prev_game_state = game_state;

      // ==== Immediately redraw everything like the start of a game ====
      // drawGrid();
      // drawAllPlaying();
      // drawScoreboard();
      // highlightCursor(cursorIndex);

      firstFrame = true;
      return;

    } else if (endScreen.exit) {
      return;
    }

    // Exit to game menu
    xWins = 0;
    oWins = 0;

    BluetoothManager::reset(false);

    selection = 0;
    subselection = 0;
    prevSelection = -1;
    prevSubselection = -1;
    prevGameState = static_cast<State>(-1);

    game_state = HOMESCREEN;
    drawHomeScreen();
  }
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Check Winner ========== //
void checkWinner() {
  const int wins[8][3] = {
      {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
      {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // cols
      {0, 4, 8}, {2, 4, 6}             // diagonals
  };

  for (int i = 0; i < 8; i++) {
    String a = board[wins[i][0]];
    String b = board[wins[i][1]];
    String c = board[wins[i][2]];

    if (a != "**" && a == b && b == c) {
      winner = a.charAt(0);
      winCombo[0] = wins[i][0];
      winCombo[1] = wins[i][1];
      winCombo[2] = wins[i][2];
      winTime = millis();
      roundEnded = true;

      if (winner == 'X')
        xWins++;
      else if (winner == 'O')
        oWins++;

      // ================= Badge Unlock Logic =================
      if (!multiplayerMode) {
        // Perfect game: X wins 2-0
        if (xWins == 2 && oWins == 0) {
          session.consecutiveWins++;
        } else if (oWins == 2) {
          session.consecutiveWins = 0;
        }

        if (session.consecutiveWins >= 3 && !badgeProgress[2] &&
            !session.badgeUnlocked) {
          badgeProgress[2] = true;
          isUnlocked[2] = true;
          saveBadgeProgress();
          session.badgeUnlocked = true;

          hasPendingNotification = true;
          pendingNotificationMessage = "Tic Tac Toe Badge Unlocked!";
          pendingNotificationDuration = 3000;
        }
        checkFinalBadgeUnlock();
      }
      return;
    }
  }

  // Check draw
  bool full = true;
  for (int i = 0; i < 9; i++) {
    if (board[i] == "**") {
      full = false;
      break;
    }
  }
  if (full) {
    winner = 'D';
    winTime = millis();
    roundEnded = true;
  }
}

// ========== Selector ========== //
void highlightCursor(int index) {

  if (index < 0 || index > 8) {
    Serial.printf("⚠️ Invalid cursor index: %d\n", index);
    return;
  }

  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size + cell_size / 3 - 3;
  int y = y_start + row * cell_size + cell_size / 3 - 3;

  tft.drawRect(x, y, cell_size - 30, cell_size - 30, TFT_WHITE);
}

// ========== AI Moves ========== //
int findBestMove(char aiSymbol, char playerSymbol) {
  // Winning combinations
  const int wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                          {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

  // 1. Try to win
  for (const auto &combo : wins) {
    int countAI = 0, empty = -1;
    for (int idx : combo) {
      if (board[idx] == String(aiSymbol))
        countAI++;
      else if (board[idx] == "**")
        empty = idx;
    }
    if (countAI == 2 && empty != -1)
      return empty;
  }

  // 2. Block opponent
  for (const auto &combo : wins) {
    int countPlayer = 0, empty = -1;
    for (int idx : combo) {
      if (board[idx] == String(playerSymbol))
        countPlayer++;
      else if (board[idx] == "**")
        empty = idx;
    }
    if (countPlayer == 2 && empty != -1)
      return empty;
  }

  // 3. Take center
  if (board[4] == "**")
    return 4;

  // 4. Take a corner
  for (int i : {0, 2, 6, 8}) {
    if (board[i] == "**")
      return i;
  }

  // 5. Take any side
  for (int i : {1, 3, 5, 7}) {
    if (board[i] == "**")
      return i;
  }

  // If somehow none of these work
  return -1;
}

// ========== Reset Board ========== //
void resetBoardState(bool clearScreen = true) {
  for (int i = 0; i < 9; i++)
    board[i] = "**";
  currentPlayer = 'X';
  cursorIndex = 0;
  lastCursor = -1;
  winner = 'N';
  winCombo[0] = winCombo[1] = winCombo[2] = -1;
  roundEnded = false;
  moveCount = 0;
  if (clearScreen)
    tft.fillScreen(orange_color);
}

// ========== Reset Single Player ========== //
void resetToSinglePlayerDefaults() {
  multiplayerMode = false;
  localPlayerSymbol = 'X';
  ticTacToeStateChanged = false;
  cursorIndex = 0;
  lastCursor = -1;
}

// ========== Reset Multiplayer ========== //
void resetMultiplayerState(bool clearScreen) {
  multiplayerMode = false;
  xWins = 0;
  oWins = 0;
  roundEnded = false;
  winner = 'N';
  winCombo[0] = winCombo[1] = winCombo[2] = -1;
  moveCount = 0;
  cursorIndex = 0;
  lastCursor = -1;
  currentPlayer = 'X';

  for (int i = 0; i < 9; ++i)
    board[i] = "**";

  for (int i = 0; i < 6; ++i) {
    moveQueue[i].index = -1;
    moveQueue[i].symbol = '*';
  }

  if (clearScreen)
    tft.fillScreen(orange_color);
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// ========== Draw Score Board ========== //
void drawScoreboard() {
  int centerY = tft.height() / 2;
  int padding = 20;

  // Settings for big scoreboard
  int textSize = 4;
  tft.setTextSize(textSize);
  tft.setTextDatum(MC_DATUM);

  int underlineWidth = 40;
  int underlineThickness = 4; // <== THICKNESS OF THE LINE
  int underlineOffset = 24;   // Vertical distance from text to line
  int scoreOffset = 32;       // Distance from underline to score

  // === X Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xX = padding + underlineWidth;
  int yX = centerY - (underlineOffset + scoreOffset) / 2;
  tft.drawString("X", xX, yX);

  // Thick underline using fillRect
  int xLineY = yX + underlineOffset;
  tft.fillRect(xX - underlineWidth / 2, xLineY, underlineWidth,
               underlineThickness, TFT_WHITE);

  // Score for X
  tft.drawString(String(xWins), xX, xLineY + scoreOffset);

  // === O Side ===
  tft.setTextColor(TFT_WHITE, orange_color);
  int xO = tft.width() - padding - underlineWidth;
  int yO = yX;
  tft.drawString("O", xO, yO);

  // Thick underline using fillRect
  int oLineY = yO + underlineOffset;
  tft.fillRect(xO - underlineWidth / 2, oLineY, underlineWidth,
               underlineThickness, TFT_WHITE);

  // Score for O
  tft.drawString(String(oWins), xO, oLineY + scoreOffset);
}

// ========== Draw Line Over Assets ========== //
void drawWinLine() {
  if (winner != 'X' && winner != 'O')
    return;

  int i1 = winCombo[0];
  int i3 = winCombo[2];

  // Get cell positions
  int row1 = i1 / 3, col1 = i1 % 3;
  int row3 = i3 / 3, col3 = i3 % 3;

  // Compute center points of the winning cells
  int x1 = x_start + col1 * cell_size + cell_size / 2 + 6;
  int y1 = y_start + row1 * cell_size + cell_size / 2 + 6;
  int x3 = x_start + col3 * cell_size + cell_size / 2 + 6;
  int y3 = y_start + row3 * cell_size + cell_size / 2 + 6;

  uint16_t color = (winner == 'X') ? TFT_RED : TFT_BLUE;

  // Determine if this is a backslash or slash diagonal
  bool isSlashDiagonal = (col3 - col1) * (row3 - row1) < 0;

  // Normalize thickness by keeping offset only *perpendicular* to direction
  for (int t = -2; t <= 2; t++) {
    int x1_offset = x1, x3_offset = x3;
    int y1_offset = y1, y3_offset = y3;

    if (row1 == row3) {
      // Horizontal Line
      y1_offset += t;
      y3_offset += t;
    } else if (col1 == col3) {
      // Vertical Line
      x1_offset += t;
      x3_offset += t;
    } else {
      if (isSlashDiagonal) {
        // From top-right to bottom-left
        x1_offset += t;
        y1_offset += t;
        x3_offset += t;
        y3_offset += t;
      } else {
        // From top-left to bottom-right
        x1_offset += t;
        y1_offset -= t;
        x3_offset += t;
        y3_offset -= t;
      }
    }

    tft.drawLine(x1_offset, y1_offset, x3_offset, y3_offset, color);
  }
}

// ========== Draw Winner Message ========== //
void drawWinnerMessage() {
  String msg;
  uint16_t color = TFT_WHITE;
  uint16_t bgColor = TFT_BLACK;

  // Determine message and color
  if (winner == 'X') {
    msg = "X WINS!";
    color = TFT_RED;
  } else if (winner == 'O') {
    msg = "O WINS!";
    color = TFT_BLUE;
  } else if (winner == 'D') {
    msg = "DRAW!";
    color = TFT_YELLOW;
  }

  // Draw rounded box
  int boxWidth = 180;
  int boxHeight = 50;
  int x = (tft.width() - boxWidth) / 2;
  int y = 20;

  tft.fillRoundRect(x, y, boxWidth, boxHeight, 8, orange_color);
  tft.drawRoundRect(x, y, boxWidth, boxHeight, 8, color);

  // Draw glowing text in center
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(color, orange_color);
  tft.drawString(msg, tft.width() / 2, y + boxHeight / 2);
}

// ========== Draw HomeScreen ========== //
void drawHomeScreen() {

  drawTitleAndGrid();     // Draw static background
  drawHomescreenSelect(); // Draw dynamic buttons
}

// ========== Draw HomeScreen Buttons ========== //
void drawHomescreenSelect() {
  int y_single = 200;
  int y_multi = 250;
  int y_sub = y_multi + 40;

  // Always draw title + grid once when entering this screen
  if (prevSelection == -1 || game_state != prevGameState) {
    drawTitleAndGrid();
  }

  // === Draw Buttons (always) ===
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  tft.fillRect(0, y_single - 15, screen_width, 35, orange_color);
  tft.fillRect(0, y_multi - 15, screen_width, 80, orange_color);

  tft.setTextSize(selection == 0 ? 3 : 2);
  tft.drawString("Start Single-Player", screen_width / 2, y_single);

  tft.setTextSize(selection == 1 ? 3 : 2);
  tft.drawString("Start Multiplayer", screen_width / 2, y_multi);

  // ---------- Sub-options for Multiplayer ----------
  if (game_state == MULTIPLAYER_SELECTION) {
    const char *sub1 = "Host a Game";
    const char *sub2 = "Join a Game";

    int textSize = 2;
    tft.setTextSize(textSize);
    tft.setTextDatum(MC_DATUM);

    int padding_x = 10;
    int padding_y = 2;

    int sub1Width = tft.textWidth(sub1);
    int sub2Width = tft.textWidth(sub2);

    int sub1BoxWidth = sub1Width + padding_x * 2;
    int sub2BoxWidth = sub2Width + padding_x * 2;
    int boxHeight = 16 * textSize + padding_y * 2;

    int x_sub1 = screen_width / 4;
    int x_sub2 = 3 * screen_width / 4;

    // Clear suboption area every time in this case
    tft.fillRect(0, y_sub - boxHeight / 2 - 2, screen_width, boxHeight + 10,
                 orange_color);

    // Draw highlight
    if (subselection == 0) {
      tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                   sub1BoxWidth, boxHeight, TFT_WHITE);
    } else {
      tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                   sub2BoxWidth, boxHeight, TFT_WHITE);
    }

    // Draw text
    tft.drawString(sub1, x_sub1, y_sub);
    tft.drawString(sub2, x_sub2, y_sub);
  }

  // ---------- Save state ----------
  prevSelection = selection;
  prevSubselection = subselection;
  prevGameState = game_state;
}

// ========== Draw Title and Grid ========== //
void drawTitleAndGrid() {
  // Clear the screen with orange background
  tft.fillScreen(orange_color);

  // Title
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.drawString("TIC TAC TOE", screen_width / 2, 40);

  // Grid
  int gridSize = 90;
  int cellSize = gridSize / 3;
  int gridX = (screen_width - gridSize) / 2;
  int gridY = 30;

  tft.fillRect(gridX + cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  tft.fillRect(gridX + 2 * cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  tft.fillRect(gridX, gridY + cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
  tft.fillRect(gridX, gridY + 2 * cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
}

// ========== Draw Moves ========== //
void drawAllPlaying() {

  drawScoreboard();
  drawGrid();

  drawing.pushSprite(); // Needed so the selector does not disappear on
                        // singleplayer

  bool showCursor = false;

  // Multiplayer: show cursor only if it's your turn and game is not over
  if (multiplayerMode) {
    showCursor = (localPlayerSymbol == currentPlayer && !roundEnded);
  }
  // SinglePlayer: always show cursor unless game ended
  else {
    showCursor = !roundEnded;
  }

  if (showCursor) {
    highlightCursor(cursorIndex);
  }

  drawing.pushSprite();
}

// ========== Create Grid ========== //
void drawGrid() {

  drawing.drawSdJpeg(BOARD_PATH, x_start, y_start);

  // Draw current board state
  for (int i = 0; i < 9; i++) {
    if (board[i] != "**") {
      int row = i / 3;
      int col = i % 3;
      int x = col * cell_size + cell_size / 3 - 3;
      int y = row * cell_size + cell_size / 3 - 3;

      drawing.setFirst(false); // Reset "First" before each symbol

      if (board[i] == "X") {
        if (moveCount >= 5 && moveQueue[0].index == i) {
          drawing.drawSdJpeg(DIS_X_PATH, x, y);
        } else {
          drawing.drawSdJpeg(X_PATH, x, y);
        }
      } else {
        if (moveCount >= 5 && moveQueue[0].index == i) {
          drawing.drawSdJpeg(DIS_O_PATH, x, y);
        } else {
          drawing.drawSdJpeg(O_PATH, x, y);
        }
      }
    }
  }
}

// ========== Remove Cursor ========== //
void clearCursor(int index) {
  if (index < 0 || index > 8)
    return;

  int row = index / 3;
  int col = index % 3;

  int x = x_start + col * cell_size;
  int y = y_start + row * cell_size;

  // Fully clear and redraw the tile as white
  tft.fillRect(x, y, cell_size, cell_size, TFT_WHITE);
  tft.drawRect(x, y, cell_size, cell_size, TFT_BLACK);

  // Redraw the grid lines manually if needed (for crossovers)
  if (col > 0)
    tft.drawLine(x, y, x, y + cell_size, TFT_BLACK); // left
  if (col < 2)
    tft.drawLine(x + cell_size, y, x + cell_size, y + cell_size,
                 TFT_BLACK); // right
  if (row > 0)
    tft.drawLine(x, y, x + cell_size, y, TFT_BLACK); // top
  if (row < 2)
    tft.drawLine(x, y + cell_size, x + cell_size, y + cell_size,
                 TFT_BLACK); // bottom
}

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Placing Marker ========== //
void playMoveSound() {
  tone(SPEAKER_PIN, 660, 100); // Frequency, Duration
}

// ========== Winning sound ========== //
void playWinSound() {
  tone(SPEAKER_PIN, 880, 300);
  delay(100);
  tone(SPEAKER_PIN, 990, 300);
}

// ========== Error sound ========== //
void playErrorSound() { tone(SPEAKER_PIN, 300, 300); }

// ####################################################################################################
//  Bluetooth Logic
// ####################################################################################################

// ========== Generate Tic Tac Toe State String ========== //
String generateTicTacToeStateString() {
  String state = "ttt@";

  // Use real player names
  String xName = (localPlayerSymbol == 'X') ? formatName(settings.name)
                                            : formatName(remotePlayerName);
  String oName = (localPlayerSymbol == 'O') ? formatName(settings.name)
                                            : formatName(remotePlayerName);

  state += "0,X," + String(xWins) + "," + xName + ";\n";
  state += "1,O," + String(oWins) + "," + oName + ";\n";

  // Best of condition
  state += "5;\n";

  // Whose turn
  state += String(currentPlayer) + ";\n";

  // Board state with aging info
  for (int i = 0; i < 9; ++i) {
    bool found = false;
    for (int j = 0; j < moveCount; ++j) {
      if (moveQueue[j].index == i) {
        state += moveQueue[j].symbol;
        state += j; // Age
        found = true;
        break;
      }
    }
    if (!found)
      state += "**";
    if (i < 8)
      state += ",";
  }
  state += ";\n";

  // Cursor index
  state += String(cursorIndex) + ";\n";

  // Winner + win combo
  if (winner == 'X' || winner == 'O' || winner == 'D') {
    state += String(winner) + ";" + String(winCombo[0]) + "," +
             String(winCombo[1]) + "," + String(winCombo[2]) + ";\n";
  } else {
    state += "N;\n"; // 'N' for no winner yet
  }

  return state;
}

// ========== Reads Tic Tac Toe State String ==========
void readTicTacToeString(String oldState, const char *data) {
  String input = String(data);
  input.trim();
  input.replace("ttt@", "");

  for (int i = 0; i < 9; i++)
    board[i] = "**";
  for (int i = 0; i < 6; i++) {
    moveQueue[i].index = -1;
    moveQueue[i].symbol = '*';
  }
  moveCount = 0;

  int line = 0;
  int i = 0;
  int idx = 0;

  while (idx != -1 && line < 7) {
    idx = input.indexOf(";\n", i);
    String part = input.substring(i, idx);
    i = idx + 2;

    switch (line) {
    case 0: { // Player X
      auto parts = split(part, ',');
      if (parts.size() >= 4) {
        int parsedXScore = parts[2].toInt();
        if (localPlayerSymbol == 'O') {
          xWins = parsedXScore; // Opponent's score
          remotePlayerName = parts[3];
        } else {
          // Optional: xWins = parsedXScore; // In case host needs to sync its
          // own value
        }
      }
      break;
    }

    case 1: { // Player O
      auto parts = split(part, ',');
      if (parts.size() >= 4) {
        int parsedOScore = parts[2].toInt();
        if (localPlayerSymbol == 'X') {
          oWins = parsedOScore; // Opponent's score
          remotePlayerName = parts[3];
        } else {
          // Optional: oWins = parsedOScore;
        }
      }
      break;
    }

    case 2:
      break; // Best-of
    case 3:
      currentPlayer = part.charAt(0);
      break;
    case 4: {
      int moveIndex = 0;
      for (int j = 0; j < 9; j++) {
        int comma = part.indexOf(',', moveIndex);
        String cell =
            part.substring(moveIndex, (comma == -1) ? part.length() : comma);

        if (cell != "**" && cell.length() == 2) {
          char symbol = cell.charAt(0);
          int age = cell.charAt(1) - '0';
          board[j] = String(symbol);
          moveQueue[age].index = j;
          moveQueue[age].symbol = symbol;
          if (age >= moveCount)
            moveCount = age + 1;
        } else {
          board[j] = "**";
        }

        moveIndex = comma + 1;
      }
      break;
    }
    case 5: {
      if (localPlayerSymbol != currentPlayer)
        cursorIndex = part.toInt();
      break;
    }
    case 6: {
      int sep = part.indexOf(';');
      if (sep != -1) {
        winner = part.substring(0, sep).charAt(0);
        if (winner == 'X' || winner == 'O' || winner == 'D') {
          roundEnded = true;
          winTime = millis();

          String comboStr = part.substring(sep + 1);
          int comma1 = comboStr.indexOf(',');
          int comma2 = comboStr.lastIndexOf(',');

          winCombo[0] = comboStr.substring(0, comma1).toInt();
          winCombo[1] = comboStr.substring(comma1 + 1, comma2).toInt();
          winCombo[2] = comboStr.substring(comma2 + 1).toInt();
        } else {
          winner = 'N';
          roundEnded = false;
          winCombo[0] = winCombo[1] = winCombo[2] = -1;
        }
      }
      break;
    }
    }

    line++;
  }

  multiplayerMode = true;
  ticTacToeStateChanged = true;

  if (winner == 'N')
    checkWinner();

  Serial.println("Parsed board:");
  for (int k = 0; k < 9; ++k) {
    Serial.print(board[k]);
    Serial.print(" ");
    if (k % 3 == 2)
      Serial.println();
  }

  Serial.println("Parsed moveQueue:");
  for (int k = 0; k < moveCount; ++k) {
    Serial.printf("Move %d: %c at %d\n", k, moveQueue[k].symbol,
                  moveQueue[k].index);
  }
}

// ========== Helper: Split String ========== //
std::vector<String> split(const String &s, char delimiter) {
  std::vector<String> result;
  int start = 0;
  int end = s.indexOf(delimiter);

  while (end != -1) {
    result.push_back(s.substring(start, end));
    start = end + 1;
    end = s.indexOf(delimiter, start);
  }
  result.push_back(s.substring(start));
  return result;
}

// ========== Format Name ========== //
String formatName(String name) {
  if (name.length() == 0)
    return name;
  name.toLowerCase();
  name.setCharAt(0, toupper(name.charAt(0)));
  return name;
}

// Handle the ready message
void handleReadyMessage(const std::string& message) {
    // Ensure the message starts with "ready@"
    if (message.rfind("ready@", 0) != 0) return;

    size_t at = message.find('@');
    size_t comma = message.find(',');

    if (comma == std::string::npos || at == std::string::npos || comma <= at + 1) return;

    std::string key = message.substr(at + 1, comma - (at + 1));
    std::string valueStr = message.substr(comma + 1);

    if (ready.find(key) != ready.end()) {
        ready[key] = (valueStr == "true");
        Serial.printf("✅ Ready state updated: %s => %s\n", key.c_str(), valueStr.c_str());
    } else {
        Serial.printf("⚠️ Unknown key in ready message: %s\n", key.c_str());
    }
}