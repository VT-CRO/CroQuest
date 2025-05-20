// Buttons.cpp
#include "Buttons.hpp"

// Digital Buttons
Button A(22, "A", DIGITAL);
Button B(39, "B", DIGITAL);
Button Start(36, "Start", DIGITAL);

// Analog Buttons
Button up(35, "Up", ANALOG_INPUT, 2000, 3600);
Button right(35, "Right", ANALOG_INPUT, 3601, 4095);
Button left(34, "Left", ANALOG_INPUT, 2000, 3600);
Button down(34, "Down", ANALOG_INPUT, 3601, 4095);

void initButtons() {
  A.begin();
  B.begin();
  Start.begin();
  up.begin();
  down.begin();
  left.begin();
  right.begin();
}

// void updateAllButtons() {
//   A.update();
//   B.update();
//   Start.update();
//   up.update();
//   down.update();
//   left.update();
//   right.update();
// }
