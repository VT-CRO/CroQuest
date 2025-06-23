#include "BackButton.hpp"

void back(int selection, uint16_t bgcolor, const char* label) {
    // --- Draw "Back" button at top left ---
    tft.setTextSize(2);
    if (selection == 0) {
        tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Highlighted (white background)
    } else {
        tft.setTextColor(TFT_WHITE, bgcolor);  // Normal
    }

    tft.setCursor(10, 10);
    tft.print(label);
}