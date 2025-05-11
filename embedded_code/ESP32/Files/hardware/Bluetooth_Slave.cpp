// Bluetooth_Slave.cpp

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

#define BUILTIN_LED 22

struct master {
  String name;
  String message;
  bool status;
  unsigned long lastSeen;
};

master connectedMaster = {"ESP32_Master", "", false};

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Bluetooth Setup ######################
BluetoothSerial SerialBT;

// ###################### Setup ######################
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Screen Initialization
  tft.init();
  tft.setRotation(2);

  // Set the built-in LED pin as output
  pinMode(BUILTIN_LED, OUTPUT);

  // Start Bluetooth communication in Slave mode with device name
  SerialBT.begin("ESP32_Slave");

  // Attempt to connect to the slave ESP32 device
  printMessage("ESP32 Slave Bluetooth Serial Started. \n Waiting for Master to "
               "connect... ");

  while (!SerialBT.hasClient()) {
    delay(1000); // Wait for 1 second before checking again
  }

  // Once connected, print a confirmation message and change status
  printMessage("Connected to " + connectedMaster.name);
  connectedMaster.status = true;

  // Turn on the built-in LED to indicate a successful connection
  // digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {

  checkConnectionStatus();

  // Check if data is received from the master device via Bluetooth
  if (SerialBT.available()) {
    // Read the incoming data as a string
    String incomingData = SerialBT.readString();
    printMessage("Received JSON: ");
    printMessage(incomingData);

    // Create a JSON document to hold the parsed data
    JsonDocument jsonDoc; // Adjust the size as needed

    // Attempt to deserialize the JSON data from the incoming string
    DeserializationError error = deserializeJson(jsonDoc, incomingData);

    // Check if deserialization was successful
    if (error) {
      printMessage("JSON Deserialization failed: ");
      printMessage(error.c_str()); // Print the error message
      return;                      // Exit the loop if there's an error
    }

    // Extract the pin number and status from the JSON document
    int pin_number = jsonDoc["pin_number"];
    bool pin_status = jsonDoc["pin_status"];

    // Set the specified pin as output
    pinMode(pin_number, OUTPUT);

    // Write the received status to the specified pin (HIGH or LOW)
    digitalWrite(pin_number, pin_status);

    // Send a confirmation back to the master device
    SerialBT.println("Command executed successfully");
  }

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
    connectedMaster.lastSeen = millis();
  }

  if (isConnected && !connectedMaster.status) {
    printMessage("Connection established successfully with " +
                 connectedMaster.name);
    connectedMaster.status = true;
    // digitalWrite(BUILTIN_LED, HIGH);
  }

  else if (!isConnected && connectedMaster.status) {
    printMessage("Connection with " + connectedMaster.name + " lost.");
    connectedMaster.status = false;
    // digitalWrite(BUILTIN_LED, LOW);

    // Prints how long ago the conneciton was lost.
    // unsigned long secondsSinceDisconnect =
    //     (millis() - connectedMaster.lastSeen) / 1000;
    // printMessage("Disconnected from " + connectedMaster.name + "\nLast seen "
    // +
    //              String(secondsSinceDisconnect) + " seconds ago.");
  }
}