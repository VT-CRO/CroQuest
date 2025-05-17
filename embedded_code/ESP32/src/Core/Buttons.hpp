// Buttons.hpp

#pragma once
#include <Arduino.h>

enum ButtonType { DIGITAL, ANALOG_INPUT };

class Button {
private:
  int pin;
  const char *label;
  ButtonType type;
  int lastState;
  int thresholdLow;
  int thresholdHigh;

public:
  Button(int pin, const char *label, ButtonType type = DIGITAL,
         int thresholdLow = 0, int thresholdHigh = 4095)
      : pin(pin), label(label), type(type), lastState(LOW),
        thresholdLow(thresholdLow), thresholdHigh(thresholdHigh) {}

  void begin() {
    if (type == DIGITAL) {
      pinMode(pin, INPUT_PULLUP); // Recommended for button wiring
    } else {
      pinMode(pin, INPUT);
    }
  }

  bool isPressed() const {
    if (type == DIGITAL) {
      return digitalRead(pin) == LOW; // LOW = pressed when using pull-up
    } else {
      int value = analogRead(pin);
      return (value >= thresholdLow && value <= thresholdHigh);
    }
  }

  bool wasJustPressed() {
    if (type == DIGITAL) {
      int currentState = digitalRead(pin);
      bool pressed = (currentState == LOW && lastState == HIGH);
      lastState = currentState;
      return pressed;
    } else {
      return isPressed(); // Analog buttons can't track previous state yet
    }
  }

  int getAnalogValue() const {
    return (type == ANALOG_INPUT) ? analogRead(pin) : -1;
  }

  const char *getLabel() const { return label; }
  int getPin() const { return pin; }
};
