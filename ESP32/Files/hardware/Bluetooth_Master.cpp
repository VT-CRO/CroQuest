// Bluetooth_Master.cpp

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include <ArduinoJson.h>
#include <BluetoothSerial.h>

// Function to print messages to the screen
void printMessage(String text);

// Function prototype for looping through buttons
void loopThroughButton(String received);

// Function to check connection while running
void checkConnectionStatus();

struct slave {
  String name;
  String message;
  bool status;
  unsigned long lastSeen;
};

slave connectedSlave = {"ESP32_Slave", "", false};

#define BUILTIN_LED 22

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Bluetooth Setup ######################
BluetoothSerial SerialBT;

// ###################### Button Setup ######################

// Define a struct to encapsulate button-related variables
struct Button {
  const int pin; // Pin where the button is connected
  int state;     // Current button state
  int lastState; // Last button state
  bool isOn;     // ON/OFF state to represent if the button is toggled
};

// Initialize an array of Button structs with initial values for multiple
// buttons
Button buttons[] = {
    {22, 0, 0, false}, // Button 1: connected to pin 22
                       // {5, 0, 0, false}, // Button 2: connected to pin 5
                       // {15, 0, 0, false} // Button 3: connected to pin 15
};

// Calculate the number of buttons in the array
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

// ###################### Setup ######################
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  tft.init();
  tft.setRotation(2);

  // Set the built-in LED pin as output
  pinMode(BUILTIN_LED, OUTPUT);

  // Set all button pins as input
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT);
  }

  // Start Bluetooth communication in Master mode with device name
  // "ESP32_Master" Passing 'true' as the second argument enables master mode
  SerialBT.begin("ESP32_Master", true);

  printMessage("ESP32 Master Bluetooth Serial Started. \n Discoverable as "
               "ESP32_Master.");

  // Attempt to connect to the slave ESP32 device
  while (!SerialBT.connect("ESP32_Slave")) {
    printMessage("Trying to connect to ESP32_Slave...");
    delay(1000); // Wait for 1 second before retrying
  }

  // Once connected, print a confirmation message and change status
  printMessage("Connected to " + connectedSlave.name);
  connectedSlave.status = true;

  // Turn on the built-in LED to indicate a successful connection
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {

  checkConnectionStatus();

  // Check if data is received from the slave device
  if (connectedSlave.status && SerialBT.available()) {
    connectedSlave.message = SerialBT.readStringUntil('\n');
    printMessage("Received from Slave: " + connectedSlave.message);
  }

  // Call the function to handle button state changes
  loopThroughButton(connectedSlave.message);

  delay(50); // Small delay to avoid overwhelming the loop
}

// ###################### Print Message ######################

void printMessage(String text) {
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);

  int y = 10;
  int lineHeight = 20;
  int maxHeight = tft.height() - lineHeight;

  String msg(text);
  int idx = 0;

  while (idx < msg.length() && y <= maxHeight) {
    int lineEnd = msg.indexOf('\n', idx);
    if (lineEnd == -1)
      lineEnd = msg.length();

    String line = msg.substring(idx, lineEnd);
    tft.drawString(line, 10, y);
    y += lineHeight;

    idx = lineEnd + 1;
  }
}

// ###################### Check For Connection ######################

void checkConnectionStatus() {

  bool isConnected = SerialBT.hasClient();

  if (isConnected) {
    connectedSlave.lastSeen = millis();
  }

  if (isConnected && !connectedSlave.status) {
    printMessage("Connection established successfully with " +
                 connectedSlave.name);
    connectedSlave.status = true;
    // digitalWrite(BUILTIN_LED, HIGH);
  }

  else if (!isConnected && connectedSlave.status) {
    printMessage("Connection with " + connectedSlave.name + " lost.");
    connectedSlave.status = false;
    // digitalWrite(BUILTIN_LED, LOW);

    // Attempt reconnection if disconnected
    if (!isConnected) {
      static unsigned long lastAttempt = 0;
      unsigned long now = millis();
      if (now - lastAttempt > 3000) {
        printMessage("Attempting reconnection to " + connectedSlave.name +
                     "...");
        if (SerialBT.connect(connectedSlave.name.c_str())) {
          printMessage("Reconnection Successful!");
          connectedSlave.status = true;
          digitalWrite(BUILTIN_LED, HIGH);
        } else {
          printMessage("Reconnection failed. ");
        }
        lastAttempt = now;
      }
    }

    // Prints how long ago the conneciton was lost.
    // unsigned long secondsSinceDisconnect =
    //     (millis() - connectedMaster.lastSeen) / 1000;
    // printMessage("Disconnected from " + connectedMaster.name + "\nLast seen "
    // +
    //              String(secondsSinceDisconnect) + " seconds ago.");
  }
}

// ###################### Checks All Buttons ######################

void loopThroughButton(String received) {
  // Loop through each button in the array
  for (int i = 0; i < numButtons; i++) {
    // Read the current state of the button
    buttons[i].state = digitalRead(buttons[i].pin);

    // Check if the button state has changed from its last recorded state
    if (buttons[i].state != buttons[i].lastState) {
      // If the button state has changed to HIGH, it indicates a button press
      if (buttons[i].state == HIGH) {
        // Toggle the ON/OFF state of the button
        buttons[i].isOn = !buttons[i].isOn;

        // Print the current state to the Serial Monitor
        printMessage("Received from Slave: " + connectedSlave.message);
        printMessage("Button pin " + String(buttons[i].pin) + ": " +
                     (buttons[i].isOn ? "ON" : "OFF"));

        // Create a JSON document to send the button state
        JsonDocument jsonDoc;
        // Add the pin number and its state to the JSON document
        jsonDoc["pin_number"] = buttons[i].pin;
        jsonDoc["pin_status"] = buttons[i].isOn;

        // Serialize the JSON document into a string
        String jsonString;
        serializeJson(jsonDoc, jsonString);

        // Send the JSON string to the slave device via Bluetooth
        SerialBT.println(jsonString);
      }

      // Small delay to debounce the button press
      delay(50);
    }

    // Update the last state of the button for the next loop iteration
    buttons[i].lastState = buttons[i].state;
  }
}