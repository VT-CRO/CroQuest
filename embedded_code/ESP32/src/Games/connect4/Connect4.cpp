// src/Games/connect4/Connect4.cpp

#include "Connect4.hpp"
#include <unordered_map>

// ========== Game States ==========
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

static State game_state = HOMESCREEN;

// Logic
static bool firstFrame = true;
static bool multiplayerMode = false;
bool connect4StateChanged = false;

static int lastDropRow = -1;
static int lastDropCol = -1;

// Game Board
static unsigned long winTime = 0;
static int cursorIndex = 0;

// Bluetooth Turns
static int localPlayerId = 1; // 1 for red, 2 for blue
static int playerDrop = 1;
static int pieceDropCol = 0;

// Game Menu
static int prevSelection = -1;
static int prevSubselection = -1;
static State prevGameState = HOMESCREEN;

static int selection = 0;
static int subselection = 0;
static const unsigned long moveDelay = 100;
static bool buttonPreviouslyPressed = false;

// Badges
static int totalConnect4Wins;

const int CELL_SIZE = 30;
const int COLS = 7;
const int ROWS = 6;
const int GRID_TOP = 40;

// Playable grid area starts 15px from edge of image
const int IMAGE_X = 80; // x offset to center 320px image on 480px screen
const int IMAGE_Y = 40; // y offset to center 240px image on 320px screen

const int CELL_W = 40; // 30px circle + 10px horizontal spacing
const int CELL_H = 37; // 30px circle + 7px vertical spacing

const int PIECE_RADIUS =
    14; // 30px diameter → radius = 15, minus a little padding

static int board[ROWS][COLS] = {0}; // 0 = empty, 1 = red, 2 = yellow
static int cursorCol = 3;
static int gridOffsetX = 0;
static int gridOffsetY = 0;

static int player1Wins = 0;
static int player2Wins = 0;
static int currentPlayer = 1; // red = 1, blue = 2

static String remotePlayerName = "Player 2"; // default fallback

// These global variables are used to draw the win line
static int winStartRow, winStartCol;
static int winDirRow, winDirCol;

// Game Board
static bool roundEnded = false;

// Screen
static int screen_width, screen_height;

// Background color
uint16_t bgColor = tft.color565(32, 58, 66);

// NumPad set up
static NumPad<State> pad(drawHomeScreen, drawAllPlaying, &game_state,
                         HOMESCREEN, SINGLE_PLAYER);

extern std::unordered_map<std::string, bool> ready;

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runConnect4() {
  resetExitFlag(); // So menu knows we haven’t exited yet
  game_state = HOMESCREEN;

  tft.fillScreen(bgColor);

  player1Wins = 0;
  player2Wins = 0;
  roundEnded = false;
  multiplayerMode = false;
  lastDropRow = -1;
  lastDropCol = -1;
  currentPlayer = 1;
  cursorCol = 3;
  firstFrame = true;
  ready["periph"] = false;
  ready["host"] = false;

  screen_width = tft.width();
  screen_height = tft.height();

  // Clear sprite/drawing cache to prevent memory issues
  drawing.clearCache();
  drawing.clearSprite();
  drawing.deleteSprite();

  cursorIndex = 0;
  selection = 0;
  subselection = 0;
  prevSelection = -1;
  prevSubselection = -1;
  prevGameState = static_cast<State>(-1); // force redraw

  drawHomeScreen();
  drawHomescreenSelect();

  while (true) {
    handleConnect4Frame();

    if (getExitFlag()) {
      BluetoothManager::reset();
      return; // Return to game menu
    }

    // keep support for exiting with B from homescreen
    if (game_state == HOMESCREEN && B.wasJustPressed()) {
      Serial.println("Returning to menu");
      delay(500);
      BluetoothManager::reset();
      break;
    }
  }
}

