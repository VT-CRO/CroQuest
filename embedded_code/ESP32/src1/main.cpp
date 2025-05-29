#include <Arduino.h>

const int ANALOG_BUTTON_PIN = 36;
const int ANALOG_BUTTON_UP_RIGHT = 35;
const int ANALOG_BUTTON_DOWN_LEFT = 34;
int lastZone = -1;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ANALOG_BUTTON_PIN, INPUT);
  pinMode(ANALOG_BUTTON_UP_RIGHT, INPUT);
  pinMode(ANALOG_BUTTON_DOWN_LEFT, INPUT);

  Serial.println("Button test started...");
}

void loop() {
  int value = analogRead(ANALOG_BUTTON_PIN);
  int up_right = analogRead(ANALOG_BUTTON_UP_RIGHT);
  int down_left = analogRead(ANALOG_BUTTON_DOWN_LEFT);
  int currentZone = -1;

  // Define button zones based on observed values
  if (value > 1650 && value < 1850) {
    Serial.println("Pressed A");
  } else if (value > 2400 && value < 2600) {
    currentZone = 2; // Button 2
    Serial.println("Pressed Start");
  } else if (value > 4000 && value <= 4095) {
    Serial.println("Pressed B");
  }

  if (up_right > 2000 && up_right < 3800) {
    Serial.println("Pressed UP");
  } else if (up_right > 3801 && up_right <= 4095) {
    Serial.println("Pressed RIGHT");
  }

  if (down_left > 2000 && down_left < 3800) {
    Serial.println("Pressed LEFT");
  } else if (down_left > 3801 && down_left <= 4095) {
    Serial.println("Pressed DOWN");
  }

  // // Print only when button is newly pressed
  // if (currentZone != -1 && currentZone != lastZone) {
  //   Serial.print(" pressed! Value: ");
  //   Serial.println(value);
  //   Serial.println(up_right);
  //   Serial.println(down_left);
  // }

  // // Reset zone when button is released
  // if (currentZone == -1) {
  //   lastZone = -1;
  // } else {
  //   lastZone = currentZone;
  // }

  delay(100); // Small delay for stability
}
