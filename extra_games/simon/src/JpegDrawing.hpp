#ifndef JPEG_DRAWING_HPP
#define JPEG_DRAWING_HPP

#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include <SPI.h>
#include <SD.h>

class JpegDrawing {
private:
    TFT_eSprite sprite;
    bool swapBytes;
    bool buffer_created;
    int buffer_width;
    int buffer_height;
    int x_pos;
    int y_pos;
    bool first = true;
    
    bool createBuffer(int width, int height);
    void jpegRender(int xpos, int ypos);
    
public:
    // Constructor
    JpegDrawing(TFT_eSPI& tft);
    
    // Public struct for image information
    struct ImageInfo {
        uint32_t width;
        uint32_t height;
        bool valid;
    };
    
    // Public methods
    void drawSdJpeg(const char *filename, int xpos, int ypos);
    void pushSprite(bool persistent = false, bool transparent = false, uint16_t transparent_color = 0);
    ImageInfo getJpegDimensions(const char *filename);
    void deleteSprite();
};

#endif // JPEG_DRAWING_HPP