// ========== Manual Loop ========== //
void handleConnect4Frame() {

  static int lastCursorCol = -1;
  static unsigned long lastMoveTime = 0;

  // Check if the Start Button was pressed and goes back to Main Menu
  if (checkStartButtonAndExit(tft))
    return;

  // ================== HOMESCREEN ================== //
  if (game_state == HOMESCREEN) {
    if (millis() - lastMoveTime > moveDelay / 2) {
      if (A.wasJustPressed()) {
        if (selection == 0) {
          resetMultiplayerState(true);
          multiplayerMode = false;
          game_state = SINGLE_PLAYER;
          firstFrame = true;

        } else if (selection == 1) {
          resetMultiplayerState(true);

          game_state = MULTIPLAYER_SELECTION;
          drawHomescreenSelect();
        }
      }

      // Menu navigation
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
        if (subselection == 0) {
          //clear ready status
          ready["periph"] = false;
          ready["host"] = false;
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
            localPlayerId = 1;

            game_state = HOST_SCREEN;
            firstFrame = true;

            // SEND INITIAL GAME STATE TO PERIPHERAL
            String initialState = generateConnect4StateString();
            BluetoothCentral &central = BluetoothManager::getCentral();
            for (auto *client : central.getConnectedClients()) {
              central.sendToDevice(client, initialState.c_str());
            }

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
            tft.fillScreen(bgColor);
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

  // ========== Multiplayer Logic for Host/Peripheral ========== //
  if (game_state == HOST_SCREEN || game_state == MULTIPLAYER_PLAYING) {
    // Don't continue if the host or peripheral aren't ready yet
    if(!ready["host"] || !ready["periph"])
      return;
    
    if(firstFrame){
      drawAllPlaying();
      firstFrame = false;
    }

    // Redraw If Stated Changed
    if (connect4StateChanged) {
      // Check dropped piece
      if(pieceDropCol >= 0 && playerDrop >= 0){
        if (playerDrop == localPlayerId)
          dropPiece(pieceDropCol, (3 - playerDrop));

      }
      // Check win
      if (checkWin((3 - playerDrop))) {
        ((3 - playerDrop) == 1 ? player1Wins++ : player2Wins++);
        playWinSound();
        drawWinLine((3 - playerDrop));
        roundEnded = true;
        winTime = millis();
      } else if (isBoardFull()) {
        roundEnded = true;
        winTime = millis();
      }

      // Draw updated score/cursor
      drawScorePanel(player1Wins, player2Wins, currentPlayer);
      drawCursor();
      connect4StateChanged = false;
    }

    // Turn Indicator
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, bgColor);

    if ((currentPlayer == 2 && game_state == HOST_SCREEN) ||
        (currentPlayer == 1 && game_state == MULTIPLAYER_PLAYING)) {
      tft.drawString("Waiting... ", tft.width() / 2, tft.height() - 10);
    } else {
      String turnText = (currentPlayer == 1) ? "Red's Turn " : "Blue's Turn";
      tft.drawString(turnText, tft.width() / 2, tft.height() - 10);


      // Handle Cursor Movement
      if (!roundEnded && millis() - lastMoveTime > moveDelay / 2) {
        if (left.isPressed() && cursorCol > 0) {
          cursorCol--;
          drawCursor();
        } else if (right.isPressed() && cursorCol < COLS - 1) {
          cursorCol++;
          drawCursor();
        }
        lastMoveTime = millis();
      }
  
      bool selectPressed = A.wasJustPressed();
  
      // Handle Drop
      if (!roundEnded && selectPressed && !buttonPreviouslyPressed &&
          localPlayerId == currentPlayer) {
  
        bool placed = dropPiece(cursorCol, currentPlayer);
  
        if (placed) {
          playDropSound(); // Play sound
  
          // Get position
          lastDropRow = getDropRow(cursorCol);
          lastDropCol = cursorCol;
          currentPlayer = (currentPlayer == 1) ? 2 : 1;
  
          // Draw updated score/cursor
          drawScorePanel(player1Wins, player2Wins, currentPlayer);
          drawCursor();
  
          String newState = generateConnect4StateString();
  
          if (game_state == HOST_SCREEN) {
            BluetoothCentral &central = BluetoothManager::getCentral();
            for (auto *client : central.getConnectedClients()) {
              if (!central.sendToDevice(client, newState.c_str())) {
                Serial.println("❌ Failed to notify client. Disconnecting...");
                ConnectionScreen::showMessage("Client disconnected.");
                shouldExitToMenu = true;
                return;
              }
            }
          } else if (game_state == MULTIPLAYER_PLAYING) {
            BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
            if (!peripheral.sendAction(newState.c_str())) {
              ConnectionScreen::showMessage("Disconnected (send failed)");
              shouldExitToMenu = true;
              return;
            }
          }

          // Check Winner
          if (checkWin(3 - currentPlayer)) {
            ( (3 - currentPlayer) == 1 ? player1Wins++ : player2Wins++);
            playWinSound();
            drawWinLine(3 - currentPlayer);
            roundEnded = true;
            winTime = millis();
          } else if (isBoardFull()) {
            roundEnded = true;
            winTime = millis();
          }
          delay(400); // debounce
        } else {
          playErrorSound();
          flashCursorRed();
        }
      }
  
      buttonPreviouslyPressed = selectPressed;
  
      // Redraw when moving or selecting
      if (cursorCol != lastCursorCol || selectPressed) {
        lastCursorCol = cursorCol;
      }
    }


    // Auto Restart
    if (roundEnded && millis() - winTime >= 5000) {
      resetConnect4Board(true);
      if (player1Wins >= 2 || player2Wins >= 2) {
        prevGameState = game_state;
        game_state = GAMEOVER_SCREEN;
        ready["host"] = false;
        ready["periph"] = false;
      } else {
        firstFrame = true;
      }
    }
  }

  // ================== SINGLE_PLAYER ================== //
  else if (game_state == SINGLE_PLAYER) {
    if (firstFrame) {
      drawAllPlaying();
      firstFrame = false;
    }

    // Cursor movement
    if (!roundEnded && millis() - lastMoveTime > moveDelay / 2) {
      if (left.isPressed() && cursorCol > 0) {
        cursorCol--;
        drawCursor();
      } else if (right.isPressed() && cursorCol < COLS - 1) {
        cursorCol++;
        drawCursor();
      }
      lastMoveTime = millis();
    }

    // ===== Human Turn ===== //
    if (!roundEnded && A.wasJustPressed() && currentPlayer == 1) {
      bool placed = dropPiece(cursorCol, currentPlayer);

      if (placed) {
        playDropSound();

        if (checkWin(currentPlayer)) {
          if (currentPlayer == 1) {
            player1Wins++;

            // BADGE LOGIC – only if Player 1 wins
            totalConnect4Wins++;
            if (totalConnect4Wins >= 3 && !badgeProgress[4]) {
              badgeProgress[4] = true;
              isUnlocked[4] = true;
              saveBadgeProgress();
              checkFinalBadgeUnlock();

              hasPendingNotification = true;
              pendingNotificationMessage = "Connect4 Badge Unlocked!";
              pendingNotificationDuration = 3000;
            }

          } else if (currentPlayer == 2) {
            player2Wins++;
          }

          drawScorePanel(player1Wins, player2Wins, currentPlayer);
          playWinSound();
          drawWinLine(currentPlayer);
          winTime = millis();
          roundEnded = true;

        } else if (isBoardFull()) {
          drawScorePanel(player1Wins, player2Wins, currentPlayer);
          winTime = millis();
          roundEnded = true;

        } else {
          currentPlayer = 2;
          drawScorePanel(player1Wins, player2Wins, currentPlayer);
        }
      } else {
        playErrorSound();
        flashCursorRed();
      }
    }

    // ===== AI Turn ===== //
    if (!roundEnded && currentPlayer == 2) {
      delay(300);                             // Optional: pause for realism
      int aiCol = findBestConnect4Move(2, 1); // AI is player 2

      if (aiCol != -1) {
        bool placed = dropPiece(aiCol, 2);
        if (placed) {
          playDropSound();

          if (checkWin(2)) {
            player2Wins++;
            drawScorePanel(player1Wins, player2Wins, 2);
            playWinSound();
            drawWinLine(2);
            winTime = millis();
            roundEnded = true;

          } else if (isBoardFull()) {
            drawScorePanel(player1Wins, player2Wins, 2);
            winTime = millis();
            roundEnded = true;

          } else {
            currentPlayer = 1;
            drawScorePanel(player1Wins, player2Wins, 1);
          }
        }
      }
    }

    // ===== Auto Restart ===== //
    if (roundEnded && millis() - winTime >= 3000) {
      if (player1Wins >= 2 || player2Wins >= 2) {
        game_state = GAMEOVER_SCREEN;
        resetConnect4Board(true);
      } else {
        resetConnect4Board(true);
        drawCursor();
        drawScorePanel(player1Wins, player2Wins, currentPlayer);
      }
    }
  }

  // ================== GAMEOVER_SCREEN ================== //
  else if (game_state == GAMEOVER_SCREEN) {
    std::vector<String> playerNames;
    std::vector<int> playerScores;

    bool multiplayer = multiplayerMode;

    if (multiplayer) {
      String redName = (localPlayerId == 1) ? settings.name : remotePlayerName;
      String blueName = (localPlayerId == 2) ? settings.name : remotePlayerName;

      playerNames = {redName, blueName};
      playerScores = {player1Wins, player2Wins};
    } else {
      playerNames = {"AI", settings.name};
      playerScores = {player2Wins, player1Wins}; // AI is player2
    }

    EndScreen endScreen(playerNames, playerScores, multiplayer, settings.name,
                        localPlayerId == 1 ? player1Wins : player2Wins); 

    if (endScreen.handleUserInput()) {

      // ==== Shared reset ==== //
      player1Wins = 0;
      player2Wins = 0;
      roundEnded = false;
      lastDropRow = -1;
      lastDropCol = -1;
      currentPlayer = 1;
      cursorCol = 3;
      firstFrame = true;

      // ===== Reset Board Data =====
      for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
          board[r][c] = 0;

      // ==== Multiplayer-specific state ====
      if (multiplayer) {
        game_state = prevGameState;
      }

      else {

        game_state = SINGLE_PLAYER;
        prevGameState = SINGLE_PLAYER;
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
      if(game_state == HOST_SCREEN || game_state == MULTIPLAYER_PLAYING){
        if(!ready["periph"] || !ready["host"]){
            tft.fillScreen(TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("WAITING FOR OTHER PLAYER...", tft.width()/2, tft.height()/2);
            tft.setTextDatum(TL_DATUM);
        }
      }

      // // ===== Redraw Everything Both States =====
      // drawGrid();
      // drawCursor();
      // drawScorePanel(player1Wins, player2Wins, currentPlayer);

      return;

    } else if (endScreen.exit) {
      BluetoothManager::reset();
      return;
    }

    // ===== Return to Menu =====
    player1Wins = 0;
    player2Wins = 0;
    BluetoothManager::reset(false);
    game_state = HOMESCREEN;
    drawHomeScreen();
  }

  // ================== BLUETOOTH_NUMPAD State =================== //
  else if (game_state == BLUETOOTH_NUMPAD) {
    pad.handleButtonInput(&lastMoveTime, moveDelay);

    std::string enteredCode = pad.getCode();
    if (enteredCode.length() == 6 && pad.wasEnterPressed()) {

      // JOIN = PERIPHERAL
      BluetoothManager::initPeripheral(tft);
      BluetoothPeripheral &peripheral = BluetoothManager::getPeripheral();
      peripheral.beginAdvertising(enteredCode);
      pad.clearCode();
      
      ready["periph"] = true;
      
      // Initial setup
      firstFrame = true;
      cursorCol = 3;
      roundEnded = false;
      localPlayerId = 2;
      multiplayerMode = true;
    }
    // Only start the game when both the host and peripheral are ready
    if(ready["periph"] && ready["host"]){
      BluetoothManager::getPeripheral().sendMessage("ready@periph,true"); // send ready message
      game_state = MULTIPLAYER_PLAYING;
    }
  }
}

// ####################################################################################################
//  Game Logic
// ####################################################################################################

// ========== Drop Asset ========== //
bool dropPiece(int col, int player) {
  for (int r = ROWS - 1; r >= 0; r--) {
    if (board[r][col] == 0) {
      animatePieceDrop(col, r, player); // animate it
      board[r][col] = player;
      Serial.println(board[r][col]);
      lastDropRow = r;
      lastDropCol = col;
      return true;
    }
  }
  return false; // Column full
}

// ========== Check Winner ========== //
bool checkWin(int player) {
  // Horizontal check
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS - 3; c++) {
      if (board[r][c] == player && board[r][c + 1] == player &&
          board[r][c + 2] == player && board[r][c + 3] == player) {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 0;
        winDirCol = 1;
        return true;
      }
    }
  }

  // Vertical check
  for (int c = 0; c < COLS; c++) {
    for (int r = 0; r < ROWS - 3; r++) {
      if (board[r][c] == player && board[r + 1][c] == player &&
          board[r + 2][c] == player && board[r + 3][c] == player) {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 1;
        winDirCol = 0;
        return true;
      }
    }
  }

  // Diagonal (\) check
  for (int r = 0; r < ROWS - 3; r++) {
    for (int c = 0; c < COLS - 3; c++) {
      if (board[r][c] == player && board[r + 1][c + 1] == player &&
          board[r + 2][c + 2] == player && board[r + 3][c + 3] == player) {
        winStartRow = r;
        winStartCol = c;
        winDirRow = 1;
        winDirCol = 1;
        return true;
      }
    }
  }

  // Diagonal (/) check
  for (int r = 3; r < ROWS; r++) {
    for (int c = 0; c < COLS - 3; c++) {
      if (board[r][c] == player && board[r - 1][c + 1] == player &&
          board[r - 2][c + 2] == player && board[r - 3][c + 3] == player) {
        winStartRow = r;
        winStartCol = c;
        winDirRow = -1;
        winDirCol = 1;
        return true;
      }
    }
  }

  return false; // no win found
}

// ========== Reset Game Board ========== //
void resetConnect4Board(bool clearScreen) {
  for (int r = 0; r < ROWS; r++)
    for (int c = 0; c < COLS; c++)
      board[r][c] = 0;

  cursorCol = 3;
  currentPlayer = 1;
  roundEnded = false;
  winTime = 0;
  winStartRow = winStartCol = winDirRow = winDirCol = 0;
  firstFrame = true;

  if (clearScreen)
    tft.fillScreen(bgColor);
}

// ========== Reset Single Player Defaults ========== //
void resetToSinglePlayerDefaults() {
  multiplayerMode = false;
  currentPlayer = 1;
  cursorCol = 3; // start at center
  roundEnded = false;
  player1Wins = 0;
  player2Wins = 0;
}

// ========== Reset Multiplayer State ========== //
void resetMultiplayerState(bool clearScreen) {
  multiplayerMode = true;
  currentPlayer = 1;
  cursorCol = 3;
  roundEnded = false;
  player1Wins = 0;
  player2Wins = 0;
  winStartRow = winStartCol = winDirRow = winDirCol = 0;

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      board[r][c] = 0;
    }
  }

  if (clearScreen)
    tft.fillScreen(bgColor);
}

