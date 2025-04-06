#include <TFT_eSPI.h>     // For TFT screen handling
#include <Button2.h>       // For button handling
#include <WiFi.h>          // For WiFi connectivity (if needed later)
#include <BluetoothSerial.h> // For Bluetooth connectivity (if needed later)

TFT_eSPI tft = TFT_eSPI();  // Create TFT object
Button2 buttonA(0);         // Example button setup (GPIO 0)
Button2 buttonB(1);         // Another button (GPIO 1)
Button2 buttonStart(2);     // Start button

int menuSelection = 0;      // Tracks the current selected menu option
const char* menuItems[] = {"Snake", "Pong", "Super Mario Bros"};

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);  // Set the display orientation (adjust as needed)
  tft.fillScreen(TFT_BLACK);

  buttonA.begin();
  buttonB.begin();
  buttonStart.begin();

  displayMenu();
}

void loop() {
  buttonA.loop();
  buttonB.loop();
  buttonStart.loop();

  // Handle button presses
  if (buttonA.wasPressed()) {
    navigateMenu(-1);  // Move up
  }
  if (buttonB.wasPressed()) {
    navigateMenu(1);   // Move down
  }
  if (buttonStart.wasPressed()) {
    selectGame();  // Start selected game
  }
}

void displayMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.print("Select a Game:");

  // Display menu options
  for (int i = 0; i < 3; i++) {
    if (i == menuSelection) {
      tft.setTextColor(TFT_YELLOW); // Highlight the selected game
    } else {
      tft.setTextColor(TFT_WHITE);
    }
    tft.setCursor(10, 50 + (i * 30));
    tft.print(menuItems[i]);
  }
}

void navigateMenu(int direction) {
  menuSelection += direction;
  if (menuSelection < 0) menuSelection = 2;  // Wrap around
  if (menuSelection > 2) menuSelection = 0;  // Wrap around
  displayMenu();
}

void selectGame() {
  if (menuSelection == 0) {
    startSnake();
  } else if (menuSelection == 1) {
    startPong();
  } else if (menuSelection == 2) {
    startSuperMarioBros();
  }
}

void startSnake() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 20);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.print("Starting Snake...");
  delay(1000);  // Simulate game startup
  // Add Snake game logic here
}

void startPong() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 20);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.print("Starting Pong...");
  delay(1000);  // Simulate game startup
  // Add Pong game logic here
}

void startSuperMarioBros() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 20);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.print("Starting Super Mario Bros...");
  delay(1000);  // Simulate game startup
  // Add Super Mario Bros game logic here
}

