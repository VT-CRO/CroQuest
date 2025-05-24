// #include "Brightness.hpp"

// static TFT_eSPI *tft = nullptr;
// static TFT_eSprite *overlay = nullptr; // Now a pointer!

// void initBrightness(TFT_eSPI *display) {
//   tft = display;

//   // Allocate overlay sprite only once
//   if (!overlay) {
//     overlay = new TFT_eSprite(tft);
//     overlay->setColorDepth(8);
//   }
// }

// #define SETTINGS_BG_COLOR 0x528A

// void fakeDimOverlay(int brightnessLevel) {
//   if (!overlay || !tft)
//     return;

//   if (brightnessLevel >= 10) {
//     overlay->deleteSprite(); // remove overlay if full bright
//     return;
//   }

//   // Map brightnessLevel 0–9 to a dim gray shade (0 = darkest)
//   uint8_t shade = map(10 - brightnessLevel, 0, 10, 0, 160);
//   uint16_t dimColor = tft->color565(shade, shade, shade);

//   // Always recreate the sprite with full screen size (no need to reuse)
//   if (overlay->created())
//     overlay->deleteSprite(); // prevent memory leak

//   overlay->createSprite(tft->width(), tft->height());
//   overlay->fillSprite(dimColor);

//   // ⚠️ Push without transparency to fully dim screen
//   overlay->pushSprite(0, 0); // REMOVE the third arg
// }

// void runBrightnessMenu() {
//   if (!tft)
//     return;

//   int &brightness = settings.brightness;
//   constexpr int maxBrightness = 10;
//   constexpr int minBrightness = 0;

//   tft->fillScreen(SETTINGS_BG_COLOR);
//   tft->setTextDatum(MC_DATUM);
//   tft->setTextColor(TFT_WHITE);
//   tft->setTextSize(2);
//   tft->drawString("Adjust Brightness", tft->width() / 2, 30);

//   // === Draw Slider ===
//   auto drawSlider = [&]() {
//     int sliderX = 30;
//     int sliderY = 100;
//     int sliderW = tft->width() - 60;
//     int sliderH = 16;

//     tft->fillRect(sliderX - 2, sliderY - 2, sliderW + 4, sliderH + 4,
//                   SETTINGS_BG_COLOR);
//     tft->drawRoundRect(sliderX, sliderY, sliderW, sliderH, 4, TFT_WHITE);
//     int fillW = (brightness * sliderW) / maxBrightness;
//     tft->fillRect(sliderX + 1, sliderY + 1, fillW - 2, sliderH - 2, TFT_WHITE);

//     tft->fillRect(tft->width() / 2 - 30, sliderY + 25, 60, 20,
//                   SETTINGS_BG_COLOR);
//     tft->setTextDatum(MC_DATUM);
//     tft->setTextColor(TFT_WHITE, SETTINGS_BG_COLOR);
//     tft->drawString(String(brightness), tft->width() / 2, sliderY + 30);

//     fakeDimOverlay(brightness); // ✅ Overlay here
//   };

//   drawSlider();

//   tft->setTextSize(1);
//   tft->setTextDatum(BC_DATUM);
//   tft->drawString("Use LEFT/RIGHT to adjust, A to return", tft->width() / 2,
//                   tft->height() - 10);

//   while (true) {
//     if (left.wasJustPressed()) {
//       if (brightness > minBrightness) {
//         brightness--;
//         drawSlider();
//         delay(150);
//       }
//     } else if (right.wasJustPressed()) {
//       if (brightness < maxBrightness) {
//         brightness++;
//         drawSlider();
//         delay(150);
//       }
//     } else if (A.wasJustPressed()) {
//       break;
//     }
//     delay(10);
//   }

//   tft->setTextSize(2);
//   tft->setTextDatum(TL_DATUM);

//   // Make sure dim stays after menu closes
//   fakeDimOverlay(brightness);
// }