// ========== Flash Cursor Red (Invalid Move Feedback) ========== //
void flashCursorRed() {
  int x = IMAGE_X + 15 + cursorCol * CELL_W + 25;
  int y = IMAGE_Y - 20;

  for (int i = 0; i < 2; i++) {
    tft.fillTriangle(x - 8, y, x + 8, y, x, y + 13, TFT_RED);
    delay(100);
    tft.fillTriangle(x - 8, y, x + 8, y, x, y + 13, bgColor);
    delay(100);
  }

  drawCursor(); // Restore normal green arrow
}

// ========== AI Setup ========== //
int findBestConnect4Move(int aiPlayer, int humanPlayer) {
  // 1. Try to win
  for (int col = 0; col < COLS; col++) {
    if (canDrop(col)) {
      int row = getAvailableRow(col);
      board[row][col] = aiPlayer;
      bool win = checkWin(aiPlayer);
      board[row][col] = 0; // Undo
      if (win)
        return col;
    }
  }

  // 2. Block opponent's winning move
  for (int col = 0; col < COLS; col++) {
    if (canDrop(col)) {
      int row = getAvailableRow(col);
      board[row][col] = humanPlayer;
      bool win = checkWin(humanPlayer);
      board[row][col] = 0; // Undo
      if (win)
        return col;
    }
  }

  // 3. Play center column
  int center = COLS / 2;
  if (canDrop(center))
    return center;

  // 4. Play random valid column
  std::vector<int> validCols;
  for (int col = 0; col < COLS; col++) {
    if (canDrop(col))
      validCols.push_back(col);
  }
  if (!validCols.empty())
    return validCols[random(validCols.size())];

  // No valid moves
  return -1;
}

