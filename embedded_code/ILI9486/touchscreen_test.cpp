#include <XPT2046_Touchscreen.h>

// Adjust to your wiring!
#define CS_PIN  5
#define TIRQ_PIN  2  // optional, for interrupt

XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);

void setup() {
  Serial.begin(115200);
  ts.begin();
  ts.setRotation(1);

  Serial.println("Testing SPI Touch...");

  delay(500);
  if (ts.touched()) {
    Serial.println("XPT2046 detected (touched).");
  } else {
    Serial.println("XPT2046 may be connected but not touched yet.");
  }
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Touch at: ");
    Serial.print(p.x);
    Serial.print(", ");
    Serial.println(p.y);
  }
}

