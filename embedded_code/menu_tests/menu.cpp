#include <LovyanGFX.hpp>

// Custom LGFX class for ILI9341 on ESP32
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;

public:
  LGFX(void) {
    auto cfg = _bus_instance.config();

    cfg.spi_host   = SPI3_HOST;
    cfg.spi_mode   = 0;
    cfg.freq_write = 40000000;
    cfg.freq_read  = 16000000;
    cfg.spi_3wire  = false;
    cfg.use_lock   = true;
    cfg.dma_channel = 1;

    cfg.pin_sclk = 14;   // SCK
    cfg.pin_mosi = 13;   // MOSI
    cfg.pin_miso = 12;   // MISO
    cfg.pin_dc   = 2;


    _bus_instance.config(cfg);
    _panel_instance.setBus(&_bus_instance);

    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs  = 15;
    panel_cfg.pin_rst = 4;

    panel_cfg.memory_width  = 240;
    panel_cfg.memory_height = 320;
    panel_cfg.panel_width   = 240;
    panel_cfg.panel_height  = 320;
    panel_cfg.offset_x      = 0;
    panel_cfg.offset_y      = 0;
    panel_cfg.offset_rotation = 0;
    panel_cfg.dummy_read_pixel = 8;
    panel_cfg.dummy_read_bits = 1;
    panel_cfg.readable      = true;
    panel_cfg.invert        = false;
    panel_cfg.rgb_order     = false;
    panel_cfg.dlen_16bit    = false;
    panel_cfg.bus_shared    = true;

    _panel_instance.config(panel_cfg);
    setPanel(&_panel_instance);
  }
};

LGFX tft;

#define TFT_LED  21
#define BUTTON_RIGHT 36
#define BUTTON_LEFT  35

#define BUTTON_SELECT 34  // Choose the actual GPIO pin that will be used


const int screenWidth = 320;
const int screenHeight = 240;

const int gridRows = 3;
const int gridCols = 3;
const int cellWidth = screenWidth / gridCols;
const int cellHeight = screenHeight / gridRows;
const int padding = 10;
const int cornerRadius = 12;

// Games
const char* gameNames[gridRows][gridCols] = {
  { "Snake", "Pong", "Mario" },
  { "Tetris", "Breakout", "Flappy" },
  { "Asteroids", "Racer", "Shooter" }
};

int selectedRow = 0;
int selectedCol = 0;
int prevRow = 0;
int prevCol = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP); // Make sure to add the new button

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);


  drawDecoratedGrid();
}

void loop() {
  updateSelection();
  handleGameSelection();
}

void drawDecoratedGrid() {
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;

  for (int row = 0; row < gridRows; row++) {
    for (int col = 0; col < gridCols; col++) {
      int x = offsetX + col * cellWidth;
      int y = offsetY + row * cellHeight;
      int squareWidth = cellWidth - 2 * padding;
      int squareHeight = cellHeight - 2 * padding;

      uint16_t fillColor = TFT_BLUE;
      uint16_t borderColor = (row == selectedRow && col == selectedCol) ? TFT_RED : TFT_WHITE;

      tft.fillRoundRect(x + padding, y + padding, squareWidth, squareHeight, cornerRadius, fillColor);

      int thickness = (row == selectedRow && col == selectedCol) ? 3 : 1;
      for (int i = 0; i < thickness; i++) {
        tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i, squareHeight + 2 * i, cornerRadius + i, borderColor);
      }
      
      tft.setTextColor(TFT_WHITE);
      tft.setTextDatum(MC_DATUM);
      tft.setFont(&fonts::Font2);
      tft.drawString(gameNames[row][col], x + cellWidth / 2, y + cellHeight / 2);
    
    }
  }
}

void redrawChangedSquares() {
  drawSingleSquare(prevRow, prevCol, false);
  drawSingleSquare(selectedRow, selectedCol, true);
}

void updateSelection() {
  bool rightPressed = digitalRead(BUTTON_RIGHT) == LOW;
  bool leftPressed = digitalRead(BUTTON_LEFT) == LOW;

  static bool leftLast = false;
  static bool rightLast = false;

  prevRow = selectedRow;
  prevCol = selectedCol;

  if (leftPressed && !leftLast) {
    if (selectedRow == 0 && selectedCol == 0) {
      selectedRow = gridRows - 1;
      selectedCol = gridCols - 1;
    } else if (selectedCol > 0) {
      selectedCol--;
    } else {
      selectedCol = gridCols - 1;
      selectedRow--;
    }
    redrawChangedSquares();
  }

  if (rightPressed && !rightLast) {
    if (selectedRow == gridRows - 1 && selectedCol == gridCols - 1) {
      selectedRow = 0;
      selectedCol = 0;
    } else if (selectedCol < gridCols - 1) {
      selectedCol++;
    } else {
      selectedCol = 0;
      selectedRow++;
    }
    redrawChangedSquares();
  }

  leftLast = leftPressed;
  rightLast = rightPressed;
}

void drawSingleSquare(int row, int col, bool isSelected) {
  int offsetX = (screenWidth - (cellWidth * gridCols)) / 2;
  int offsetY = (screenHeight - (cellHeight * gridRows)) / 2;
  int x = offsetX + col * cellWidth;
  int y = offsetY + row * cellHeight;
  int squareWidth = cellWidth - 2 * padding;
  int squareHeight = cellHeight - 2 * padding;

  for (int i = 0; i < 3; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i, squareHeight + 2 * i, cornerRadius + i, TFT_BLUE);
  }

  int thickness = isSelected ? 3 : 1;
  uint16_t borderColor = isSelected ? TFT_RED : TFT_WHITE;
  for (int i = 0; i < thickness; i++) {
    tft.drawRoundRect(x + padding - i, y + padding - i, squareWidth + 2 * i, squareHeight + 2 * i, cornerRadius + i, borderColor);
  }
}

void handleGameSelection() {
  static bool selectLast = false;
  bool selectPressed = digitalRead(BUTTON_SELECT) == LOW;

  if (selectPressed && !selectLast) {
    const char* selectedGame = gameNames[selectedRow][selectedCol];
    Serial.print("Launching: ");
    Serial.println(selectedGame);

    // Call actual game start functions here
    if (strcmp(selectedGame, "Snake") == 0) {
      // startSnakeGame();
    } else if (strcmp(selectedGame, "Pong") == 0) {
      // startPongGame();
    } else if (strcmp(selectedGame, "Mario") == 0) {
      // startPlatformerGame();
    } else {
      Serial.println("Game not implemented.");
    }
  }

  selectLast = selectPressed;
}