// ========== Get Available Row ========== //
int getAvailableRow(int col) {
  for (int row = ROWS - 1; row >= 0; row--) {
    if (board[row][col] == 0)
      return row;
  }
  return -1;
}

// ========== Can Move ========== //
bool canDrop(int col) { return board[0][col] == 0; }

// ========== Board Full ========== //
bool isBoardFull() {
  for (int col = 0; col < COLS; col++) {
    if (board[0][col] == 0)
      return false;
  }
  return true;
}

// ========== Gets Position of Dropped ========== //
int getDropRow(int col) {
  for (int r = 0; r < ROWS; r++) {
    if (board[r][col] != 0) {
      return r;
    }
  }
  return ROWS - 1; // fallback (shouldn't happen if placed correctly)
}

// ####################################################################################################
//  Game Drawing
// ####################################################################################################

// ========== Draw Grid + Piece ========== //
void drawGrid() {
  tft.fillScreen(bgColor);

  // Now draw your board on top of that
  drawing.drawSdJpeg("/connect4/assets/connectBoard.jpg", IMAGE_X, IMAGE_Y);
  drawing.pushSprite(true, true,
                     TFT_BLACK); // Or bgColor if needed — doesn't matter now
  tft.setTextColor(TFT_WHITE);

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      int cx = IMAGE_X + 15 + c * CELL_W + 25;
      int cy = IMAGE_Y + 15 + r * CELL_H + 12;
      int px = cx - 15;
      int py = cy - 15;

      if (board[r][c] == 1) {
        drawing.drawSdJpeg("/connect4/assets/redPiece.jpg", px, py);
        drawing.pushSprite(true, bgColor);
      } else if (board[r][c] == 2) {
        drawing.drawSdJpeg("/connect4/assets/bluePiece.jpg", px, py);
        drawing.pushSprite(true, bgColor);
      }
    }
  }
}

