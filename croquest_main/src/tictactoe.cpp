#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "tictactoe.hpp"
#include "bluetooth_guest.hpp"
#include "bluetooth_host.hpp"

uint64_t last_ms = 0;
int random_number = 0;
int is_host = 1;

void printMessage(TFT_eSPI *tft, const String &message)
{
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print(message);
}

uint16_t host_code = 292;

void tictactoe(TFT_eSPI *tft)
{
    uint64_t current_ms = millis();
    Serial.printf("Current time: %llu\n", current_ms);

    if (current_ms - last_ms > 100)
    {
        last_ms = current_ms;
        random_number = random(0, 9999);
        if (is_host) {
            int numberOfGuests = btConnectedDevices().size();
            printMessage(tft, "Host Code: " + String(host_code) + "\n" + "# of Guests: " + String(numberOfGuests));
        }
        
    }
}