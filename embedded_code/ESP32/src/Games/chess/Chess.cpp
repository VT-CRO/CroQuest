// Games/chss/Chess.cpp

#include "Chess.hpp"

// ####################################################################################################
//  Functions Declarations
// ####################################################################################################

// ========== Drawing ==========
void drawChessBoard();
void drawAllChess(bool forceRedraw); // new declaration
void highlightCursor(int row, int col);
void drawJpegDirect(const char *filename, int xpos, int ypos);
void drawPossibleMoves();
void clearPossibleMoves();

// ========== Sound ==========

// ========== Logic ==========
void resetChessBoard(bool clearScreen = true);
void handleCursorMovement();
void handleSelection();
void makeAIMove();

std::vector<std::pair<int, int>> getPossibleMoves(char piece, int row, int col);
std::vector<std::tuple<int, int, char>> highlightedTiles;
std::vector<std::tuple<int, int, int, int>> getAllLegalMoves(bool forWhite);

// ========== Game States ==========
enum ChessState {
  CHESS_HOMESCREEN,
  CHESS_MULTIPLAYER,
  CHESS_PLAYING,
  CHESS_GAMEOVER,
  CHESS_SINGLE_PLAYER,
  CHESS_GAMEOVER_SCREEN,
  CHESS_BLUETOOTH_NUMPAD,
  CHESS_MULTIPLAYER_SELECTION,
  CHESS_MULTIPLAYER_PLAYING,
  CHESS_JOIN_SCREEN,
  CHESS_HOST_SCREEN,
};

static ChessState chess_state = CHESS_PLAYING;

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

// Speaker Pin
#define SPEAKER_PIN 21

#define LIGHT_BROWN 0xBC49
#define DARK_BROWN 0x6249
#define CURSOR_COLOR TFT_GREEN

#define TILE_SIZE 32

const int BOARD_SIZE = TILE_SIZE * 8;

const int BOARD_WIDTH = TILE_SIZE * 8;
const int BOARD_HEIGHT = TILE_SIZE * 8;

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 320;

int BOARD_X = 0;
int BOARD_Y = 0;

static int cursorX = 0;
static int cursorY = 0;

bool whiteTurn = true;

int selectedRow = -1;
int selectedCol = -1;
bool hasSelection = false;
std::vector<std::pair<int, int>> possibleMoves;

bool boardNeedsRedraw = true;

char chessBoard[8][8]; // 'P','p','R','r', etc. (uppercase = white, lowercase =
                       // black)

static bool firstFrame = true;
static unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 100;

// ####################################################################################################
//  Setup & Loop
// ####################################################################################################

// ========== Run Game ========== //
void runChess() {

  resetExitFlag();

  tft.setRotation(3); // Landscape mode (wide screen)
  tft.fillScreen(TFT_BLACK);

  // Changes position of selector
  BOARD_X = (tft.width() - BOARD_WIDTH) / 2;
  BOARD_Y = (tft.height() - BOARD_HEIGHT) / 2;

  resetChessBoard();

  firstFrame = true;

  while (true) {
    handleChessFrame();

    if (getExitFlag())
      return;

    if (B.wasJustPressed()) {
      Serial.println("Returning to menu from Chess");
      delay(500);
      return;
    }
  }
}