// ========== Draw Piece ========== //
void drawSinglePieceAt(int row, int col) {
  int px = IMAGE_X + col * CELL_W;
  int py = IMAGE_Y + row * CELL_H;

  drawing.setFirst(false); // ← disables internal (0,0) offset
  drawing.x_pos = px;      // ← explicitly sets draw position
  drawing.y_pos = py;

  if (board[row][col] == 1) {
    drawing.drawSdJpeg("/connect4/assets/redPiece.jpg", 0,
                       0); // draw into sprite at (0,0)
  } else if (board[row][col] == 2) {
    drawing.drawSdJpeg("/connect4/assets/bluePiece.jpg", 0, 0);
  }

  drawing.pushSprite(true, false); // pushes the sprite to (x_pos, y_pos)
}

void drawBoardPieces() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      int piece = board[r][c];
      if (piece == 1 || piece == 2) {
        drawSinglePieceAt(
            r, c); // Draw each piece individually at its correct location
      }
    }
  }
}

// ========== Draw Cursor ========== //
void drawCursor() {
  tft.fillRect(0, 0, tft.width(), 40, bgColor); // clear top area

  int x = IMAGE_X + 15 + cursorCol * CELL_W + 25; // left edge of column
  int y = IMAGE_Y - 20;                           // above board

  tft.fillTriangle(x - 8, y, x + 8, y, x, y + 13, TFT_GREEN);
}

