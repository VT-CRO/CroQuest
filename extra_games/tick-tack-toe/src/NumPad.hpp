#include <string>
#include "JpegDrawing.hpp"
#include <TFT_eSPI.h>

class NumPad {
    enum button_type {DEL = -1, ENTER = -2};
public:
    enum direction {UP, DOWN, LEFT, RIGHT, NONE};
    enum button_state {PRESSED, BASIC, SELECTED};
    //Constructor
    NumPad(TFT_eSPI& tft) : tft(tft) {
            // Center point of the screen
        int centerX = 240; // Assuming 480px width
        int bottomOffset = 80; // Distance from bottom of screen
        int screenHeight = 320; // Assuming this is your screen height
        
        // Calculate y-positions from bottom
        int startY = screenHeight - bottomOffset - (3 * 50); // 50 is vertical spacing between rows
        
        // Space between buttons horizontally
        int buttonSpacing = 55;
        
        // Calculate starting x position to center the keypad
        int startX = centerX - buttonSpacing;
        
        // Setup button positions (centered, closer to bottom)
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 3; col++) {
                buttonPos[row][col].row = startX + (col * buttonSpacing);
                buttonPos[row][col].column = startY + (row * 50);
            }
        }
    }

    // Modify a button
    void modButtonState(enum direction direction, enum button_state pressed);
    void drawAllButtons();
    private:
    // Pad
    int pad[4][3] = {{1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9}, 
    {DEL, 0, ENTER}};
    
    struct Position {
        int row;
        int column;
    };
    
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
    
    TFT_eSPI& tft;
    
    void drawButton(enum button_state pressed);
    void drawCode();
};