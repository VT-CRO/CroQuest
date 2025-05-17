#include "JpegDrawing.hpp"

bool JpegDrawing::createBuffer(int width, int height){
    if (buffer_created) {
        sprite.deleteSprite();
    }

    // Create a new sprite with the specified dimensions
    buffer_created = sprite.createSprite(width, height);
    
    if (buffer_created) {
        buffer_width = width;
        buffer_height = height;

        // Ensure correct byte order for RGB565
        swapBytes = sprite.getSwapBytes();
        sprite.setSwapBytes(true);
        
        // Fill with background color
        sprite.fillSprite(TFT_BLACK);
        return true;
    } else {
        Serial.println("ERROR: Buffer allocation failed!");
        return false;
    }
}

void JpegDrawing::jpegRender(int xpos, int ypos) {
    uint16_t *pImg;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;

    if(!buffer_created){
        Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
        Serial.println(max_x);
        Serial.println(max_y);
        createBuffer(max_x, max_y);
        x_pos = xpos;
        y_pos = ypos;
    }

    // MCU edge handling
    uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
    uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);
    uint32_t win_w = mcu_w;
    uint32_t win_h = mcu_h;

    while (JpegDec.read()) {
        pImg = JpegDec.pImage;
        int mcu_x = JpegDec.MCUx * mcu_w;
        int mcu_y = JpegDec.MCUy * mcu_h;

        if (mcu_x + mcu_w <= max_x) win_w = mcu_w; else win_w = min_w;
        if (mcu_y + mcu_h <= max_y) win_h = mcu_h; else win_h = min_h;

        if (win_w != mcu_w) {
        uint16_t *cImg = pImg + win_w;
        int p = 0;
        for (int h = 1; h < win_h; h++) {
            p += mcu_w;
            for (int w = 0; w < win_w; w++) {
            *cImg = *(pImg + w + p);
            cImg++;
            }
        }
        }
        if(first){
            // Draw to the sprite (not directly to TFT)
            sprite.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
        }
        else{
                sprite.pushImage(mcu_x + xpos, mcu_y + ypos, win_w, win_h, pImg);
            }
        }
        first = false;
}

// Constructor - sets up basic properties
JpegDrawing::JpegDrawing(TFT_eSPI& tft) : sprite(&tft) {
    sprite.setColorDepth(8); // 8-bit for JPEG colors
    buffer_created = false;
    buffer_width = 0;
    buffer_height = 0;
}

void JpegDrawing::drawSdJpeg(const char *filename, int xpos, int ypos) {

    // Open the named file (the Jpeg decoder library will close it)
    File jpegFile =
        SD.open(filename, FILE_READ); // or, file handle reference for SD library

    if (!jpegFile) {
        Serial.print("ERROR: File \"");
        Serial.print(filename);
        Serial.println("\" not found!");
        return;
    }

    // Use one of the following methods to initialise the decoder:
    bool decoded =
        JpegDec.decodeSdFile(jpegFile); // Pass the SD file handle to the decoder,
    // bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename
    // (String or character array)

    if (decoded) {
        // render the image onto the screen at given coordinates
        jpegRender(xpos, ypos);
    } else {
        Serial.println("Jpeg file format not supported!");
    }
}

void JpegDrawing::pushSprite(){
    // Push full image to the screen
    sprite.pushSprite(x_pos, y_pos);

    // Clean up
    sprite.setSwapBytes(swapBytes);

    sprite.deleteSprite();
    buffer_created = false;
    buffer_width = 0;
    buffer_height = 0;
    x_pos = 0; 
    y_pos = 0;
    first = true;
}

JpegDrawing::ImageInfo JpegDrawing::getJpegDimensions(const char *filename) {
    ImageInfo info = {0, 0, false};
    
    // Open the named file
    File jpegFile = SD.open(filename, FILE_READ);
    
    if (!jpegFile) {
        Serial.print("ERROR: File \"");
        Serial.print(filename);
        Serial.println("\" not found!");
        return info;
    }
    
    // Decode JPEG file to get dimensions
    bool decoded = JpegDec.decodeSdFile(jpegFile);
    
    if (decoded) {
        info.width = JpegDec.width;
        info.height = JpegDec.height;
        info.valid = true;
    } else {
        Serial.println("ERROR: Jpeg file format not supported!");
    }
    
    // We need to clean up, but don't add to buffer
    JpegDec.abort();
    
    return info;
}