// ========== Draw Winner Line ========== //
void drawWinLine(int player) {
  uint16_t color = (player == 1) ? TFT_RED : TFT_BLUE;

  // Start and end cells
  int startCol = winStartCol;
  int startRow = winStartRow;
  int endCol = winStartCol + winDirCol * 3;
  int endRow = winStartRow + winDirRow * 3;

  // Compute center of each cell
  int x1 = IMAGE_X + 15 + startCol * CELL_W + 25;
  int y1 = IMAGE_Y + 15 + startRow * CELL_H + 12;
  int x2 = IMAGE_X + 15 + endCol * CELL_W + 25;
  int y2 = IMAGE_Y + 15 + endRow * CELL_H + 12;

  // Draw a thick line (5px total: offset -2 to +2)
  for (int offset = -2; offset <= 2; offset++) {
    tft.drawLine(x1 + offset, y1, x2 + offset, y2,
                 color); // horizontal offset
    tft.drawLine(x1, y1 + offset, x2, y2 + offset,
                 color); // vertical offset
  }
}

// ========== Draw Score Panel ========== //
void drawScorePanel(int p1Wins, int p2Wins, int currentPlayer) {
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);

  // Player 1 on the left
  tft.setTextColor(currentPlayer == 1 ? TFT_RED : TFT_LIGHTGREY);
  tft.fillRect(0, 40, 60, 60, bgColor);
  tft.drawString("P1", 20, 45);
  tft.drawString(String(p1Wins), 25, 70);
  tft.drawLine(15, 63, 10 + tft.textWidth("P1", 2), 63,
               currentPlayer == 1 ? TFT_RED : TFT_LIGHTGREY);

  // Player 2 on the right
  tft.setTextColor(currentPlayer == 2 ? TFT_BLUE : TFT_LIGHTGREY);
  tft.fillRect(420, 40, 60, 60, bgColor);
  tft.drawString("P2", 430, 45);
  tft.drawString(String(p2Wins), 440, 70);
  tft.drawLine(425, 63, 430 + tft.textWidth("P2", 2), 63,
               currentPlayer == 2 ? TFT_BLUE : TFT_LIGHTGREY);
}

