#include "Name.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"

#define TFT_DULL_YELLOW tft.color565(255, 180, 0)   // More desaturated

//Selection
static int prevSelectedLetterIndex = 0;
static int selectedLetterIndex = 0;

//Box Sizes
int boxSize = 80;
int spacing = 10;
int startX = 20;

//Box Y-level
int y = 100;

//Previous letters
char prevChars[6];

const int SCREEN_HEIGHT = 320;
const int SCREEN_WIDTH = 480;


//tft object
extern TFT_eSPI tft;

// Will be used to change the color of the up and down arrows
enum NameSelection {NO_ARROW_SELECTED, UP_SELECTED, DOWN_SELECTED};

//Static functions
static void handleDrawing(char name[], NameSelection selection);
static void scrollingAudio();
static void nextCharAudio();

//Runs the NameMenu
void runNameMenu(){
    //Copies settings.name into prevChars
    memcpy(prevChars, settings.name, sizeof(prevChars));
    tft.fillScreen(SETTINGS_BG_COLOR);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Edit Name", SCREEN_WIDTH / 2, 40, 4);
    tft.setTextSize(1);
    tft.drawString("Use LEFT/RIGHT to select, UP/DOWN to change letter, A to return", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, 1);
    

    //Prints initial layout
    handleDrawing(settings.name, NO_ARROW_SELECTED);

    // Main loop for Editing Name
    while(1){
        if (left.isPressed()) {
            if (selectedLetterIndex > 0) {
                selectedLetterIndex--;
                handleDrawing(settings.name, NO_ARROW_SELECTED);
                nextCharAudio();
                delay(150);
            }
        } else if (right.isPressed()) {
            if (selectedLetterIndex < 4) {
                selectedLetterIndex++;
                handleDrawing(settings.name, NO_ARROW_SELECTED);
                nextCharAudio();
                delay(150);
            }
        } else if(up.isPressed()){
            char& c = settings.name[selectedLetterIndex];
            c = (c >= 'A' && c < 'Z') ? c + 1 : 'A';
            handleDrawing(settings.name, UP_SELECTED);
            scrollingAudio();
            delay(150);
        }
        else if(down.isPressed()){
            char& c = settings.name[selectedLetterIndex];
            c = (c > 'A' && c <= 'Z') ? c - 1 : 'Z';
            handleDrawing(settings.name, DOWN_SELECTED);
            scrollingAudio();
            delay(150);
        }
        else if (A.wasJustPressed()) {
            backAudio();
            break;
        }
        delay(10);
        //Handles drawing the name and boxes

        //Resets the color of the up and down arrows above and below the box
        if(!up.isPressed() || !down.isPressed()){
            handleDrawing(settings.name, NO_ARROW_SELECTED);
        }
    }
}

// Handles drawing the boxes
static void handleDrawing(char name[], NameSelection selection){
    int arrow_offset = 6;
    int base = 20;
    int height = 16;

    for (int i = 0; i < 5; ++i) {
    int x = startX + i * (boxSize + spacing);

    if(i == prevSelectedLetterIndex && prevSelectedLetterIndex != selectedLetterIndex){
        // Remove highlight
        tft.drawRect(x - 2, y - 2, boxSize + 4, boxSize + 4, SETTINGS_BG_COLOR);

        //remove arrows
        tft.fillRect(x + boxSize / 2 - base/2, y - height - 6, base + 4, height + 4, SETTINGS_BG_COLOR);       // up
        tft.fillRect(x + boxSize / 2 - base/2, y + boxSize + 4, base + 4, height + 4, SETTINGS_BG_COLOR); // down

        prevSelectedLetterIndex = selectedLetterIndex;
    }

    if (i == selectedLetterIndex) {
        // Draw highlight
        tft.drawRect(x - 2, y - 2, boxSize + 4, boxSize + 4, TFT_YELLOW);

        // Up arrow color selection indicator
        uint16_t color = selection ==  UP_SELECTED ? TFT_YELLOW : TFT_DULL_YELLOW;

        // Up arrow
        tft.fillTriangle(x + boxSize / 2, y - height - arrow_offset, 
                            x + boxSize / 2 - base / 2, y - arrow_offset, 
                            x + boxSize / 2 + base / 2, y - arrow_offset, color);
        
        // Down arrow color selection indicator
        color = selection == DOWN_SELECTED ? TFT_YELLOW : TFT_DULL_YELLOW;
        // Down arrow
        tft.fillTriangle(x + boxSize / 2, y + boxSize + height + arrow_offset, 
                            x + boxSize / 2 - base / 2, y + boxSize + arrow_offset,
                            x + boxSize / 2 + base / 2, y + boxSize + arrow_offset, color);
    }

    // Box
    tft.drawRect(x, y, boxSize, boxSize, TFT_WHITE);
    tft.drawRect(x + 1, y + 1, boxSize - 2, boxSize - 2, TFT_WHITE);
    tft.drawRect(x + 2, y + 2, boxSize - 4, boxSize - 4, TFT_WHITE);

    if(prevChars[i] != name[i]){
        tft.fillRect(x + 3, y + 3, boxSize - 6, boxSize - 6, SETTINGS_BG_COLOR);
        prevChars[i] = name[i];
    }
        // Letter
        char str[2] = {name[i], '\0'};
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(6);
        tft.drawString(str, x + boxSize / 2, y + boxSize / 2, 1);
    }
}

// ========================= AUDIO ========================= //

static void scrollingAudio(){
    playTone(880, volume);
    delay(40);
    playTone(0, volume);
}

static void nextCharAudio(){
    playTone(1200, volume);
    delay(30);
    playTone(0, volume);
}