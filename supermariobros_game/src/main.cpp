// Demo based on:
// UTFT_Demo by Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
/*

 The delay between tests is set to 0. The tests run so fast you will need to
 change the WAIT value below to see what is being plotted!

 This sketch uses the GLCD and font 2 only.

 Make sure all the required fonts are loaded by editing the
 User_Setup.h file in the TFT_eSPI library folder.

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  ######            TO SELECT THE FONTS YOU USE, SEE ABOVE           ######
  #########################################################################
 */

#include <Arduino.h>
#include "game.cpp"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft =
    TFT_eSPI(); // Invoke custom library with default width and height

uint32_t runTime = 0;
Game game;

void setup_game() {
  // Initialize the game
  game.parseWorld("whatever");
}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(38400);
  // Setup the LCD
  tft.init();
  tft.setRotation(1);
  setup_game();
}

float convertToScreenY(float y, float h) {
  return tft.height() - y - h;
}

void render_game_frame() {
  // Render the game frame
  tft.fillScreen(TFT_CYAN);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Game Frame");
  for (const auto& platform : game.scenes[0].platforms) {
    tft.fillRect(platform.x, convertToScreenY(platform.y, platform.h), platform.w, platform.h, TFT_GREEN);
  }
  for (const auto& entity : game.scenes[0].entities) {
    tft.drawEllipse(entity.x, convertToScreenY(entity.y, entity.h), entity.w, entity.h, TFT_RED);
  }
}

void loop() {
  render_game_frame();
  delay(1000 / 60); // Simulate 60 FPS
}
