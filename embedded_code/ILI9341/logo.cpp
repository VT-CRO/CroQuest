#include <LovyanGFX.hpp>
#include <SD.h>
#include <JPEGDecoder.h>

// Define Pins
#define TFT_MISO 13
#define TFT_LED 21
#define TFT_SCK 12
#define TFT_MOSI 11
#define TFT_RESET 4
#define TFT_CS 15

// Screen pin
#define TFT_DC 2

// SD pin
#define SD_CS 8

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI3_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = TFT_SCK;
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = TFT_MISO;
      cfg.pin_dc   = TFT_DC;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    {
      auto cfg = _panel.config();
      cfg.pin_cs   = TFT_CS;
      cfg.pin_rst  = TFT_RESET;
      cfg.pin_busy = -1;

      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable    = true;
      cfg.invert      = false;
      cfg.rgb_order   = false;
      cfg.dlen_16bit  = false;
      cfg.bus_shared  = true;

      _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

LGFX tft;

void printMessage(const char* message) {
  tft.fillScreen(TFT_BLACK);           // Clear screen before showing a new message
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(message);                // Display message
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);  // turn on backlight

  tft.init();
  tft.setRotation(3);

  // Print a message indicating the start of the process
  printMessage("Initializing SD card...");

  // Check if SPI is properly initialized
  Serial.println("Initializing SPI...");
  SPI.begin(12, 13, 11, SD_CS); // Add SD_CS here for the SD card
  Serial.println("SPI initialized.");

    // Add a short delay before attempting SD card initialization
  delay(500);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    tft.println("SD card error!");
    while(true); // Stay in a loop to show the error
  } else {
  printMessage("SD card initialized.");
  }

  File jpgFile = SD.open("/vtcro_logo.jpg");
  if (!jpgFile) {
    printMessage("File not found!");
    return;
  }

  printMessage("File found, reading image...");

  // Use JPEGDecoder to decode and display the image
  if (JpegDec.decodeSdFile("/vtcro_logo.jpg") != 0) {
    printMessage("JPEG decode failed!");
    return;
  }

  // Retrieve image dimensions
  int width = JpegDec.width;
  int height = JpegDec.height;
  
  // Display the image
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (JpegDec.read() == 0) {
        // Access the current RGB pixel
        uint8_t r = JpegDec.pImage[(y * width + x) * 3];     // Red component
        uint8_t g = JpegDec.pImage[(y * width + x) * 3 + 1]; // Green component
        uint8_t b = JpegDec.pImage[(y * width + x) * 3 + 2]; // Blue component

        uint16_t color = tft.color565(r, g, b); // Convert to 16-bit color
        tft.drawPixel(x, y, color);  // Draw the pixel
      } else {
        printMessage("Error reading pixel!");
        Serial.println("Error reading pixel data!");
        return;
      }
    }
  }

  jpgFile.close();
  printMessage("Image displayed.");
}

void loop() {}