// ========== Manual Loop ========== //
void handleChessFrame() {
  if (checkStartButtonAndExit(tft))
    return;

  if (firstFrame || boardNeedsRedraw) {
    drawAllChess(false); // board + pieces into sprite
    boardNeedsRedraw = false;
    firstFrame = false;
  }

  if (millis() - lastMoveTime > moveDelay) {
    handleCursorMovement(); // only updates cursor
    handleSelection();      // eventually triggers move
    lastMoveTime = millis();
  }

  // Always redraw cursor on top of sprite
  drawing.pushSprite(BOARD_X, BOARD_Y, TFT_WHITE);

  highlightCursor(cursorY, cursorX);
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Cursor Movement ========== //
void handleCursorMovement() {
  static int lastX = -1;
  static int lastY = -1;

  int newX = cursorX;
  int newY = cursorY;

  if (up.isPressed() && cursorY > 0)
    newY--;
  else if (down.isPressed() && cursorY < 7)
    newY++;
  else if (left.isPressed() && cursorX > 0)
    newX--;
  else if (right.isPressed() && cursorX < 7)
    newX++;

  if (newX != cursorX || newY != cursorY) {
    cursorX = newX;
    cursorY = newY;
  }
}

// ========== Cursor Selection ========== //
void handleSelection() {
  if (!A.wasJustPressed())
    return;

  char current = chessBoard[cursorY][cursorX];

  // === 1. No piece selected yet ===
  if (!hasSelection && current != '.') {
    bool isWhite = isupper(current);
    if (isWhite != whiteTurn) {
      Serial.println("⛔ Not your turn.");
      clearPossibleMoves();
      return; // Wrong turn
    }

    selectedRow = cursorY;
    selectedCol = cursorX;
    hasSelection = true;
    possibleMoves = getPossibleMoves(current, selectedRow, selectedCol);
    clearPossibleMoves();
    drawPossibleMoves(); // show valid moves
    return;
  }

  // === 2. Already selected a piece ===
  if (hasSelection) {
    // If user selects another piece of their own, switch selection
    if (current != '.') {
      bool isWhite = isupper(current);

      if (isWhite == whiteTurn) {
        clearPossibleMoves();

        selectedRow = cursorY;
        selectedCol = cursorX;
        possibleMoves = getPossibleMoves(current, selectedRow, selectedCol);
        drawPossibleMoves();
        return;
      }
    }

    // === 3. Check if clicked square is a valid move ===
    for (auto &move : possibleMoves) {
      if (move.first == cursorY && move.second == cursorX) {

        // Move the piece
        char movedPiece = chessBoard[selectedRow][selectedCol];
        chessBoard[cursorY][cursorX] = movedPiece;
        chessBoard[selectedRow][selectedCol] = '.';

        // Redraw only the origin and destination tiles
        int fromX = BOARD_X + selectedCol * TILE_SIZE;
        int fromY = BOARD_Y + selectedRow * TILE_SIZE;
        int toX = BOARD_X + cursorX * TILE_SIZE;
        int toY = BOARD_Y + cursorY * TILE_SIZE;

        bool fromLight = (selectedRow + selectedCol) % 2 == 0;
        bool toLight = (cursorY + cursorX) % 2 == 0;

        tft.fillRect(fromX, fromY, TILE_SIZE, TILE_SIZE,
                     fromLight ? LIGHT_BROWN : DARK_BROWN);
        tft.fillRect(toX, toY, TILE_SIZE, TILE_SIZE,
                     toLight ? LIGHT_BROWN : DARK_BROWN);

        char path[32];
        snprintf(path, sizeof(path), "/chess/assets/%c%c.jpg",
                 isupper(movedPiece) ? 'W' : 'B', toupper(movedPiece));

        JpegDrawing drawer(tft);
        drawer.setFirst(true);
        drawer.createBuffer(TILE_SIZE, TILE_SIZE);
        drawer.clearSprite(TFT_WHITE);
        drawer.drawSdJpeg(path, 0, 0);
        drawer.x_pos = toX + 10;
        drawer.y_pos = toY + 5;
        drawer.pushSprite(false, true, TFT_WHITE);

        // Reset selection before AI turn
        hasSelection = false;
        possibleMoves.clear();

        // End turn
        whiteTurn = false;
        delay(300); // Optional small pause
        makeAIMove();
        return;
      }
    }
  }
}

// ========== Possible Moves ========== //
std::vector<std::pair<int, int>> getPossibleMoves(char piece, int row,
                                                  int col) {
  std::vector<std::pair<int, int>> moves;
  bool isWhite = isupper(piece);
  piece = tolower(piece);

  auto isInside = [](int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
  };

  auto canMoveTo = [&](int r, int c) {
    if (!isInside(r, c))
      return false;
    char target = chessBoard[r][c];
    return target == '.' || isupper(target) != isWhite;
  };

  if (piece == 'p') {
    int dir = isWhite ? -1 : 1;
    int startRow = isWhite ? 6 : 1;

    // Forward move
    if (isInside(row + dir, col) && chessBoard[row + dir][col] == '.')
      moves.push_back({row + dir, col});

    // Double forward on first move
    if (row == startRow && chessBoard[row + dir][col] == '.' &&
        chessBoard[row + 2 * dir][col] == '.')
      moves.push_back({row + 2 * dir, col});

    // Diagonal captures
    for (int d = -1; d <= 1; d += 2) {
      int newCol = col + d;
      int newRow = row + dir;
      if (isInside(newRow, newCol)) {
        char target = chessBoard[newRow][newCol];
        if (target != '.' && isupper(target) != isWhite)
          moves.push_back({newRow, newCol});
      }
    }

  } else if (piece == 'r' || piece == 'q') {
    // Rook or Queen directions
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    for (int d = 0; d < 4; ++d) {
      for (int i = 1;; ++i) {
        int nr = row + dr[d] * i;
        int nc = col + dc[d] * i;
        if (!isInside(nr, nc))
          break;

        char target = chessBoard[nr][nc];
        if (target == '.') {
          moves.push_back({nr, nc});
        } else if (isupper(target) != isWhite) {
          moves.push_back({nr, nc}); // Capture
          break;
        } else {
          break; // Blocked by own piece
        }
      }
    }
  }

  if (piece == 'b' || piece == 'q') {
    // Bishop or Queen diagonals
    const int dr[] = {-1, -1, 1, 1};
    const int dc[] = {-1, 1, -1, 1};

    for (int d = 0; d < 4; ++d) {
      for (int i = 1;; ++i) {
        int nr = row + dr[d] * i;
        int nc = col + dc[d] * i;
        if (!isInside(nr, nc))
          break;

        char target = chessBoard[nr][nc];
        if (target == '.') {
          moves.push_back({nr, nc});
        } else if (isupper(target) != isWhite) {
          moves.push_back({nr, nc});
          break;
        } else {
          break;
        }
      }
    }
  }

  if (piece == 'n') {
    const int dx[] = {-2, -1, 1, 2, 2, 1, -1, -2};
    const int dy[] = {1, 2, 2, 1, -1, -2, -2, -1};
    for (int i = 0; i < 8; ++i) {
      int nr = row + dy[i];
      int nc = col + dx[i];
      if (canMoveTo(nr, nc))
        moves.push_back({nr, nc});
    }
  }

  if (piece == 'k') {
    const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
    const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};
    for (int i = 0; i < 8; ++i) {
      int nr = row + dy[i];
      int nc = col + dx[i];
      if (canMoveTo(nr, nc))
        moves.push_back({nr, nc});
    }
  }

  return moves;
}

