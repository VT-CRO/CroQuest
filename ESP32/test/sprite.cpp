#include <Arduino.h>
#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void jpegRenderDirectFixed(int xpos, int ypos) {
  while (JpegDec.read()) {
    uint16_t *pImg = JpegDec.pImage;
    int mcuX = JpegDec.MCUx * JpegDec.MCUWidth + xpos;
    int mcuY = JpegDec.MCUy * JpegDec.MCUHeight + ypos;
    int mcuW = JpegDec.MCUWidth;
    int mcuH = JpegDec.MCUHeight;

    if ((mcuX + mcuW) >= tft.width())
      mcuW = tft.width() - mcuX;
    if ((mcuY + mcuH) >= tft.height())
      mcuH = tft.height() - mcuY;

    if (mcuX < tft.width() && mcuY < tft.height()) {
      tft.pushImage(mcuX, mcuY, mcuW, mcuH, pImg);
    }
  }
}

void drawCenteredBottomJpeg(const char *filename) {
  File jpegFile = SD.open(filename);
  if (!jpegFile) {
    Serial.println("File open failed");
    return;
  }

  // ✅ Correct way to fix inverted colors for older library versions
  JpegDec.readSwappedBytes() = true;

  if (JpegDec.decodeSdFile(jpegFile)) {
    int xOffset = (tft.width() - JpegDec.width) / 2;
    int yOffset = tft.height() - JpegDec.height;
    jpegRenderDirectFixed(xOffset, yOffset);
  } else {
    Serial.println("JPEG decode failed");
  }

  jpegFile.close();
}

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }

  drawCenteredBottomJpeg("/menu/assets/connect.jpg");
}

void loop() {}
