// src/SettingsMenu/AboutUs/AboutUs.cpp
#include "AboutUs.hpp"

extern TFT_eSPI tft;

void runAboutUs() {
  const char *credits[] = {"About CroQuest",
                           "Made with Care in Blacksburg",
                           "",
                           "",
                           "DEVELOPED BY",
                           "",
                           "",
                           "FOUNDER AND CHIEF DESIGNER",
                           "Marco Gonzales Hauger",
                           "",
                           "",
                           "TEAM LEAD AND CHIEF ENGINEER",
                           "Felipe Campoverde",
                           "",
                           "",
                           "SOFTWARE ENGINEERS",
                           "Lucas Shadoyan",
                           "Connor McCue",
                           "",
                           "",
                           "ELECTRICAL ENGINEERS",
                           "Jonas Von Stein",
                           "Andrew Viola",
                           "",
                           "",
                           "DESIGNER LEAD",
                           "Morgan Weidling",
                           "",
                           "",
                           "DESIGNERS",
                           "Ayra Nirar",
                           "",
                           "",
                           "INDUSTRIAL DESIGNER",
                           "Steve KitamorY",
                           "",
                           "",
                           "LOGISTICS COORDINATOR",
                           "Heesang Han",
                           "",
                           "",
                           "Press B to return"};

  const int lineHeight = 22;
  const int numLines = sizeof(credits) / sizeof(credits[0]);
  const int totalHeight = numLines * lineHeight;
  const int screenHeight = tft.height();
  const int screenWidth = tft.width();
  const uint16_t bg = 0x528A;
  const uint16_t text = TFT_WHITE;

  // Which entries are titles
  bool isTitle[] = {
      true,  false, false, false, // About CroQuest section
      true,  false, false, false, // Developed by section
      false, false, true,  false, // Team Lead
      false, false, true,  false, // Software Engineers
      false, false, true,  false, // Electrical Engineers
      false, false, true,  false, // Designer Lead
      false, false, true,  false, // Designers
      false, false, true,  false, // Industrial Designer
      false, false, true,  false, // Logistics Coordinator
      false                       // Press B
  };

  tft.fillScreen(bg);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(text, bg);

  int scrollY = screenHeight;
  const int scrollSpeed = 1;
  const int frameDelay = 25;
  unsigned long lastFrame = 0;

  while (true) {
    unsigned long now = millis();
    if (now - lastFrame < frameDelay) {
      delay(1);
      continue;
    }
    lastFrame = now;

    int centerX = screenWidth / 2;

    for (int i = 0; i < numLines; i++) {
      int y = scrollY + i * lineHeight;
      if (y > -lineHeight && y < screenHeight) {
        // Clear the area
        tft.fillRect(0, y, screenWidth, lineHeight, bg);

        // Draw divider line between two blank lines followed by a real line
        if (i > 0 && i < numLines - 1 && strlen(credits[i]) == 0 &&
            strlen(credits[i - 1]) == 0 && strlen(credits[i + 1]) > 0) {
          int lineWidth = 80;
          int lineX = (screenWidth - lineWidth) / 2;
          tft.drawFastHLine(lineX, y + lineHeight / 2 - 12, lineWidth,
                            TFT_WHITE);
        }

        // First title: larger size
        else if (i == 0) {
          tft.setTextColor(text, bg);
          tft.setTextSize(2);
          tft.drawString(credits[i], centerX, y + lineHeight / 2);
        }

        // Other titles: bold
        else if (isTitle[i]) {
          drawBoldString(credits[i], centerX, y + lineHeight / 2, text, bg);
        }

        // Normal text
        else {
          tft.setTextSize(1);
          tft.drawString(credits[i], centerX, y + lineHeight / 2);
        }
      }
    }

    scrollY -= scrollSpeed;
    if (scrollY + totalHeight < 0) {
      scrollY = screenHeight;
    }

    if (B.wasJustPressed()) {
      backAudio();
      return;
    }
  }
}

void drawBoldString(const char *text, int x, int y, uint16_t color,
                    uint16_t bg) {
  tft.setTextColor(color, bg);
  tft.setTextSize(1);
  tft.drawString(text, x, y);
  tft.drawString(text, x + 1, y);
  tft.drawString(text, x, y + 1);
  tft.drawString(text, x + 1, y + 1);
  tft.drawString(text, x - 1, y);
  tft.drawString(text, x, y - 1);
}