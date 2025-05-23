#include "JoinHost.hpp"

static TFT_eSPI *screen = nullptr;
static String lastCode = "";
static String lastStatus = "";

void JoinHost::init(TFT_eSPI &display) { screen = &display; }

void JoinHost::showCode(const String &code) {
  if (!screen)
    return;

  lastCode = code;

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

void JoinHost::showStatus(const String &msg) {
  if (!screen)
    return;

  lastStatus = msg;
  screen->setTextColor(TFT_YELLOW, TFT_BLACK);
  screen->setTextDatum(MC_DATUM);
  screen->setTextSize(2);
  screen->drawString(msg, screen->width() / 2, 220); // Below the code
}
