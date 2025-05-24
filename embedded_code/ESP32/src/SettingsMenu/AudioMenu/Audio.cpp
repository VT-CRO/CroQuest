#include "Audio.hpp"

extern TFT_eSPI tft;
int& volume = settings.volume;  // now expected to range 0–100

//Static functions
static void sliderSound();

void runAudioMenu() {
  constexpr int maxPercent = 100;
  constexpr int minPercent = 0;
  constexpr int stepPercent = 5;

  tft.fillScreen(SETTINGS_BG_COLOR);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Adjust Volume", tft.width() / 2, 30);

  // === Draw Slider ===
  auto drawSlider = [&]() {
    int sliderX = 30;
    int sliderY = 100;
    int sliderW = tft.width() - 60;
    int sliderH = 16;
    int fillW = (volume * sliderW) / maxPercent;

    tft.fillRect(sliderX + fillW - 2, sliderY - 2, sliderW + 4, sliderH + 4, SETTINGS_BG_COLOR);
    tft.drawRoundRect(sliderX, sliderY, sliderW, sliderH, 4, TFT_WHITE);
    tft.fillRect(sliderX + 1, sliderY + 1, fillW - 2, sliderH - 2, TFT_WHITE);

    // Clear and draw fixed-width percent label
    tft.fillRect(tft.width() / 2 - 40, sliderY + 25, 80, 25, SETTINGS_BG_COLOR);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, SETTINGS_BG_COLOR);
    char percentStr[6];

    tft.setTextSize(2);
    snprintf(percentStr, sizeof(percentStr), "%3d%%", volume);  // e.g., "  5%", "100%"
    tft.drawString(percentStr, tft.width() / 2, sliderY + 30);
  };

  drawSlider();

  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("Use LEFT/RIGHT to adjust, A to return", tft.width() / 2,
                  tft.height() - 10);

  // Main loop 
  while (true) {
    if (left.wasJustPressed()) {
      if (volume > minPercent) {
        volume -= stepPercent;
        if (volume < minPercent) volume = minPercent;
        drawSlider();
        sliderSound();
        delay(150);
      }
    } else if (right.wasJustPressed()) {
      if (volume < maxPercent) {
        volume += stepPercent;
        if (volume > maxPercent) volume = maxPercent;
        drawSlider();
        sliderSound();
        delay(150);
      }
    } else if (A.wasJustPressed()) {
      backAudio();
      break;
    }
    delay(10);
  }

  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);
}

// ============= ADJUSTABLE VOLUME ================== //

void playTone(int toneFreq, int volume) {
    if (toneFreq == 0) {
        ledcWriteTone(channel, 0);
        return;
    }

    ledcWriteTone(channel, toneFreq);
    ledcWrite(channel, (volume * 255) / 100);  // Convert percent to 0–255 range
}


// =================== SOUNDS ========================= //

void playSelectBeep() {
  playTone(1000, volume); // 1kHz tone
  delay(50);         // very short beep
  playTone(0, 0);    // stop tone
}

void playPressSound() {
  ledcAttachPin(SPEAKER_PIN, 0);

  // Descending press sound: quick drop from 1200Hz to 800Hz
  int tones[] = {1200, 800};
  int durations[] = {40, 60};

  for (int i = 0; i < 2; i++) {
    playTone(tones[i], volume);
    delay(durations[i]);
  }

  playTone(0, 0); // stop tone
}

void backAudio(){
    playTone(900, volume); delay(30);
    playTone(700, volume); delay(30);
    playTone(0, volume);
}

static void sliderSound(){
  playTone(800, volume);
  delay(15);
  playTone(0, volume);
}
