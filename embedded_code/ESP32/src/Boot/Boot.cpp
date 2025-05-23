// Boot.cpp

#include "Boot.hpp"
#include "Core/JpegDrawing.hpp"

#include <FS.h>
#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ======================== Global Display Instance ========================
TFT_eSPI tft = TFT_eSPI(); // Shared display object

JpegDrawing drawing(tft);

// ======================== Init Display + SD ========================
void initBoot() {
  Serial.begin(115200);

  // Initializes screen, rotation and background.
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Optional pin logic (only if needed for display touch/SD muxing)
  digitalWrite(22, HIGH);
  digitalWrite(15, HIGH);
  digitalWrite(5, HIGH);

  if (!SD.begin(5)) {
    Serial.println("Card Mount Failed");
  }
}

// ======================== Boot Splash + Loading ========================
void showBootWithLoading(const char *path) {
  int totalDurationMs = 3500; // Duration that iterates circles
  showBootLoadingCombined(totalDurationMs);
  tft.fillScreen(TFT_BLACK);
  drawSdJpeg(path, 0, 0);
  delay(1300); // Time that shows logo CroQuest
}

// ======================== Fade-In Effect ========================
void fadeInJpeg() {
  for (int i = 255; i >= 0; i -= 15) {
    tft.fillScreen(tft.color565(i, i, i)); // Gray fade
    delay(20);
  }
}

// ======================== Combined Loading Animation ========================
void showBootLoadingCombined(int totalDurationMs) {
  const int dotInterval = 400;
  const int circleSteps = 10;
  const int circleInterval = 60;
  int circleFrame = 0;
  int dotFrame = 0;
  const int totalCircleFrames = circleSteps * 4;

  unsigned long startTime = millis();
  unsigned long lastDotTime = startTime;
  unsigned long lastCircleTime = startTime;

  while (millis() - startTime < totalDurationMs) {
    unsigned long now = millis();

    if (now - lastCircleTime >= circleInterval) {
      float progress = (circleFrame % totalCircleFrames) / (float)circleSteps;
      drawCroQuestCircleGrid(progress);
      circleFrame++;
      lastCircleTime = now;
    }

    if (now - lastDotTime >= dotInterval) {
      showLoadingDotsLine(dotFrame++);
      lastDotTime = now;
    }
  }

  tft.fillRect(180, 290, 100, 20, TFT_BLACK); // Clear loading line
}

// ======================== Circle Animation Frame ========================
void drawCroQuestCircleGrid(float progress) {
  static uint16_t lastColors[4] = {0, 0, 0, 0};

  uint8_t colorsRGB[4][3] = {
      {206, 0, 88}, {80, 133, 144}, {247, 234, 72}, {100, 38, 103}};

  uint8_t fadedRGB[4][3] = {
      {80, 0, 30}, {30, 60, 70}, {100, 100, 30}, {50, 15, 50}};

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int offset = SPACING / 2;

  int positions[4][2] = {{centerX - offset, centerY - offset},
                         {centerX + offset, centerY - offset},
                         {centerX - offset, centerY + offset},
                         {centerX + offset, centerY + offset}};

  for (int i = 0; i < 4; i++) {
    float blend = constrain(1.0 - abs(i - progress), 0.0, 1.0);
    uint8_t r = fadedRGB[i][0] + blend * (colorsRGB[i][0] - fadedRGB[i][0]);
    uint8_t g = fadedRGB[i][1] + blend * (colorsRGB[i][1] - fadedRGB[i][1]);
    uint8_t b = fadedRGB[i][2] + blend * (colorsRGB[i][2] - fadedRGB[i][2]);

    uint16_t color = tft.color565(r, g, b);

    if (color != lastColors[i]) {
      int x = positions[i][0];
      int y = positions[i][1];
      tft.fillCircle(x, y, CIRCLE_RADIUS, color);
      lastColors[i] = color;
    }
  }
}

// ======================== Animated Loading Dots ========================
void showLoadingDotsLine(int frame) {
  const int textX = 180;
  const int textY = 290;
  const int textWidth = 100;
  const int textHeight = 20;

  tft.fillRect(textX, textY, textWidth, textHeight, TFT_BLACK);
  tft.setCursor(textX, textY);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.print("Loading");

  int numDots = frame % 4;
  for (int j = 0; j < numDots; j++) {
    tft.print(".");
  }
}

// ======================== JPEG Load Wrapper ========================
void drawSdJpeg(const char *filename, int xpos, int ypos) {
  File jpegFile = SD.open(filename, FILE_READ);
  if (!jpegFile) {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  Serial.println("Drawing JPEG: " + String(filename));
  if (JpegDec.decodeSdFile(jpegFile)) {
    jpegRender(xpos, ypos);
  } else {
    Serial.println("JPEG decode failed!");
  }
}

// ======================== JPEG Render Core ========================
void jpegRender(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width + xpos;
  uint32_t max_y = JpegDec.height + ypos;
  uint32_t min_w = jpg_min(mcu_w, JpegDec.width % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, JpegDec.height % mcu_h);
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  while (JpegDec.read()) {
    pImg = JpegDec.pImage;
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    win_w = (mcu_x + mcu_w <= max_x) ? mcu_w : min_w;
    win_h = (mcu_y + mcu_h <= max_y) ? mcu_h : min_h;

    if (win_w != mcu_w) {
      uint16_t *cImg = pImg + win_w;
      int p = 0;
      for (int h = 1; h < win_h; h++) {
        p += mcu_w;
        for (int w = 0; w < win_w; w++) {
          *cImg++ = *(pImg + w + p);
        }
      }
    }

    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort();
  }

  tft.setSwapBytes(swapBytes);
}

// ======================== Speaker Start up noise ========================
void speaker() {
  ledcAttachPin(SPEAKER_PIN, 0); // Attach speaker pin to PWM channel 0

  int melody[] = { 440, 554, 659, 880 }; // A4, C#5, E5, A5 - simple ascending notes
  int noteDurations[] = { 150, 150, 150, 300 }; // durations in ms

  for (int i = 0; i < 4; i++) {
    ledcWriteTone(0, melody[i]);
    delay(noteDurations[i]);
  }
  ledcWriteTone(0, 0); // Stop tone
}
