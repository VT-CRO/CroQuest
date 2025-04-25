#include <LovyanGFX.hpp>
#include <SPI.h>
#include <SD.h>
#include <TJpg_Decoder.h>

// Define Pins for screen
#define TFT_MISO 13
#define TFT_LED 21
#define TFT_SCK 12
#define TFT_MOSI 11
#define TFT_RESET 4
#define TFT_CS 15
#define TFT_DC 2

// SD card CS pin
// #define SD_MISO 37
// #define SD_SCK 36
// #define SD_MOSI 35
#define SD_CS 8
#define SD_MISO TFT_MISO
#define SD_SCK  TFT_SCK
#define SD_MOSI TFT_MOSI

/**************************************************************************************/
// Creates communication bridge in between the library and the screen
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

/**************************************************************************************/

SPIClass sdSPI(FSPI);  // Use FSPI (VSPI is 2nd SPI, FSPI is primary on ESP32-S3)

// Format to print messages on the screen using LovyanGFX
void printMessage(const char* message) {
  tft.fillScreen(TFT_BLACK);           // Clear screen before showing a new message
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println(message);                // Display message
}

// Function to print directories in the SD
void printDirectory(File dir) {
  int yPos = 30;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    tft.setCursor(10, yPos);
    tft.print(entry.name());

    if (entry.isDirectory()) {
      tft.println("/");
    } else {
      tft.print("  ");
      tft.println(entry.size());
    }

    entry.close();
    yPos += 20;

    if (yPos >= 320) break;  // Stop if we run out of vertical space
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give time for Serial to wake up

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);  // turn on backlight

  tft.init();
  tft.setRotation(3);

  printMessage("Initializing TFT done.");
  delay(3600);

  printMessage("Initializing SD card...");
  delay(3600);


  // Initialize SPI and SD card (SCK = 12, MISO = 13, MOSI = 11)
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  // SCK, MISO, MOSI, CS
  Serial.println("SPI started I guess");
  printMessage("SPI started.");    
  delay(3600);

  printMessage("changing pinModes");
  Serial.println("Changing PinModes");
  pinMode(SD_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);    // Ensure TFT is deselected
  digitalWrite(SD_CS, LOW);      // Select SD card briefly
  delay(10);
  digitalWrite(SD_CS, HIGH);     // Deselect before init
  delay(10);


  Serial.println("Attempting to detect SD card...");
  if (!SD.begin(SD_CS, sdSPI)) {
    printMessage("SD card failed!");
    Serial.println("SD card init failed!");
    while (true) delay(0);
  }

  printMessage("GOT IT!! SD card initialized.");
  Serial.println("You got it?");

  sdcard_type_t cardType = SD.cardType();
  Serial.print("Card type: ");
  Serial.println(cardType);


  // Open the root directory
  File root = SD.open("/");
  if (!root) {
    printMessage("Failed to open root directory!");
    return;
  }
  Serial.println("I guess is reading directories");
  printMessage("Reading directory...");

  printDirectory(root);
}


void loop() {}