// ========== Dropping Animation ========== //
void animatePieceDrop(int col, int targetRow, int player) {
  const char *piecePath = (player == 1) ? "/connect4/assets/redPiece.jpg"
                                        : "/connect4/assets/bluePiece.jpg";

  const int SPRITE_SIZE = 34;

  // Prepare eraser sprite once
  TFT_eSprite eraser = TFT_eSprite(&tft);
  eraser.createSprite(SPRITE_SIZE, SPRITE_SIZE);
  eraser.fillSprite(TFT_BLACK); // Transparent base
  eraser.fillCircle(SPRITE_SIZE / 2, SPRITE_SIZE / 2, PIECE_RADIUS + 2,
                    bgColor);

  int prevPx = 0, prevPy = 0;

  for (int r = 0; r <= targetRow; r++) {
    int px, py;
    getCellCenter(col, r, px, py); // Exact cell center

    // Erase previous sprite
    if (r > 0) {
      eraser.pushSprite(prevPx, prevPy, TFT_BLACK);
    }

    // Draw falling piece
    JpegDrawing drawer(tft);
    drawer.setFirst(true);
    drawer.createBuffer(32, 32);
    drawer.clearSprite(TFT_BLACK);
    drawer.drawSdJpeg(piecePath, 0, 0);
    drawer.x_pos = px + 2; // perfectly centered
    drawer.y_pos = py + 2;
    drawer.pushSprite(false, true, TFT_BLACK);

    prevPx = px;
    prevPy = py;

    delay(25);
  }

  eraser.deleteSprite(); // Clean up eraser

  // Final render to lock piece in place
  JpegDrawing final(tft);
  final.setFirst(true);
  final.createBuffer(32, 32);
  final.clearSprite(TFT_BLACK);
  final.drawSdJpeg(piecePath, 0, 0);
  final.x_pos = prevPx + 2;
  final.y_pos = prevPy + 2;
  final.pushSprite(false, true, TFT_BLACK);
  final.deleteSprite();
}

// ========== Center Piece ========== //
void getCellCenter(int col, int row, int &px, int &py) {
  int cx = IMAGE_X + 15 + col * CELL_W + 25;
  int cy = IMAGE_Y + 15 + row * CELL_H + 12;
  px = cx - 16; // exactly center for 32x32 sprite
  py = cy - 16;
}

// ========== Draw HomeScreen ========== //
void drawHomeScreen() {
  drawTitleAndGrid();
  drawHomescreenSelect();
}

// ========== Draw HomeScreen Buttons ========== //
void drawHomescreenSelect() {
  static int prevSelection = -1;
  static int prevSubselection = -1;
  static State prevGameState = HOMESCREEN;

  int y_single = 200;
  int y_multi = 250;
  int y_sub = y_multi + 40;

  // Always update buttons if we entered screen again
  bool stateChanged = (game_state != prevGameState);
  bool selectionChanged = (selection != prevSelection);
  bool subselectionChanged = (subselection != prevSubselection);

  // ---------- Always draw background and title when arriving ----------
  if (prevSelection == -1 || stateChanged) {
    drawTitleAndGrid();
  }

  // ---------- Always clear both main buttons ----------
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, screen_width, 35, bgColor);
  tft.fillRect(0, y_multi - 15, screen_width, 80, bgColor);

  // ---------- Draw Main Buttons (with highlight size) ----------
  tft.setTextSize(selection == 0 ? 3 : 2);
  tft.drawString("Press for Single-Player", screen_width / 2, y_single);

  tft.setTextSize(selection == 1 ? 3 : 2);
  tft.drawString("Press for Multiplayer", screen_width / 2, y_multi);

  // ---------- Draw Sub-options (Only if Multiplayer selection) ----------
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

    if (subselectionChanged || stateChanged) {
      // Clear sub-option row
      tft.fillRect(0, y_sub - boxHeight / 2 - 2, screen_width, boxHeight + 10,
                   bgColor);

      // Highlight selected suboption
      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw both options
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }

  // ---------- Save state ----------
  prevSelection = selection;
  prevSubselection = subselection;
  prevGameState = game_state;
}

// ========== Draw Title & Grid ========== //
void drawTitleAndGrid() {
  // Clear screen
  tft.fillScreen(bgColor);

  // Title
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.drawString("CONNECT 4", screen_width / 2, 40);

  // Grid preview
  int previewCols = 7, previewRows = 6, cellSize = 20;
  int previewWidth = previewCols * cellSize;
  int previewHeight = previewRows * cellSize;
  int gridX = (screen_width - previewWidth) / 2;
  int gridY = 80;

  for (int r = 0; r < previewRows; r++) {
    for (int c = 0; c < previewCols; c++) {
      int x = gridX + c * cellSize;
      int y = gridY + r * cellSize;
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 7, TFT_BLACK);
      tft.drawCircle(x + cellSize / 2, y + cellSize / 2, 7, TFT_WHITE);
    }
  }
}

// ========== Draw All Playing ========== //
void drawAllPlaying() {
  drawGrid();
  drawScorePanel(player1Wins, player2Wins, currentPlayer);

  // Only show cursor if it's your turn
  if (multiplayerMode && currentPlayer == localPlayerId && !roundEnded)
    drawCursor();
  else if (!multiplayerMode && !roundEnded)
    drawCursor();
}

