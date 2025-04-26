#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "tictactoe.hpp"
#include "bluetooth_guest.hpp"
#include "bluetooth_host.hpp"

uint64_t last_ms = 0;
int random_number = 0;
int is_host = 1;

void tictactoe(TFT_eSPI *tft)
{
    uint64_t current_ms = millis();
    Serial.printf("Current time: %llu\n", current_ms);

    if (current_ms - last_ms > 1000)
    {
        last_ms = current_ms;
        random_number = random(0, 9999);
        if (is_host) {
            printMessage(tft, String("Random number: ") + random_number + "\n" + String("Host Code: XXX") + "\n" + String("# of Guests: Y"));
        }
        
    }
}

void printMessage(TFT_eSPI *tft, const String &message)
{
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print(message);
}