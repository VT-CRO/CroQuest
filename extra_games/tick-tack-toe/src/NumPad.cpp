#include "NumPad.hpp"

//modify button position and state, also redraws the button
void NumPad::modButtonState(enum direction direction, enum button_state state){
    drawButton(BASIC);
    
    switch(direction){
        case UP:
            if (row > 0) row--;
            break;
        case DOWN:
            if (row < 3) row++;
            break;
        case LEFT:
            if (column > 0) column--;
            break;
        case RIGHT:
            if (column < 2) column++;
            break;
        default:
            break;
    }
    if(state == PRESSED){
        if(pad[row][column] == DEL){
            if(code.length() > 0){
                //removes right-most number
                code.pop_back();
            }
    
        }else if(pad[row][column] == ENTER){
            //////// TO-DO ///////////// 
            /// BLUETOOTH LOGIC //////////
        }else{
            if(code.length() < max_length){
                std::string button = std::to_string(pad[row][column]);
                code += button;
            }
        }
    }
    //Draws the code and button state
    drawCode();
    drawButton(state);
}

//Draw all of the numpad
void NumPad::drawAllButtons(){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            row = i;
            column = j;
            drawButton(BASIC);
        }
    }
    row = 0;
    column = 0;
}

//Draw a button
void NumPad::drawButton(enum button_state state){
    struct Position position =  buttonPos[row][column];
    
    switch(state){
        case PRESSED:
            tft.fillRect(position.row, position.column, buttonWidth, buttonHeight, TFT_CYAN);
            break;
        case BASIC:
            tft.fillRect(position.row, position.column, buttonWidth, buttonHeight, TFT_WHITE);
            break;
        case SELECTED:
            tft.fillRect(position.row, position.column, buttonWidth, buttonHeight, TFT_BLACK);
            break;
    }
}

//Draw Code
void NumPad::drawCode() {
    // Clear area at top for label + code with padding
    tft.fillRect(0, 0, tft.width(), 70, TFT_WHITE);

    // Draw the label "Enter Host Code" centered horizontally near the top
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    const char* label = "Enter Host Code";
    int labelWidth = tft.textWidth(label);
    int labelX = (tft.width() - labelWidth) / 2;  // center horizontally

    // For vertical centering, calculate text height: text height = text size * 8 pixels (default font)
    // So for size 2, height = 2 * 8 = 16 pixels
    // To center vertically in area y=0..30 (for example), position y = (30 - 16) / 2 = 7
    // You can adjust 7 or 8 depending on how it looks.

    int labelHeight = 2 * 8;
    int labelY = (30 - labelHeight) / 2;  // center vertically in 0..30 px region

    tft.drawString(label, labelX, labelY);

    // Draw a rounded rectangle background behind the code
    tft.fillRoundRect(40, 40, tft.width() - 80, 40, 8, TFT_LIGHTGREY);

    // Draw the code string centered inside the rounded rect at y=45
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    int codeWidth = tft.textWidth(code.c_str());
    int codeX = (tft.width() - codeWidth) / 2;
    tft.drawString(code.c_str(), codeX, 45);
}

