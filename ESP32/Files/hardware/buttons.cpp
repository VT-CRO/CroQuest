// buttons.cpp

#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h>

// ###################### Button Structure ######################
struct Button {
  const int pin; // Pin where the button is connected
  const char *label;
  int state;     // Current button state
  int lastState; // Last button state
  bool isOn;     // ON/OFF state to represent if the button is toggled
};

// ###################### Definitions ######################
// Function to print messages to the screen
void printMessage(String text);

// Function prototype for looping through buttons
void handleButtonInputs();

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Buttons Setup ######################
Button buttons[] = {
    {22, "A", 0, 0, false}, // Button: A
    // {1, "B", 0, 0, false},          // Button: B tx0
    // {3, "Start", 0, 0, false},      // Button: start rx0
    {35, "Up/Right", 0, 0, false},  // up and right
    {34, "Left/Down", 0, 0, false}, // left and down
};

// Calculate the number of buttons in the array
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

// ###################### Setup ######################
void setup() {

  // Initialize serial communication for debugging
  Serial.begin(3600);

  tft.init();
  tft.setRotation(3);

  // Set the built-in LED pin as output
  // pinMode(BUILTIN_LED, OUTPUT);

  // Set all button pins as input
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT);
  }
}

// ###################### Loop ######################
void loop() {

  handleButtonInputs();

  delay(50); // Small delay to avoid overwhelming the loop
}

// ###################### Print Message ######################
#define MAX_LINES 6
String messageBuffer[MAX_LINES];

void printMessage(String text) {
  // Do NOT call setRotation repeatedly
  // tft.setRotation(3); ← REMOVE THIS

  // Shift buffer
  for (int i = 0; i < MAX_LINES - 1; i++) {
    messageBuffer[i] = messageBuffer[i + 1];
  }
  messageBuffer[MAX_LINES - 1] = text;

  // Redraw
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);

  int y = 10;
  int lineHeight = 20;

  for (int i = 0; i < MAX_LINES; i++) {
    tft.drawString(messageBuffer[i], 10, y);
    y += lineHeight;
  }
}

// ###################### Handle Buttons Input ######################

void handleButtonInputs() {
  // Handle analog multi-direction buttons
  // int val35 = analogRead(35);
  // int val34 = analogRead(34);

  // if (val35 > 4000) {
  //   printMessage("Right Pressed");
  // } else if (val35 > 3000) {
  //   printMessage("Up Pressed");
  // }

  // if (val34 > 4000) {
  //   printMessage("Down Pressed");
  // } else if (val34 > 3000) {
  //   printMessage("Left Pressed");
  // }

  // Handle digital buttons
  for (int i = 0; i < numButtons; i++) {
    // // Skip analog pins (already handled above)
    // if (buttons[i].pin == 34 || buttons[i].pin == 35)
    //   continue;

    buttons[i].state = digitalRead(buttons[i].pin);

    if (buttons[i].state == HIGH && buttons[i].lastState == LOW) {
      printMessage("Button \"" + String(buttons[i].label) + "\" Pressed");
      Serial.println("Button \"" + String(buttons[i].label) + "\" Pressed");

      while (digitalRead(buttons[i].pin) == HIGH) {
        delay(10); // prevent bouncing
      }
    }

    buttons[i].lastState = buttons[i].state;
  }

  // delay(200); // debounce
}