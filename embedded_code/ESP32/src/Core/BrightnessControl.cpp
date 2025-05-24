// // src/Core/BrightnessControl.cpp

// #include "BrightnessControl.hpp"

// void initBacklightPWM() {
//   ledcSetup(BACKLIGHT_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
//   ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
// }

// void applyBrightness(int level) {
//   int duty = map(level, 0, 10, 10, 255); // Avoid full off
//   ledcWrite(BACKLIGHT_CHANNEL, duty);
// }