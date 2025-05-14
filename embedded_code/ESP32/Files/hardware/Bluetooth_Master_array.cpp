// Bluetooth_Master.cpp

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include <ArduinoJson.h>
#include <BluetoothSerial.h>

// ###################### Slave Structure ######################
struct Slave {
  String name;
  String message;
  bool status;
  unsigned long lastSeen;
};

Slave connectedSlave = {"ESP32_Slave", "", false};

// ###################### Button Structure ######################
struct Button {
  const int pin; // Pin where the button is connected
  int state;     // Current button state
  int lastState; // Last button state
  bool isOn;     // ON/OFF state to represent if the button is toggled
};

// ###################### Player Structure ######################
struct Player {
  String id;       // Player ID
  char symbol;     // 'X' or 'O'
  int score;       // Score
  String initials; // Player Initials
};

// ###################### Player Structure ######################
struct GameState {
  Player host;
  Player guest;
  int bestOf;
  char turn;          // 'X' or 'O'
  String board[9];    // "**", "x5", etc.
  int cursorPosition; // 0-8
};

// ###################### Definitions ######################
// Function to print messages to the screen
void printMessage(String text);

// Function prototype for looping through buttons
void handleButtonInputs();

// Function to check connection while running
void checkConnectionStatus();

// Parse the Game State
GameState parseGameState(String state);

// Define Game State
GameState gameState;

// Build Game State
String buildGameState();

// Send Game State
void sendGameStateToSlave();

// Setup New Game
void setupNewGame();

// #define BUILTIN_LED 22

// ###################### TFT Setup ######################
TFT_eSPI tft = TFT_eSPI();

// ###################### Bluetooth Setup ######################
BluetoothSerial SerialBT;

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
  // pinMode(BUILTIN_LED, OUTPUT);

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

  setupNewGame();

  // Turn on the built-in LED to indicate a successful connection
  // digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {

  checkConnectionStatus();

  if (connectedSlave.status) {
    handleButtonInputs();
  }

  String command = "";

  // Check if data is received from the slave device
  if (connectedSlave.status && SerialBT.available()) {
    command = SerialBT.readStringUntil('\n');
    printMessage("Received from Slave: " + command);
  }

  if (command.startsWith("move@")) {
    int movePos = command.substring(5).toInt();
    // TODO:
    // Apply move to game state here
    // Update board array, scores, etc.
    // Then send updated game state
    String updateState = buildGameState(); // TODO:
    SerialBT.println(updateState);
    printMessage(updateState);
  }

  else if (command.startsWith("cursor@")) {
    String dir = command.substring(7);
    // TODO:
    // Move cursor left/right/up/down as needed
    // Then send updated state
    String updatedState = buildGameState();
    SerialBT.println(updatedState);
    printMessage(updatedState);
  }

  delay(50); // Small delay to avoid overwhelming the loop
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
          // digitalWrite(BUILTIN_LED, HIGH);
        } else {
          printMessage("Reconnection failed. ");
        }
        lastAttempt = now;
      }
    }

    // Prints how long ago the conneciton was lost.
    // unsigned long secondsSinceDisconnect =
    //     (millis() - connectedMaster.lastSeen) / 1000;
    // printMessage("Disconnected from " + connectedMaster.name + "\nLast seen
    // "
    // +
    //              String(secondsSinceDisconnect) + " seconds ago.");
  }
}

// ###################### Handle Buttons Input ######################
void handleButtonInputs() {
  // Loop through each button in the array
  for (int i = 0; i < numButtons; i++) {
    // Read the current state of the button
    buttons[i].state = digitalRead(buttons[i].pin);

    // Check if the button state has changed from its last recorded state
    if (buttons[i].state == HIGH && buttons[i].lastState == LOW) {
      String moveCommand = "move@" + String(gameState.cursorPosition);
      SerialBT.println(moveCommand);

      // Optional: wait until button released
      while (digitalRead(buttons[i].pin) == HIGH) {
        delay(10); // prevents flooding with multiple moves
      }
    }

    delay(50); // debounce

    // Update the last state of the button for the next loop iteration
    buttons[i].lastState = buttons[i].state;
  }
}

// ###################### Parse Game State ######################
GameState parseGameState(String state) {
  GameState gs;
  state.remove(0, 4); // Remove "ttt@"

  int idx = 0;
  String parts[7];
  while (idx < 7) {
    int end = state.indexOf(';');
    parts[idx++] = state.substring(0, end);
    state = state.substring(end + 1);
  }

  // Parse host
  String hostFields[4];
  int hIdx = 0;
  for (int i = 0; i < 4; i++) {
    int comma = parts[0].indexOf(',');
    hostFields[i] = parts[0].substring(0, comma);
    parts[0] = parts[0].substring(comma + 1);
  }
  gs.host.id = hostFields[0];
  gs.host.symbol = hostFields[1].charAt(0);
  gs.host.score = hostFields[2].toInt();
  gs.host.initials = hostFields[3];

  // Parse guest
  String guestFields[4];
  for (int i = 0; i < 4; i++) {
    int comma = parts[1].indexOf(',');
    guestFields[i] = parts[1].substring(0, comma);
    parts[1] = parts[1].substring(comma + 1);
  }
  gs.guest.id = guestFields[0];
  gs.guest.symbol = guestFields[1].charAt(0);
  gs.guest.score = guestFields[2].toInt();
  gs.guest.initials = guestFields[3];

  // Parse best of
  gs.bestOf = parts[2].toInt();

  // Turn
  gs.turn = parts[3].charAt(0);

  // Board
  int bIdx = 0;
  while (parts[4].length() > 0 && bIdx < 9) {
    int comma = parts[4].indexOf(',');
    if (comma == -1) {
      gs.board[bIdx++] = parts[4];
      break;
    }
    gs.board[bIdx++] = parts[4].substring(0, comma);
    parts[4] = parts[4].substring(comma + 1);
  }

  // Cursor
  gs.cursorPosition = parts[5].toInt();

  return gs;
}

// ###################### Build Game State ######################
String buildGameState() {
  String result = "ttt@";

  // Host
  result += gameState.host.id + "," + gameState.host.symbol + "," +
            String(gameState.host.score) + "," + gameState.host.initials + ";";

  // Guest
  result += gameState.guest.id + "," + gameState.guest.symbol + "," +
            String(gameState.guest.score) + "," + gameState.guest.initials +
            ";";

  result += String(gameState.bestOf) + ";";
  result += gameState.turn;
  result += ";";

  for (int i = 0; i < 9; i++) {
    result += gameState.board[i];
    if (i < 8)
      result += ",";
  }
  result += ";";

  result += String(gameState.cursorPosition) + ";";

  return result;
}

// ###################### Send Game State ######################
void sendGameStateToSlave() {
  if (connectedSlave.status && SerialBT.hasClient()) {
    String stateString = buildGameState();
    printMessage("Game state sent to slave:\n" + stateString);
  } else {
    printMessage("No slave connected. Cannot send game state.");
  }
}

// ###################### Set New Game ######################
void setupNewGame() {
  gameState.host = {"host01", 'X', 0, "HST"};
  gameState.guest = {"quest01", 'O', 0, "QST"};
  gameState.bestOf = 3;
  gameState.turn = 'X';

  for (int i = 0; i < 9; i++) {
    gameState.board[i] = "**";
  }

  gameState.cursorPosition = 0;

  sendGameStateToSlave();
}