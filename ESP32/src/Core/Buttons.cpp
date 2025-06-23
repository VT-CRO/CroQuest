// Buttons.cpp
#include "Buttons.hpp"

/**
 * IMPORTANT: ALL THE "BATTERY" AVERAGE VALUES ARE WITH NEW BATTERIES
 */

// Button A(22, "A", DIGITAL);

// ###################### BUTTON 36 ######################

// Cable avg A = 1600 - 1700 ||| Battery 1600 - 1800 PROBLEM START
Button A(36, "A", ANALOG_INPUT, 100, 1800);

// Cable avg A = 2300 - 2400 ||| When connected to battery avg 2300 - 2700
Button Start(36, "Start", ANALOG_INPUT, 1801, 3000);

// Cable avg A = 4095 ||| When connected to battery avg 4095
Button B(36, "B", ANALOG_INPUT, 3001, 4095);

// ###################### BUTTON 35 ######################

// Cable avg Up = 3000 - 4000 ||| Battery avg 2900 - 4095 PROBLEM RIGHT
Button up(35, "Up", ANALOG_INPUT, 2000, 3800);

// Cable avg Right = 4095 ||| When connected to battery avg 4095
Button right(35, "Right", ANALOG_INPUT, 3801, 4095);

// ###################### BUTTON 34 ######################

// Cable avg Left = 3000 - 3100 ||| Battery avg 2900 - 4080 PROBLEM DOWN
// PROBLEM
Button left(34, "Left", ANALOG_INPUT, 2000, 3850);

// Cable avg Down = 4095 ||| When connected to battery 4095
Button down(34, "Down", ANALOG_INPUT, 3851, 4095);

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
