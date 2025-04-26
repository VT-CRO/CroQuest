#include <Arduino.h>
#include <TFT_eSPI.h>
#include "btTester.hpp"

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI();

// Set to true for host mode, false for guest mode
const bool IS_HOST = true; // Change this to false for guest mode

// Create btTester instance
btTester* tester = nullptr;

void setup() {
    Serial.begin(115200);
    Serial.println("Bluetooth Tester Example");
    
    // Initialize display
    tft.init();
    tft.setRotation(1); // Landscape
    
    // Create btTester instance
    tester = new btTester(&tft, IS_HOST);
    
    Serial.println(IS_HOST ? "Host Mode" : "Guest Mode");
    Serial.println("Host Code: 292");
}

void loop() {
    // Update and draw
    tester->update();
    tester->draw();
    
    // Small delay to prevent overwhelming the system
    delay(50);
} 