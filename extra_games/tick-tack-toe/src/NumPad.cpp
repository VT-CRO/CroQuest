#include "NumPad.hpp"

//modify button position and state, also redraws the button
void NumPad::modButtonState(enum direction direction, enum button_state state){
    
    int prev_row = row;
    int prev_col = column;
    
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
    if(prev_row != row || prev_col != column)
        drawButton(BASIC, prev_row, prev_col);

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
    drawButton(state, row, column);
}

void NumPad::drawAllButtons() {
    JpegDrawing::ImageInfo dim = drawing.getJpegDimensions("/numpad/numpad.jpg");
    int x = (480 - dim.width) / 2;
    int y = (320 - dim.height) / 2;

    drawing.drawSdJpeg("/numpad/numpad.jpg", x, y);
    drawing.pushSprite(true);
}

//Draw a button
void NumPad::drawButton(enum button_state state, int row_button, int column_button){
    struct Position position =  buttonPos[row_button][column_button];
    switch(state){
        case PRESSED:
            drawing.drawSdJpeg((std::string("/numpad/pressed/") + std::to_string(pad[row_button][column_button]) + ".jpg").c_str(), position.x, position.y);
            break;
        case BASIC:
            drawing.drawSdJpeg((std::string("/numpad/basic/") + std::to_string(pad[row_button][column_button]) + ".jpg").c_str(), position.x, position.y);
            break;
        case SELECTED:
            drawing.drawSdJpeg((std::string("/numpad/selected/") + std::to_string(pad[row_button][column_button]) + ".jpg").c_str(), position.x, position.y);
            break;
    }
    drawing.pushSprite(true);
}

//Draw Code
void NumPad::drawCode() {
    // --- Draw Label ---
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // white text on black
    const char* label = "Join a Host Code:";
    int labelWidth = tft.textWidth(label);
    int labelX = (SCREEN_WIDTH - labelWidth) / 2;
    int labelY = 10;  // near the top
    tft.drawString(label, labelX, labelY);

    // --- Clear old code text area ---
    int codeY = labelY + 30;  // space below the label
    int codeHeight = 8 * 4;    // height of textSize 4 (font height * text size)
    int codeWidth = tft.textWidth(code.c_str());
    int codeX = (SCREEN_WIDTH - codeWidth) / 2;
    // Clear the entire area behind the code with black
    tft.fillRect(0, codeY, SCREEN_WIDTH, codeHeight, TFT_BLACK);

    // --- Draw Code ---
    tft.setTextSize(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(code.c_str(), codeX, codeY);
}

void NumPad::handleButtonInput(unsigned long * lastMoveTime, const long moveDelay){
    if(millis() - *lastMoveTime > moveDelay){
      // Press Logic
      if(digitalRead(BTN_SELECT) == LOW && !pressed){
        modButtonState(NumPad::NONE, NumPad::PRESSED);
        pressed = true;
      }
      else if(digitalRead(BTN_SELECT) == HIGH && pressed){
        modButtonState(NumPad::NONE, NumPad::SELECTED);
        pressed = false;
      }
      // Selection logic
      if(digitalRead(BTN_UP) == LOW){
        modButtonState(NumPad::UP, NumPad::SELECTED);
      }
      else if(digitalRead(BTN_DOWN) == LOW){
        modButtonState(NumPad::DOWN, NumPad::SELECTED);
      }
      else if(digitalRead(BTN_RIGHT) == LOW){
        modButtonState(NumPad::RIGHT, NumPad::SELECTED);
      }
      else if(digitalRead(BTN_LEFT) == LOW){
        modButtonState(NumPad::LEFT, NumPad::SELECTED);
      }
      
      *lastMoveTime = millis();
    }
}

void NumPad::numPadSetup(){
    tft.fillScreen(TFT_BLACK);
    drawAllButtons();
    modButtonState(NumPad::NONE, NumPad::SELECTED);
}