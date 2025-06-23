// speaker.cpp

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

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// Initialize an array of Button structs with initial values for multiple
// buttons
Button buttons[] = {
    {21, "Speaker", 0, 0, false}, // speaker
};

// Calculate the number of buttons in the array
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

#define SPEAKER_PIN 21

// ###################### Setup ######################
void setup() {

  // Initialize serial communication for debugging
  Serial.begin(115200);

  printMessage("Testing speaker.");

  // Set PWM channel 0 for speaker output
  ledcAttachPin(SPEAKER_PIN, 0); // GPIO 21 to channel 0
  ledcWriteTone(0, 1000);        // 1000 Hz tone

  delay(500);          // Play for 500ms
  ledcWriteTone(0, 0); // Stop tone

  tft.init();
  tft.setRotation(3);
}

void loop() {

  //   delay(50); // Small delay to avoid overwhelming the loop
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