// ========== Legal Moves ========== //
std::vector<std::tuple<int, int, int, int>> getAllLegalMoves(bool forWhite) {
  std::vector<std::tuple<int, int, int, int>> allMoves;

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      char piece = chessBoard[row][col];
      if (piece == '.' || isupper(piece) != forWhite)
        continue;

      auto moves = getPossibleMoves(piece, row, col);
      for (auto &m : moves) {
        allMoves.emplace_back(row, col, m.first, m.second);
      }
    }
  }

  return allMoves;
}

void makeAIMove() {
  auto allMoves = getAllLegalMoves(false); // Black's moves

  if (allMoves.empty()) {
    Serial.println("AI has no moves.");
    return;
  }

  auto [fromRow, fromCol, toRow, toCol] = allMoves[random(0, allMoves.size())];

  char movedPiece = chessBoard[fromRow][fromCol];
  chessBoard[toRow][toCol] = movedPiece;
  chessBoard[fromRow][fromCol] = '.';

  // Redraw origin and destination tiles
  int fromX = BOARD_X + fromCol * TILE_SIZE;
  int fromY = BOARD_Y + fromRow * TILE_SIZE;
  int toX = BOARD_X + toCol * TILE_SIZE;
  int toY = BOARD_Y + toRow * TILE_SIZE;

  bool fromLight = (fromRow + fromCol) % 2 == 0;
  bool toLight = (toRow + toCol) % 2 == 0;

  tft.fillRect(fromX, fromY, TILE_SIZE, TILE_SIZE,
               fromLight ? LIGHT_BROWN : DARK_BROWN);
  tft.fillRect(toX, toY, TILE_SIZE, TILE_SIZE,
               toLight ? LIGHT_BROWN : DARK_BROWN);

  char path[32];
  snprintf(path, sizeof(path), "/chess/assets/%c%c.jpg",
           isupper(movedPiece) ? 'W' : 'B', toupper(movedPiece));

  JpegDrawing drawer(tft);
  drawer.setFirst(true);
  drawer.createBuffer(TILE_SIZE, TILE_SIZE);
  drawer.clearSprite(TFT_WHITE);
  drawer.drawSdJpeg(path, 0, 0);
  drawer.x_pos = toX + 10;
  drawer.y_pos = toY + 5;
  drawer.pushSprite(false, true, TFT_WHITE);

  whiteTurn = true;
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// ========== Draw Chess Board ========== //
void drawChessBoard() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      bool isLight = (row + col) % 2 == 0;
      uint16_t color = isLight ? LIGHT_BROWN : DARK_BROWN;

      int x = BOARD_X + col * TILE_SIZE;
      int y = BOARD_Y + row * TILE_SIZE;

      tft.fillRect(x, y, TILE_SIZE, TILE_SIZE, color);
    }
  }
}

