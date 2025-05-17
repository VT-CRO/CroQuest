// menu.cpp

#include <Arduino.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Uses setup in User_Setup.h

// #include "menu.hpp"

// ###################### Game Structure ######################
struct Game {
  const char *name;
  const char *imagePath;
  void (*launch)(); // Use nullptr for unimplemented games
};

// ###################### Definitions ######################

//========================================================================
// TODO: Change this later
// External button input handlers (to be defined in your buttons code)
extern bool isLeftPressed();
extern bool isRightPressed();
extern bool isAButtonPressed();
//========================================================================

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

const char *MENU_BG = "/menu/assets/background.jpg";
const char *BREAKOUT = "/menu/assets/breakout.jpg";
const char *CHECKERS = "/menu/assets/checkers.jpg";
const char *CHESS = "/menu/assets/chess.jpg";
const char *CONNECT4 = "/menu/assets/connect4.jpg";
const char *MEMORY = "/menu/assets/memory.jpg";
const char *PONG = "/menu/assets/pong.jpg";
const char *SIMON = "/menu/assets/simon.jpg";
const char *SNAKE = "/menu/assets/snake.jpg";
const char *TETRIS = "/menu/assets/tetris.jpg";
const char *TICTACTOE = "/menu/assets/"
                        "tictactoe.jpg";
const char *UNO = "/menu/assets/uno.jpg";

// Draw the Menu
void drawMenu();

// Handle Input for the Menu
void handleMenuInput();

// Draw JPEG Images
void drawSdJpeg(const char *filename, int xpos, int ypos);

// Renders JPEG Images
void jpegRender(int xpos, int ypos);

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Games List ######################
Game games[] = {
    {"Breakout", BREAKOUT, nullptr}, {"Checkers", CHECKERS, nullptr},
    {"Chess", CHESS, nullptr},       {"Connect 4", CONNECT4, nullptr},
    {"Memory", MEMORY, nullptr},     {"Pong", PONG, nullptr},
    {"Simon", SIMON, nullptr},       {"Snake", SNAKE, nullptr},
    {"Tetris", TETRIS, nullptr},     {"Tic Tac Toe", TICTACTOE, nullptr},
    {"UNO", UNO, nullptr},
};

const int GAME_COUNT = sizeof(games) / sizeof(games[0]);
int selectedGameIndex = 0;
const int GAMES_PER_PAGE = 8;

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {

  // Initialize Serial Communication
  Serial.begin(115200);

  // Initialize the Screen
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Check if SD is working
  if (!SD.begin(5)) {
    Serial.println("Card Mount Failed");
    return;
  }

  drawMenu(); // Draw the initial menu screen
}

// ####################################################################################################
//  Main loop
// ####################################################################################################

void loop() { handleMenuInput(); }

// ###################### Draw Menu ######################
void drawMenu() {
  drawSdJpeg(MENU_BG, 0, 0); // Draw background

  int start = (selectedGameIndex / 8) * 8; // 8 games per page
  const int itemsPerRow = 4;
  const int cellSize = 60;
  const int margin = 48;
  const int topMargin = 82;

  for (int i = 0; i < 8 && (start + i) < GAME_COUNT; i++) {
    int index = start + i;

    int row = i / itemsPerRow;
    int col = i % itemsPerRow;

    int x = margin + (col * (cellSize + margin));
    int y = topMargin + (row * (cellSize + margin));

    drawSdJpeg(games[index].imagePath, x, y);

    if (index == selectedGameIndex) {
      tft.drawRect(x - 2, y - 2, cellSize + 4, cellSize + 4,
                   TFT_YELLOW); // Highlight box
    }
  }
}

// ###################### Handle Menu Input ######################
void handleMenuInput() {
  static unsigned long lastInputTime = 0;
  if (millis() - lastInputTime < 200)
    return; // debounce

  // if (isLeftPressed()) {
  //   selectedGameIndex = (selectedGameIndex - 1 + GAME_COUNT) % GAME_COUNT;
  //   drawMenu();
  //   lastInputTime = millis();
  // } else if (isRightPressed()) {
  //   selectedGameIndex = (selectedGameIndex + 1) % GAME_COUNT;
  //   drawMenu();
  //   lastInputTime = millis();
  // } else if (isAButtonPressed()) {
  //   if (games[selectedGameIndex].launch != nullptr) {
  //     games[selectedGameIndex].launch();
  //   }
  //   lastInputTime = millis();
  // }
}

// ###################### Draw SD JPEG ######################
void drawSdJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile =
      SD.open(filename, FILE_READ); // or, file handle reference for SD library

  if (!jpegFile) {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: ");
  Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded =
      JpegDec.decodeSdFile(jpegFile); // Pass the SD file handle to the decoder,

  if (decoded) {
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  } else {
    Serial.println("Jpeg file format not supported!");
  }
  delay(3600);
}

// ###################### Render JPEG images ######################
void jpegRender(int xpos, int ypos) {

  // jpegInfo(); // Print information from the JPEG file (could comment this
  // line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  // Jpeg images are draw as a set of image block (tiles) called Minimum
  // Coding Units (MCUs) Typically these MCUs are 16x16 pixel blocks Determine
  // the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an
  // image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image
  // cropping to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read()) { // While there is more data in the file
    pImg = JpegDec.pImage; // Decode a MCU (Minimum Coding Unit, typically a
                           // 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x)
      win_w = mcu_w;
    else
      win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y)
      win_h = mcu_h;
    else
      win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w) {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++) {
        p += mcu_w;
        for (int w = 0; w < win_w; w++) {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    // draw image MCU block only if it will fit on the screen
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);
}
