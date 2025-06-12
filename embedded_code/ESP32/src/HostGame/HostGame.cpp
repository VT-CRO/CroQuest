// /src/HostGame/HostGame.cpp

#include "HostGame.hpp"

static TFT_eSPI *screen = nullptr;
static String lastCode = "";
static String lastStatus = "";

static unsigned long startTime = 0;
static bool messageShown = false;

// ========== Initialize Host ========== //
void HostGame::init(TFT_eSPI &display) { screen = &display; }

// ========== 6 Digit Code Screen ========== //
void HostGame::showCode(const String &code) {
  if (!screen)
    return;

  lastCode = code;
  messageShown = false;
  startTime = millis();

  screen->fillScreen(TFT_BLACK);
  screen->setTextColor(TFT_WHITE, TFT_BLACK);
  screen->setTextDatum(MC_DATUM);

  screen->setTextSize(3);
  screen->drawString("Hosting Game", screen->width() / 2, 60);

  screen->setTextSize(2);
  screen->drawString("Your Code:", screen->width() / 2, 120);

  screen->setTextSize(5);
  screen->drawString(code, screen->width() / 2, 170);

  screen->setTextSize(2);
  screen->drawString("Waiting for players...", screen->width() / 2, 240);
}

void HostGame::showStatus(const String &msg) {
  if (!screen)
    return;

  lastStatus = msg;

  screen->setTextColor(TFT_YELLOW, TFT_BLACK);
  screen->setTextDatum(MC_DATUM);
  screen->setTextSize(2);
  screen->drawString(msg, screen->width() / 2, 220); // Below the code
}

void HostGame::loopUntilConnected() {
  while (true) {
    updateAllButtons();

    if (checkStartButtonAndExit(*screen)) {
      break; // return to game menu
    }

    // Add small delay to avoid burning CPU
    delay(50);
  }
}
