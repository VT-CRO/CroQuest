// GameMenu.hpp

#pragma once

#include <JPEGDecoder.h>
#include <SD.h>

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/AppState.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "Menu/GameSetup.hpp"
// #include "SettingsMenu/BrightnessMenu/Brightness.hpp"
#include "SettingsMenu/Settings/Settings.hpp"

// === Global Flags ===
extern bool shouldExitToMenu;

// === Total Number of Games ===
constexpr int TOTAL_GAMES = 11;

// === Game Data Structure ===
struct GameBox {
  const char *name;
  int row;
  int col;
};

// === Game Menu Class ===
class GameMenu {
public:
  GameMenu(TFT_eSPI *tft);

  void draw();        // Draws the full menu interface
  void handleInput(); // Handles user input (buttons)

private:
  TFT_eSPI *tft;
  int selectedIndex = 0;
  int currentPage = 0;

  GameBox gameBoxes[TOTAL_GAMES]; // Supports all games across pages
  JpegDrawing drawer;

  void drawPage();                         // Draws the menu page and selector
  void launchGameByName(const char *name); // Launches selected game
};

// === Layout Constants ===
namespace MenuLayout {

constexpr int page1Count = 8;
constexpr int page2Count = 3;

constexpr int ICON_SIZE = 80;
constexpr int ITEMS_PER_ROW = 4;
constexpr int ITEMS_PER_PAGE = 8;

constexpr int MARGIN_Y = 32;
constexpr int LEFT_MARGIN = 45;
constexpr int H_SPACING = 25;
constexpr int TOP_MARGIN = 92;

constexpr int SELECTOR_RADIUS = 12;
constexpr int SELECTOR_THICKNESS = 5;

constexpr uint16_t BACKGROUND_COLOR = 0x528A;

} // namespace MenuLayout