// ========== Draw Pieces ========== //
void drawAllChess(bool forceRedraw = false) {
  if (!forceRedraw && !boardNeedsRedraw)
    return;

  drawChessBoard(); // Draw the chess board first

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      char piece = chessBoard[row][col];
      if (piece == '.')
        continue;

      // Build the image path: e.g., /chess/assets/WP.jpg or
      // /chess/assets/bk.jpg
      char path[32];
      snprintf(path, sizeof(path), "/chess/assets/%c%c.jpg",
               isupper(piece) ? 'W' : 'B', toupper(piece));

      int x = BOARD_X + col * TILE_SIZE;
      int y = BOARD_Y + row * TILE_SIZE;

      // Draw JPEG into a sprite with white background made transparent
      JpegDrawing drawer(tft);
      drawer.setFirst(true); // Use sprite buffer
      drawer.createBuffer(TILE_SIZE, TILE_SIZE);
      drawer.clearSprite(TFT_WHITE); // Transparent base
      drawer.drawSdJpeg(path, 0, 0); // Draw image into sprite

      // Set sprite screen position
      drawer.x_pos = x + 10;
      drawer.y_pos = y + 5;
      drawer.pushSprite(
          false, true, TFT_WHITE); // Push to screen, treat white as transparent
    }
  }

  boardNeedsRedraw = false;
}

// ========== Highlight Cursor ========== //
void highlightCursor(int row, int col) {
  static int prevRow = -1;
  static int prevCol = -1;

  // Only update if the cursor actually moved
  if (row == prevRow && col == prevCol)
    return;

  // Erase the old highlight (restore tile color)
  if (prevRow != -1 && prevCol != -1) {
    int px = BOARD_X + prevCol * TILE_SIZE;
    int py = BOARD_Y + prevRow * TILE_SIZE;

    bool isLight = (prevRow + prevCol) % 2 == 0;
    uint16_t baseColor = isLight ? LIGHT_BROWN : DARK_BROWN;

    tft.drawRect(px, py, TILE_SIZE, TILE_SIZE, baseColor);
    tft.drawRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, baseColor);
  }

  // Draw new highlight
  int x = BOARD_X + col * TILE_SIZE;
  int y = BOARD_Y + row * TILE_SIZE;
  tft.drawRect(x, y, TILE_SIZE, TILE_SIZE, CURSOR_COLOR);
  tft.drawRect(x + 1, y + 1, TILE_SIZE - 2, TILE_SIZE - 2, CURSOR_COLOR);

  // Update previous position
  prevRow = row;
  prevCol = col;
}

// ========== Possible Moves ========== //
void drawPossibleMoves() {
  highlightedTiles.clear();
  highlightedTiles.clear(); // reset previous

  for (auto &move : possibleMoves) {
    int row = move.first;
    int col = move.second;
    char original = chessBoard[row][col]; // store original piece

    highlightedTiles.push_back(
        {row, col, chessBoard[row][col]}); // Save current state

    int x = BOARD_X + col * TILE_SIZE;
    int y = BOARD_Y + row * TILE_SIZE;
    tft.drawCircle(x + TILE_SIZE / 2, y + TILE_SIZE / 2, 5, TFT_YELLOW);
  }
}

// ========== Clear Possible Moves ========== //
void clearPossibleMoves() {
  for (auto &[row, col, piece] : highlightedTiles) {
    int x = BOARD_X + col * TILE_SIZE;
    int y = BOARD_Y + row * TILE_SIZE;
    bool isLight = (row + col) % 2 == 0;

    // Clear highlight
    tft.fillRect(x, y, TILE_SIZE, TILE_SIZE,
                 isLight ? LIGHT_BROWN : DARK_BROWN);

    // REDRAW THE PIECE IF ONE EXISTS
    char currentPiece = chessBoard[row][col];
    if (currentPiece != '.') {
      char path[32];
      snprintf(path, sizeof(path), "/chess/assets/%c%c.jpg",
               isupper(currentPiece) ? 'W' : 'B', toupper(currentPiece));

      JpegDrawing drawer(tft);
      drawer.setFirst(true);
      drawer.createBuffer(TILE_SIZE, TILE_SIZE);
      drawer.clearSprite(TFT_WHITE);
      drawer.drawSdJpeg(path, 0, 0);
      drawer.x_pos = x + 10;
      drawer.y_pos = y + 5;
      drawer.pushSprite(false, true, TFT_WHITE);
    }
  }

  highlightedTiles.clear();
  possibleMoves.clear(); // Clear list after drawing
}

// ####################################################################################################
//  Initialization
// ####################################################################################################

void resetChessBoard(bool clearScreen) {
  if (clearScreen)
    tft.fillScreen(TFT_BLACK);

  // Standard initial layout (you can improve this)
  const char *startingLayout[8] = {"rnbqkbnr", "pppppppp", "........",
                                   "........", "........", "........",
                                   "PPPPPPPP", "RNBQKBNR"};

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      chessBoard[row][col] = startingLayout[row][col];
    }
  }
}