// ####################################################################################################
//  Audio Logic
// ####################################################################################################

// ========== Dropping Piece ========== //
void playDropSound() {
  playTone(700, volume);
  delay(80);
  playTone(0, 0); // stop
}

// ========== Winner Sound ========== //
void playWinSound() {
  int melody[] = {880, 988, 1047};
  for (int i = 0; i < 3; i++) {
    playTone(melody[i], volume);
    delay(150);
    playTone(0, 0);
    delay(50);
  }
}

// ========== Error Sound ========== //
void playErrorSound() {
  playTone(300, volume);
  delay(200);
  playTone(0, 0);
}

// ####################################################################################################
//  Bluetooth Logic
// ####################################################################################################

// =================== Generate Connect4 State =================== //
String generateConnect4StateString() {
  String state = "c4@";

  String redName = (localPlayerId == 1) ? formatName(settings.name)
                                        : formatName(remotePlayerName);
  String blueName = (localPlayerId == 2) ? formatName(settings.name)
                                         : formatName(remotePlayerName);

  state += "0,R," + String(player1Wins) + "," + redName + ";\n";
  state += "1,B," + String(player2Wins) + "," + blueName + ";\n";
  state += "5;\n";
  state += (currentPlayer == 1 ? "R" : "B") + String(";\n");

  state += String(cursorCol) + ";\n";

  if (roundEnded) {
    state += (currentPlayer == 1 ? "R" : "B") + String(";") +
             String(winStartRow) + "," + String(winStartCol) + "," +
             String(winDirRow) + "," + String(winDirCol) + ";\n";

  } else {
    state += "N;\n";
  }

  // Inlcude Drop Position
  state += String(lastDropRow) + "," + String(lastDropCol) + ";\n";

  return state;
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

// =================== Read Connect4 State =================== //
void readConnect4String(String oldState, const char *data) {
  String input = String(data);
  input.trim();
  input.replace("c4@", "");

  // Split on semicolon only (safer and simpler than `;\n`)
  std::vector<String> lines = split(input, ';');

  // Declare outside the loop
  int tempDropRow = -1;
  int tempDropCol = -1;
  int lastParsedPiece = -1;
  playerDrop = -1;
  pieceDropCol = -1;

  if (lines.size() < 8) {
    Serial.println("Invalid Connect4 state string (too short)");
    return;
  }

  for (int line = 0; line < lines.size(); line++) {
    String part = lines[line];
    part.trim();  // Clean up whitespace

    switch (line) {
    case 0: { // Red Player
      auto parts = split(part, ',');
      if (parts.size() >= 4 && localPlayerId == 2) {
        player1Wins = parts[2].toInt();
        remotePlayerName = parts[3];
      }
      break;
    }
    case 1: { // Blue Player
      auto parts = split(part, ',');
      if (parts.size() >= 4 && localPlayerId == 1) {
        player2Wins = parts[2].toInt();
        remotePlayerName = parts[3];
      }
      break;
    }
    case 2:
      // Reserved (maybe future data)
      break;
    case 3:
      currentPlayer = (part == "R") ? 1 : 2;
      break;
    case 4:
      if (localPlayerId != currentPlayer)
        cursorCol = part.toInt();
      break;
    case 5: {
      break;
    }
    case 6: {
      auto parts = split(part, ',');
      if (parts.size() == 2) {
        tempDropRow = parts[0].toInt();
        tempDropCol = parts[1].toInt();

        lastDropRow = tempDropRow;
        lastDropCol = tempDropCol;
      }
      break;
    }
    }
  }

  // Debugging
  Serial.print("Parsed lastDropCol: ");
  Serial.println(lastDropCol);
  Serial.print("Parsed currentPlayer: ");
  Serial.println(currentPlayer == 1);
  Serial.println(currentPlayer == 2);
  
  Serial.println(currentPlayer); 
  
  Serial.println(localPlayerId);
  
  Serial.print("Parsed tempDropRow: ");
  Serial.println(tempDropRow);
  Serial.print("Parsed tempDropCol: ");
  Serial.println(tempDropCol);

  // Draw last dropped piece
  if (localPlayerId == currentPlayer && lastDropCol >= 0) {
    playerDrop = currentPlayer;
    pieceDropCol = lastDropCol;
  }

  multiplayerMode = true;
  connect4StateChanged = true;
}



// ========== Format Name ========== //
String formatName(String name) {
  if (name.length() == 0)
    return name;
  name.toLowerCase();
  name.setCharAt(0, toupper(name.charAt(0)));
  return name;
}