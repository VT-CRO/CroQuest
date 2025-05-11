#include <Arduino.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
void drawSdJpeg(const char *filename, int xpos, int ypos);
void jpegRender(int xpos, int ypos);

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {
  Serial.begin(115200);

  // Set all chip selects high to avoid bus contention during initialisation of
  // each peripheral
  digitalWrite(22, HIGH); // Touch controller chip select (if used)
  digitalWrite(15, HIGH); // TFT screen chip select
  digitalWrite(5, HIGH);  // SD card chips select, must use GPIO 5 (ESP32 SS)

  tft.begin();

  tft.setRotation(1); //portrait

  if (!SD.begin(5)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  
  uint16_t color = tft.color565(0xFF, 0x70, 0x00);
  tft.fillScreen(color);
  
  // Get the screen dimensions
  int screenWidth = tft.width();
  int screenHeight = tft.height();
  
  // First, we need to get the image dimensions from the JPEG file
  // This requires us to decode the JPEG header
  File jpegFile = SD.open("/board.jpg", FILE_READ);
  if (!jpegFile) {
    Serial.println("ERROR: Cannot open board.jpg");
    return;
  }
  
  bool decoded = JpegDec.decodeSdFile(jpegFile);
  if (!decoded) {
    Serial.println("Error decoding JPEG");
    return;
  }
  
  // Get the image dimensions
  uint32_t imageWidth = JpegDec.width;
  uint32_t imageHeight = JpegDec.height;
  
  // Calculate the position to center the image
  int xpos = (screenWidth - imageWidth) / 2;
  int ypos = (screenHeight - imageHeight) / 2;
  
  // Make sure positions aren't negative
  xpos = max(0, xpos);
  ypos = max(0, ypos);
  
  Serial.print("Screen dimensions: ");
  Serial.print(screenWidth);
  Serial.print(" x ");
  Serial.println(screenHeight);
  
  Serial.print("Image dimensions: ");
  Serial.print(imageWidth);
  Serial.print(" x ");
  Serial.println(imageHeight);
  
  Serial.print("Centering at position: (");
  Serial.print(xpos);
  Serial.print(", ");
  Serial.print(ypos);
  Serial.println(")");
  
  // Draw the centered image
  drawSdJpeg("/board.jpg", xpos, ypos);
}

// ####################################################################################################
//  Main loop
// ####################################################################################################
void loop() {
}

// ####################################################################################################
//  Draw a JPEG on the TFT pulled from SD Card
// ####################################################################################################
//  xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile =
      SD.open(filename, FILE_READ); // or, file handle reference for SD library

  if (!jpegFile) {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  // Use one of the following methods to initialise the decoder:
  bool decoded =
      JpegDec.decodeSdFile(jpegFile); // Pass the SD file handle to the decoder,
  // bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename
  // (String or character array)

  if (decoded) {
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  } else {
    Serial.println("Jpeg file format not supported!");
  }
}

// ####################################################################################################
//  Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if
//  they do not fit
// ####################################################################################################
//  This function assumes xpos,ypos is a valid screen coordinate. For
//  convenience images that do not fit totally on the screen are cropped to the
//  nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Create a sprite big enough for the whole image
  TFT_eSprite sprite = TFT_eSprite(&tft);
  sprite.setColorDepth(8);
  if (!sprite.createSprite(max_x, max_y)) {
    Serial.println("ERROR: Sprite allocation failed!");
    return;
  }

  // Ensure correct byte order for RGB565
  bool swapBytes = sprite.getSwapBytes();
  sprite.setSwapBytes(true);

  // MCU edge handling
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  while (JpegDec.read()) {
    pImg = JpegDec.pImage;
    int mcu_x = JpegDec.MCUx * mcu_w;
    int mcu_y = JpegDec.MCUy * mcu_h;

    if (mcu_x + mcu_w <= max_x) win_w = mcu_w; else win_w = min_w;
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h; else win_h = min_h;

    if (win_w != mcu_w) {
      uint16_t *cImg = pImg + win_w;
      int p = 0;
      for (int h = 1; h < win_h; h++) {
        p += mcu_w;
        for (int w = 0; w < win_w; w++) {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // Draw to the sprite (not directly to TFT)
    sprite.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
  }

  // Push full image to the screen
  sprite.pushSprite(xpos, ypos);

  // Clean up
  sprite.setSwapBytes(swapBytes);
  sprite.deleteSprite();
}