#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ###################### Constants ######################
#define ITEMS_PER_ROW 3
#define ITEM_COUNT 6

// #define PRESS_THRESHOLD 0.60 // anything below = noise
#define UP_LEFT_MIN 0.50
#define UP_LEFT_MAX 0.90
#define RIGHT_DOWN_MIN 0.91

// ###################### Global Variables ######################
int selectedIndex = 0;

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Button Structure ######################
struct Button {
  const int pin;
  const char *label;
  int state;
  int lastState;
  bool isOn;
};

// ###################### Buttons ######################
Button buttons[] = {{22, "A", 0, 0, false},
                    {35, "Up/Right", 0, 0, false},
                    {34, "Left/Down", 0, 0, false}};

const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

// ###################### Function Prototypes ######################
void printMessage(String text);
void handleInput();
bool isAButtonPressed();
bool isStartButtonPressed();
bool isBButtonPressed();

// ###################### Setup ######################
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT);
  }

  printMessage("Ready.\nUse buttons to navigate.");
}

// ###################### Loop ######################
void loop() {
  Serial.print("Button 22: ");
  Serial.println(digitalRead(22));

  handleInput();
  delay(50);
}

// ###################### Print Message ######################
void printMessage(String text) {
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);

  int y = 10;
  int lineHeight = 20;
  int maxLines = 3; // Show up to 3 lines
  int lineCount = 0;

  int idx = 0;
  while (idx < text.length() && lineCount < maxLines) {
    int lineEnd = text.indexOf('\n', idx);
    if (lineEnd == -1)
      lineEnd = text.length();

    String line = text.substring(idx, lineEnd);
    tft.drawString(line, 10, y);
    y += lineHeight;
    idx = lineEnd + 1;
    lineCount++;
  }
}

// ###################### Input Handling ######################
void handleInput() {
  static unsigned long lastInput = 0;
  if (millis() - lastInput < 200)
    return;

  bool moved = false;

  float val35 = analogRead(35) / 4095.0f;
  float val34 = analogRead(34) / 4095.0f;

  Serial.println(analogRead(35));
  Serial.println(analogRead(34));

  String debugText =
      "35: " + String(analogRead(35)) + "\n34: " + String(analogRead(34));
  printMessage(debugText);

  // UP
  if (val35 > UP_LEFT_MIN && val35 < UP_LEFT_MAX) {
    Serial.println("Up Pressed");
    printMessage("Up Pressed");
    if (selectedIndex >= ITEMS_PER_ROW) {
      selectedIndex -= ITEMS_PER_ROW;
      moved = true;
    }
  }

  // RIGHT
  else if (val35 > RIGHT_DOWN_MIN) {
    Serial.println("Right Pressed");
    printMessage("Right Pressed");
    if ((selectedIndex % ITEMS_PER_ROW) < (ITEMS_PER_ROW - 1) &&
        selectedIndex + 1 < ITEM_COUNT) {
      selectedIndex++;
      moved = true;
    }
  }

  // LEFT
  // if (val34 > UP_LEFT_MIN && val34 < UP_LEFT_MAX) {
  if (val34 > UP_LEFT_MIN && val34 < UP_LEFT_MAX) {
    Serial.println("Left Pressed");
    printMessage("Left Pressed");
    if ((selectedIndex % ITEMS_PER_ROW) > 0) {
      selectedIndex--;
      moved = true;
    }
  }

  // DOWN
  else if (val34 > RIGHT_DOWN_MIN) {
    Serial.println("Down Pressed");
    printMessage("Down Pressed");
    if (selectedIndex + ITEMS_PER_ROW < ITEM_COUNT) {
      selectedIndex += ITEMS_PER_ROW;
      moved = true;
    }
  }

  // Handle digital buttons
  if (isAButtonPressed()) {
    printMessage("A Button Pressed - Select");
    // lastInput = millis();
  } else if (isStartButtonPressed()) {
    printMessage("Start Pressed - return to menu.");
    // lastInput = millis();
  } else if (isBButtonPressed()) {
    printMessage("B Pressed - future feature.");
    // lastInput = millis();
  }
}

// ###################### Button State Checkers ######################
bool isAButtonPressed() { return digitalRead(22) == HIGH; }

bool isStartButtonPressed() {
  return false; // Placeholder for button pin (e.g., pin 3)
}

bool isBButtonPressed() {
  return false; // Placeholder for button pin (e.g., pin 1)
}
