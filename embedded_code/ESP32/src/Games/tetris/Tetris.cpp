#include "Tetris.hpp"

// Tetris States
enum State {HOMESCREEN, PLAYING, ENDSCREEN, MULTIPLAYER_SELECTION, JOIN_SCREEN, BLUETOOTH_NUMPAD};
static State currentState = HOMESCREEN;

// ============ Drawing ============= //
static void drawHomeScreen(); 
static void drawHomeSelection();

// ============== GAME Functions ============== //
static void startNewGame();
static void handleTetrisFrame();

// Numpad
static NumPad<State> pad(drawHomeScreen, startNewGame,
                              &currentState, HOMESCREEN,
                              PLAYING);

// Selection
static int selection = 0;
static int subselection = 0;

// Timing variables
static unsigned long lastButtonPressTime = 0;
static unsigned long buttonDebounceDelay = 200;

static uint16_t bgColor = TFT_BLACK;

static const int GRID_HEIGHT = 20;
static const int GRID_WIDTH = 10;

// GAME VARIABLES
// static uint8_t grid[GRID_HEIGHT][GRID_WIDTH];
// static uint8_t colors[6]  = {TFT_CYAN, TFT_YELLOW, TFT_PURPLE, TFT_GREEN, TFT_RED, TFT_BLUE};
// static int score = 0;
// static int fallInterval = 500;


// All Possible Shapes
const uint8_t SHAPES[7][4][4] = {
  // I
  {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0}
  },
  // O
  {
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // T
  {
    {0, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // S
  {
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // Z
  {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // J
  {
    {1, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // L
  {
    {0, 0, 1, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  }
};

// Block Types
enum BlockType {I, O, T, S, Z, J, L};

// Block Colors
const uint16_t colors[7] = 
    {TFT_BLUE, TFT_GREEN, TFT_RED, TFT_ORANGE, TFT_CYAN, TFT_YELLOW, TFT_PINK};



// Setup and entry point from menu
void runTetris(){

    // Clear Screen
    tft.fillScreen(TFT_BLACK);
    drawHomeScreen();
    //Loop
    for(;;){
        handleTetrisFrame();
        if (currentState == HOMESCREEN && B.wasJustPressed()) {
            Serial.println("Returning to menu from Breakout");
            delay(500);
            return;
        }
    }
}

//Handles States, input and generally controls gameplay
static void handleTetrisFrame(){
    switch(currentState){
        case HOMESCREEN:
            if (millis() - lastButtonPressTime > buttonDebounceDelay) {
                if (A.wasJustPressed()) {
                    if (selection == 1) {
                    currentState = MULTIPLAYER_SELECTION;
                    drawHomeSelection();
                    } else {
                    startNewGame();
                    }
                    lastButtonPressTime = millis();
                } else if (up.isPressed()) {
                    if (selection == 1) {
                    selection = 0;
                    drawHomeSelection();
                    }
                    lastButtonPressTime = millis();
                } else if (down.isPressed()) {
                    if (selection == 0) {
                    selection = 1;
                    drawHomeSelection();
                    }
                    lastButtonPressTime = millis();
                }
            }
            break;
        case MULTIPLAYER_SELECTION:
            if (millis() - lastButtonPressTime > buttonDebounceDelay) {
                if (left.wasJustPressed()) {
                    if (subselection == 1) {
                    subselection = 0;
                    drawHomeSelection();
                    }
                } else if (right.wasJustPressed()) {
                    if (subselection == 0) {
                    subselection = 1;
                    drawHomeSelection();
                    }
                } else if (A.wasJustPressed()) {
                    if (subselection == 0) {
                    tft.fillScreen(TFT_BLUE);
                    currentState = JOIN_SCREEN;
                    } else {
                        pad.numPadSetup();
                        currentState = BLUETOOTH_NUMPAD;
                        break;
                    }
                } else if (up.wasJustPressed()) {
                    currentState = HOMESCREEN;
                    subselection = 0;
                    selection = 1;
                    drawHomeSelection();
                }
                lastButtonPressTime = millis();
            }
            break;
        case PLAYING:
            break;
        case ENDSCREEN:
            break;
    }
}


static void startNewGame(){

}



// ============== DRAWING =============== //

static void drawHomeScreen() {

  // Set title properties
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(4);
  tft.drawString("TETRIS", tft.width() / 2, 40);

  // Tagline
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Falling Block Puzzle", tft.width() / 2, 90);

  // Options
  tft.setTextSize(2);
  tft.drawString("Start Single Player", tft.width() / 2, 180);
  tft.drawString("Start Multiplayer", tft.width() / 2, 230);

  drawHomeSelection();
}

static void drawHomeSelection() {
  int y_single = 180;
  int y_multi = 230;
  int y_sub = y_multi + 40;

  // Clear option areas
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, y_single - 15, tft.width(), 35, TFT_BLACK);
  tft.fillRect(0, y_multi - 15, tft.width(), 80, TFT_BLACK);

  if (selection == 0) {
    // Single-player selected
    tft.setTextSize(3);
    tft.drawString("Start Single Player", tft.width() / 2, y_single);

    tft.setTextSize(2);
    tft.drawString("Start Multiplayer", tft.width() / 2, y_multi);
  } else {
    // Multiplayer selected
    tft.setTextSize(2);
    tft.drawString("Start Single Player", tft.width() / 2, y_single);

    tft.setTextSize(3);
    tft.drawString("Start Multiplayer", tft.width() / 2, y_multi);

    if (currentState == MULTIPLAYER_SELECTION) {
      const char *sub1 = "Host Game";
      const char *sub2 = "Join Game";

      tft.setTextSize(2);
      int padding_x = 10;
      int padding_y = 4;
      int boxHeight = 20 + padding_y * 2;

      int sub1Width = tft.textWidth(sub1);
      int sub2Width = tft.textWidth(sub2);
      int sub1BoxWidth = sub1Width + padding_x * 2;
      int sub2BoxWidth = sub2Width + padding_x * 2;

      int x_sub1 = tft.width() / 4;
      int x_sub2 = 3 * tft.width() / 4;

      // Highlight rectangle
      if (subselection == 0) {
        tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2,
                     sub1BoxWidth, boxHeight, TFT_WHITE);
      } else if (subselection == 1) {
        tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2,
                     sub2BoxWidth, boxHeight, TFT_WHITE);
      }

      // Draw sub-option labels
      tft.drawString(sub1, x_sub1, y_sub);
      tft.drawString(sub2, x_sub2, y_sub);
    }
  }
}