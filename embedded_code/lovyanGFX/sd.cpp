#include <LovyanGFX.hpp>
#include <SD.h>
#include <SPI.h>

#define TFT_MISO 13 // 12
#define TFT_LED 21
#define TFT_SCK 12 // 14
#define TFT_MOSI 11 // 13
#define TFT_RESET 4
#define TFT_CS 15

// Screen pin
#define TFT_DC 2

// SD Card pin
#define SD_CS    8  // Change to your SD card CS pin

// Setup for ILI9341
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 panel;
  lgfx::Bus_SPI bus;

public:
  LGFX() {
    auto cfg = bus.config();
    cfg.spi_host = SPI3_HOST;
    cfg.spi_mode = 0;
    cfg.freq_write = 40000000;
    cfg.pin_sclk = 14;
    cfg.pin_mosi = 13;
    cfg.pin_miso = 12;
    cfg.pin_dc = TFT_DC;

    bus.config(cfg);
    panel.setBus(&bus);

    auto panel_cfg = panel.config();
    panel_cfg.pin_cs = TFT_CS;
    panel_cfg.pin_rst = TFT_RESET;
    panel_cfg.pin_busy = -1;
    panel_cfg.memory_width = 240;
    panel_cfg.memory_height = 320;
    panel_cfg.panel_width = 240;
    panel_cfg.panel_height = 320;
    panel_cfg.offset_x = 0;
    panel_cfg.offset_y = 0;
    panel.config(panel_cfg);

    setPanel(&panel);
  }
};

LGFX tft;

// List of image filenames
const char* imageFiles[] = {
  "/images/image1.jpg",
  "/images/image2.jpg",
  "/images/image3.jpg"
};
const int numImages = sizeof(imageFiles) / sizeof(imageFiles[0]);

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Use same SPI bus as TFT (MOSI=13, MISO=12, SCK=14)
  SPI.begin(14, 12, 13, SD_CS);  // Add SD_CS here for the SD card

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("SD card error!");
    while (true);  // Stay in a loop to show the error
  } else {
    Serial.println("SD card initialized.");
  }
}

void loop() {
  for (int i = 0; i < numImages; i++) {
    showImage(imageFiles[i]);
    delay(3000);  // Delay between images
  }
}

void showImage(const char* filename) {
  File file = SD.open(filename);
  if (!file) {
    Serial.print("Failed to open: ");
    Serial.println(filename);

    // Display error on screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Error opening file!");
    return;
  }

  Serial.println("File opened successfully");

  // Read the file into a byte array
  size_t fileSize = file.size();
  uint8_t* imgData = new uint8_t[fileSize];
  size_t bytesRead = file.read(imgData, fileSize);
  file.close();

  // If reading the file fails, show an error message
  if (bytesRead != fileSize) {
    Serial.println("Error reading file!");

    // Display static image as fallback
    tft.fillRect(10, 10, 100, 50, TFT_RED);  // Draw a red rectangle

    // Display error on screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Error reading file!");
    tft.println("Try again later.");
    delete[] imgData;
    return;
  }

  // Clear the screen and display the image
  tft.fillScreen(TFT_BLACK);
  bool success = tft.drawJpg(imgData, fileSize, 0, 0);  // Draw image at (0, 0)

  if (!success) {
    Serial.println("Failed to display the image!");

    // Display error on screen if image could not be shown
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Error displaying image!");
    delete[] imgData;
    return;
  }

  // Free the allocated memory
  delete[] imgData;

  Serial.print("Displayed: ");
  Serial.println(filename);
}

