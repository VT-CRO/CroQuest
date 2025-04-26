#include "btTester.hpp"
#include <BLEDevice.h>

btTester::btTester(TFT_eSPI* tft, bool isHost) {
    _tft = tft;
    _isHost = isHost;
    _hostCode = 292;
    _lastUpdateTime = 0;
    _lastRandomTime = 0;
    _randomNumber = 0;
    _currentColor = "BLACK";
    
    // Initialize Bluetooth based on mode
    if (_isHost) {
        btScanAndConnect(_hostCode);
    } else {
        btStartAdvertising(_hostCode);
    }
    
    // Set initial background color
    setBackgroundColor(_currentColor);
}

void btTester::update() {
    uint32_t currentTime = millis();
    
    // Update every 500ms
    if (currentTime - _lastUpdateTime >= 500) {
        _lastUpdateTime = currentTime;
        
        if (_isHost) {
            updateHost();
        } else {
            updateGuest();
        }
    }
    
    // Generate random number every 500ms (for guest)
    if (!_isHost && currentTime - _lastRandomTime >= 500) {
        _lastRandomTime = currentTime;
        _randomNumber = random(1, 1000);
        
        // Send random number to host
        String message = "random:" + String(_randomNumber);
        btSendToHost(message);
    }
}

void btTester::updateHost() {
    // Check for messages from guests
    std::vector<String> messages = btReceivedFromGuest();
    for (const String& message : messages) {
        if (message.startsWith("color:")) {
            // Extract color from message
            String color = message.substring(6);
            setBackgroundColor(color);
            
            // Send color to all guests
            btSendToGuests("color:" + color);
        } else if (message.startsWith("random:")) {
            // Extract random number from message
            String randomStr = message.substring(7);
            int randomNum = randomStr.toInt();
            
            // Extract guest ID from the full message
            String guestId = message.substring(0, message.indexOf(":"));
            
            // Update guest messages
            bool found = false;
            for (size_t i = 0; i < _guestMessages.size(); i++) {
                if (_guestMessages[i].startsWith(guestId)) {
                    _guestMessages[i] = guestId + ": " + String(randomNum);
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                _guestMessages.push_back(guestId + ": " + String(randomNum));
            }
        }
    }
    
    // Change background color every 5 seconds
    if (currentTime % 5000 < 500) {
        _currentColor = getRandomColor();
        setBackgroundColor(_currentColor);
        btSendToGuests("color:" + _currentColor);
    }
}

void btTester::updateGuest() {
    // Check for messages from host
    std::vector<String> messages = btReceivedFromHost();
    for (const String& message : messages) {
        if (message.startsWith("color:")) {
            // Extract color from message
            String color = message.substring(6);
            setBackgroundColor(color);
        }
    }
}

void btTester::draw() {
    if (_isHost) {
        drawHost();
    } else {
        drawGuest();
    }
}

void btTester::drawHost() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->setTextSize(2);
    
    // Draw header
    _tft->setCursor(10, 10);
    _tft->print("Host Mode - Code: ");
    _tft->print(_hostCode);
    
    // Draw connection count
    _tft->setCursor(10, 40);
    _tft->print("Connected Guests: ");
    _tft->print(btConnectedDevices().size());
    
    // Draw guest messages
    _tft->setCursor(10, 70);
    _tft->print("Guest Messages:");
    
    for (size_t i = 0; i < _guestMessages.size(); i++) {
        _tft->setCursor(20, 100 + i * 30);
        _tft->print(_guestMessages[i]);
    }
    
    // Draw current color
    _tft->setCursor(10, _tft->height() - 40);
    _tft->print("Color: ");
    _tft->print(_currentColor);
}

void btTester::drawGuest() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->setTextSize(2);
    
    // Draw header
    _tft->setCursor(10, 10);
    _tft->print("Guest Mode - Code: ");
    _tft->print(_hostCode);
    
    // Draw connection status
    _tft->setCursor(10, 40);
    _tft->print("Connected: ");
    _tft->print(btIsConnected() ? "Yes" : "No");
    
    // Draw random number
    _tft->setCursor(10, 70);
    _tft->print("Random: ");
    _tft->print(_randomNumber);
    
    // Draw current color
    _tft->setCursor(10, 100);
    _tft->print("Color: ");
    _tft->print(_currentColor);
}

void btTester::setBackgroundColor(const String& color) {
    _currentColor = color;
    
    if (color == "RED") {
        _tft->fillScreen(TFT_RED);
    } else if (color == "GREEN") {
        _tft->fillScreen(TFT_GREEN);
    } else if (color == "BLUE") {
        _tft->fillScreen(TFT_BLUE);
    } else if (color == "YELLOW") {
        _tft->fillScreen(TFT_YELLOW);
    } else if (color == "CYAN") {
        _tft->fillScreen(TFT_CYAN);
    } else if (color == "MAGENTA") {
        _tft->fillScreen(TFT_MAGENTA);
    } else {
        _tft->fillScreen(TFT_BLACK);
    }
}

String btTester::getRandomColor() {
    const char* colors[] = {"RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "BLACK"};
    int index = random(0, 7);
    return String(colors[index]);
} 