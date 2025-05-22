#ifndef JPEG_DRAWING_HPP
#define JPEG_DRAWING_HPP

#include <JPEGDecoder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <map>
#include <string>

class JpegDrawing {
private:
  TFT_eSPI &tft;
  TFT_eSprite sprite;
  bool swapBytes;
  bool buffer_created;
  int buffer_width;
  int buffer_height;
  int x_pos;
  int y_pos;
  bool first = true;

  bool using_cached_data = false;

  //Cached Sprites
  std::map<std::string, TFT_eSprite*> spriteCache;

  bool createBuffer(int width, int height);
  void jpegRender(int xpos, int ypos);
  
public:
  // Constructor
  JpegDrawing(TFT_eSPI &tft);

  // Public struct for image information
  struct ImageInfo {
    uint32_t width;
    uint32_t height;
    bool valid;
  };

  // Public methods
  void drawSdJpeg(const char *filename, int xpos, int ypos);
  void pushSprite(bool persistent = false, bool transparent = false,
                  uint16_t transparent_color = 0);
  void drawJpegTile(const char *filename, int srcX, int srcY, int w, int h,
                    int dstX, int dstY);

  void deleteSprite();

  void clearSprite(uint16_t color = TFT_BLACK);

  void setFirst(bool value); // Cntrol sprite logic manually

  ImageInfo getJpegDimensions(const char *filename);

  // Caching API
  void addToCache(const char *path);
  void clearCache();
};

#endif // JPEG_DRAWING_HPP