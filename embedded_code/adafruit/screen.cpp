#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Pin definitions
#define TFT_MISO 12
#define TFT_LED  21
#define TFT_SCK  14
#define TFT_MOSI 13
#define TFT_DC   2
#define TFT_RESET 4
#define TFT_CS   15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);

unsigned long lastTime;
float fps;

void setup() {
  Serial.begin(9600);

  Serial.println("Start");

  // Backlight on
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Init display at faster SPI speed
  tft.begin(40000000); // 40 MHz
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  lastTime = millis();
}

void loop() {
  // Start time
  unsigned long frameStart = millis();

  // Fill screen with two alternating colors
  tft.fillScreen(ILI9341_MAGENTA);
  tft.fillScreen(ILI9341_BLUE);

  // Calculate FPS
  unsigned long frameEnd = millis();
  int frameDuration = (frameEnd - frameStart) / 1000.0;
  fps = 1.0 / frameDuration;

  // Draw FPS text at bottom of screen
  tft.fillRect(0, tft.height() - 20, tft.width(), 20, ILI9341_BLACK); // Clear previous text
  tft.setCursor(10, tft.height() - 18);
  tft.print("FPS: ");
  tft.println((int)frameEnd);
}

