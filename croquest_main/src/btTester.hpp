#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "bluetooth_host.hpp"
#include "bluetooth_guest.hpp"

class btTester {
public:
    btTester(TFT_eSPI* tft, bool isHost);
    void update();
    void draw();

private:
    TFT_eSPI* _tft;
    bool _isHost;
    uint16_t _hostCode;
    uint32_t _lastUpdateTime;
    uint32_t _lastRandomTime;
    int _randomNumber;
    String _currentColor;
    std::vector<String> _guestMessages;
    
    void updateHost();
    void updateGuest();
    void drawHost();
    void drawGuest();
    void setBackgroundColor(const String& color);
    String getRandomColor();
}; 