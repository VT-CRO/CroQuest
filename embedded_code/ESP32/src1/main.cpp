#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <TouchScreen.h>
#include <WiFi.h> // Librería para manejar Wi-Fi
#define YP 33     // Analog input
#define XM 32     // Analog input
#define YM 14     // Digital output
#define XP 27     // Digital output
#define RXPLATE 338

TouchScreen ts = TouchScreen(XP, YP, XM, YM, RXPLATE);

void setup() {
  Serial.begin(115200);
  Serial.println("Touch test starting...");
}

void loop() {
  TSPoint p = ts.getPoint();

  // Restore pin mode after getPoint() overrides it
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);

  if (p.z > 10 && p.z < 1000) {
    Serial.printf("Touch detected: x=%d y=%d z=%d\n", p.x, p.y, p.z);
  }

  delay(100);
}