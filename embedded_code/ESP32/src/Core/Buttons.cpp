// Buttons.cpp
#include "Buttons.hpp"

// Button A(22, "A", DIGITAL);
// Button B(39, "B", DIGITAL);
// Button Start(36, "Start", DIGITAL);

// Working buttons (hopefully)

Button A(36, "A", ANALOG_INPUT, 100, 1800);
Button B(36, "B", ANALOG_INPUT, 2800, 4095);
Button Start(36, "Start", ANALOG_INPUT, 1801, 2600);

// Analog Buttons
Button up(35, "Up", ANALOG_INPUT, 2000, 3800);
Button right(35, "Right", ANALOG_INPUT, 3801, 4095);
Button left(34, "Left", ANALOG_INPUT, 2000, 3800);
Button down(34, "Down", ANALOG_INPUT, 3801, 4095);

void initButtons() {
  A.begin();
  B.begin();
  Start.begin();
  up.begin();
  down.begin();
  left.begin();
  right.begin();
}

void updateAllButtons() {
  A.update();
  B.update();
  Start.update();
  up.update();
  down.update();
  left.update();
  right.update();
}
