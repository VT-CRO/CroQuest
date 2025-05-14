#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <TouchScreen.h>
#include <WiFi.h> // Librería para manejar Wi-Fi

// Definición de los pines de la pantalla táctil
#define YP 33 // WR
#define XM 32 // DC
#define YM 14 // D7
#define XP 27 // D6
const int TS_LEFT = 3751, TS_RT = 798, TS_TOP = 3920, TS_BOT = 597;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

TFT_eSPI tft = TFT_eSPI(); // Instancia de la pantalla
TouchScreen ts =
    TouchScreen(XP, YP, XM, YM, 338); // Instancia de la pantalla táctil

TFT_eSPI_Button scanButton; // Botón para escanear Wi-Fi

int pixel_x, pixel_y; // Variables para almacenar coordenadas táctiles

// Funciones declaradas
bool Touch_getXY(void);
void showWiFiMenu();
void scanWiFiNetworks();

void setup() {
  delay(500);
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1); // HORIZONTAL
  tft.fillScreen(BLACK);
  tft.setFreeFont(&FreeMono9pt7b);

  // Inicializa el botón para escanear redes Wi-Fi
  scanButton.initButton(&tft, 150, 150, 100, 40, TFT_BLACK, TFT_NAVY, TFT_WHITE,
                        (char *)"WiFi", 1);
  scanButton.drawButton(false);

  WiFi.mode(WIFI_STA); // Modo Wi-Fi para escanear
  WiFi.disconnect();   // Desconectar de cualquier red para iniciar el escaneo
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

bool Touch_getXY(void) {
  static unsigned long lastPrint = 0;

  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);

  p.z = abs(p.z);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);

  if (pressed) {
    pixel_x = map(p.y, TS_BOT, TS_TOP, 0, tft.width());
    pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());

    // Limit printing to once every 100ms
    if (millis() - lastPrint > 100) {
      Serial.print("Touch at x: ");
      Serial.print(pixel_x);
      Serial.print(", y: ");
      Serial.println(pixel_y);
      lastPrint = millis();
    }
  }

  return pressed;
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
    tft.println("No networks found.");
  } else {
    tft.setCursor(10, 10);
    tft.setTextColor(WHITE);
    tft.printf("%d networks found:\n", n);

    for (int i = 0; i < n; ++i) {
      // Limitamos el número de redes mostradas a 5 para no saturar la pantalla
      if (i >= 5)
        break;
      tft.setCursor(10, 30 + (i * 20));
      tft.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(),
                 WiFi.RSSI(i));
    }
  }

  // Añade un botón para volver al menú principal
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
      setup(); // Vuelve a configurar la pantalla inicial
      break;
    }
  }
}
