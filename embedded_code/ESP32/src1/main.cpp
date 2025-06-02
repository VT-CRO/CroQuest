#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TouchScreen.h>
#include <WiFi.h>

// Touchscreen pin mapping (shared with TFT)
#define YP 33 // WR
#define XM 32 // RS
#define YM 14 // D7
#define XP 27 // D6

// Raw calibration values for landscape orientation
const int TS_LEFT = 3751, TS_RT = 798, TS_TOP = 3920, TS_BOT = 597;

#define MINPRESSURE 100
#define MAXPRESSURE 10000

// Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Display & touch instances
TFT_eSPI tft = TFT_eSPI();
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 338);
TFT_eSPI_Button scanButton;

int pixel_x, pixel_y;

// Function declarations
void showMainMenu();
void showWiFiMenu();
void scanWiFiNetworks();
bool Touch_getXY();

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3); // Landscape
  tft.setFreeFont(&FreeMono9pt7b);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  showMainMenu();
}

void loop() {
  bool down = Touch_getXY();
  scanButton.press(down && scanButton.contains(pixel_x, pixel_y));

  if (scanButton.justReleased()) {
    scanButton.drawButton();
  }

  if (scanButton.justPressed()) {
    scanButton.drawButton(true);
    showWiFiMenu();
  }
}

void showMainMenu() {
  tft.fillScreen(BLACK);
  scanButton.initButton(&tft, 150, 150, 100, 40, TFT_BLACK, TFT_NAVY, TFT_WHITE,
                        (char *)"WiFi", 1);
  scanButton.drawButton(false);
}

void showWiFiMenu() {
  tft.fillScreen(BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);
  tft.println("Scanning for WiFi networks...");
  scanWiFiNetworks();
}

void scanWiFiNetworks() {
  int n = WiFi.scanNetworks();
  tft.fillScreen(BLACK);

  if (n == 0) {
    tft.setCursor(10, 10);
    tft.println("No networks found.");
  } else {
    tft.setCursor(10, 10);
    tft.setTextColor(WHITE);
    tft.printf("%d networks found:\n", n);

    for (int i = 0; i < n && i < 5; ++i) {
      tft.setCursor(10, 30 + (i * 20));
      tft.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(),
                 WiFi.RSSI(i));
    }
  }

  scanButton.initButton(&tft, 372, 160, 100, 40, TFT_BLACK, TFT_NAVY, TFT_WHITE,
                        (char *)"Atras", 1);
  scanButton.drawButton(false);

  while (true) {
    bool down = Touch_getXY();
    scanButton.press(down && scanButton.contains(pixel_x, pixel_y));

    if (scanButton.justReleased()) {
      scanButton.drawButton();
    }

    if (scanButton.justPressed()) {
      scanButton.drawButton(true);
      showMainMenu();
      break;
    }
  }
}

bool Touch_getXY() {
  // Tri-state TFT pins
  pinMode(XP, INPUT);
  pinMode(YP, INPUT);
  pinMode(XM, INPUT);
  pinMode(YM, INPUT);
  delayMicroseconds(10); // allow voltages to settle

  TSPoint p = ts.getPoint();

  // Restore TFT pins
  pinMode(XP, OUTPUT);
  digitalWrite(XP, HIGH);
  pinMode(YP, OUTPUT);
  digitalWrite(YP, HIGH);
  pinMode(XM, OUTPUT);
  digitalWrite(XM, HIGH);
  pinMode(YM, OUTPUT);
  digitalWrite(YM, HIGH);

  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);

  if (pressed) {
    pixel_x = map(p.y, TS_BOT, TS_TOP, 0, tft.width());
    pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());
    Serial.printf("Touch → Raw x:%d y:%d z:%d | Mapped x:%d y:%d\n", p.x, p.y,
                  p.z, pixel_x, pixel_y);
  }

  return pressed;
}
