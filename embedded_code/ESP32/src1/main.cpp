// buttons.cpp

#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h>
#include <SD.h>

// ###################### Definitions ######################
// Function to print messages to the screen
void printMessage(String text);

// Function prototype for looping through buttons
void handleButtonInputs();

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Buttons Setup ######################
const int Button_Down_Right = 34;
const int Button_Up_Left = 35;
const int Button_A_B_Start = 36;

// ###################### Setup ######################
void setup() {

  // Initialize serial communication for debugging
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Testing Sound
  // ======================== Speaker Start up noise ========================
  ledcAttachPin(21, 0);
  int melody[] = {440, 554, 659,
                  880}; // A4, C#5, E5, A5 - simple ascending notes
  int noteDurations[] = {150, 150, 150, 300}; // durations in ms

  for (int i = 0; i < 4; i++) {
    ledcWriteTone(0, melody[i]);

    ledcWrite(0, 255);
    delay(noteDurations[i]);
  }
  ledcWrite(0, 0); // Stop tone

  tft.setTextSize(4);
  tft.setTextDatum(MC_DATUM);

  tft.drawString("<<< TESTING >>>", tft.width() /2, tft.height()/ 2);
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  
  // Testing SD Mounting
  if(!SD.begin(5)){}

  if (!SD.begin(5)) {
    tft.setTextColor(TFT_RED);
    tft.drawString("CARD MOUNT FAILED", tft.width() / 2, tft.height()/2);
    delay(1000);
    Serial.println("Card Mount Failed");
  }else{
    tft.setTextColor(TFT_GREEN);
    tft.drawString("CARD MOUNT SUCCESS", tft.width() / 2, tft.height()/2);
    delay(1000);
  }
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);

  // Set all button pins as input
  pinMode(Button_A_B_Start, INPUT);
  pinMode(Button_Down_Right, INPUT);
  pinMode(Button_Up_Left, INPUT);
}

// ###################### Loop ######################
void loop() {

  handleButtonInputs();

  delay(100); // Small delay to avoid overwhelming the loop
}

// ###################### Print Message ######################
#define MAX_LINES 6
String messageBuffer[MAX_LINES];

void printMessage(String text) {

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

  int y = (tft.height() / 2) - 40;
  int lineHeight = 35;
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);
  for (int i = 0; i < MAX_LINES; i++) {
    tft.drawString(messageBuffer[i], tft.width() / 2, y);
    y += lineHeight;
  }
}

// ###################### Handle Buttons Input ######################

void handleButtonInputs() {
  // Handle analog multi-direction buttons
  int val35 = analogRead(35);
  int val34 = analogRead(34);
  int val36 = analogRead(36);

  // ###################### BUTTON 36 ######################
  if ((val36 > 100) && (val36 < 2301)) {
    printMessage("A Pressed");
    printMessage(String(val36));
    printMessage("");

  } else if ((val36 > 2301) && (val36 < 3000)) {
    printMessage("Start Pressed");
    printMessage(String(val36));
    printMessage("");

  } else if ((val36 > 3001) && (val36 < 4096)) {
    printMessage("B Pressed");
    printMessage(String(val36));
    printMessage("");
  }

  // ###################### BUTTON 35 ######################
  if ((val35 > 100) && val35 < 3900) {
    printMessage("Up Pressed");
    printMessage(String(val35));
    printMessage("");

  } else if ((val35 > 3901) && (val35 < 4096)) {
    printMessage("Right Pressed");
    printMessage(String(val35));
    printMessage("");
  }

  // ###################### BUTTON 34 ######################
  if ((val34 > 2000) && (val34 < 3900)) {
    printMessage("Left Pressed");
    printMessage(String(val34));
    printMessage("");

  } else if ((val34 > 3901) && (val34 < 4096)) {
    printMessage("Down Pressed");
    printMessage(String(val34));
    printMessage("");
  }
}