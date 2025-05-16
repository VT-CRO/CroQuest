#include <string>
#include "JpegDrawing.hpp"
#include <TFT_eSPI.h>

class NumPad {
    enum button_type {DEL = 10, ENTER = 11};
    const int SCREEN_WIDTH = 480;
public:
    enum direction {UP, DOWN, LEFT, RIGHT, NONE};
    enum button_state {PRESSED, BASIC, SELECTED};

    //Constructor
    NumPad(TFT_eSPI& tft, JpegDrawing& drawing, int btnUp, int btnDown, int btnLeft, int btnRight, int btnSelect) 
        : tft(tft), 
        drawing(drawing),
        BTN_UP(btnUp), 
        BTN_DOWN(btnDown), 
        BTN_LEFT(btnLeft), 
        BTN_RIGHT(btnRight), 
        BTN_SELECT(btnSelect) {

        // 126 = numpad image width, 132 = numpad image height, + 3 is the x padding around the image, + 5 is the y padding
        int x = (480 - 126) / 2 + 3;
        int y = (320 - 132) / 2 + 5;

        Serial.println(x);
        Serial.println(y);

        // Define layout
        const int buttonWidth = 36;
        const int buttonHeight = 24;
        const int spacingX = 6;
        const int spacingY = 9;
        
        // Calculate positions relative to the image
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 3; col++) {
                buttonPos[row][col].x = x + col * (buttonWidth + spacingX);
                buttonPos[row][col].y = y + row * (buttonHeight + spacingY);
            }
        }
        drawing.drawSdJpeg("/numpad/numpad.jpg", x, y); 
    }
    void drawAllButtons();
    void handleButtonInput(unsigned long * lastMoveTime, const long moveDelay);
    // Modify a button
    void modButtonState(enum direction direction, enum button_state pressed);
    void numPadSetup();
private:
    // Pad
    int pad[4][3] = {{1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9}, 
    {DEL, 0, ENTER}};
    
    struct Position {
        int x;
        int y;
    };

    // Button pins
    const int BTN_UP;
    const int BTN_DOWN;
    const int BTN_LEFT;
    const int BTN_RIGHT;
    const int BTN_SELECT;

    // Button pressed
    bool pressed = false;
    
    // Button positions (centered horizontally, spaced vertically)
    struct Position buttonPos[4][3];

    // Smaller button dimensions
    int buttonWidth = 50;
    int buttonHeight = 45;
    
    //The input code
    std::string code; 
    
    // row and column position
    int row = 0;
    int column = 0;
    int max_length = 4;
    
    //Screen objects/drawing
    TFT_eSPI& tft;
    JpegDrawing& drawing;
    
    void drawButton(enum button_state state, int row_button, int column_button);
    void drawCode();
};