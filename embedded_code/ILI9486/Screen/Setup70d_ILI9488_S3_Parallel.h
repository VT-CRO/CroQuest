
#define USER_SETUP_ID 146

#define TFT_PARALLEL_8_BIT

// #define ILI9341_DRIVER
// #define ST7796_DRIVER
// #define ILI9488_DRIVER
#define ILI9486_DRIVER

// ESP32 S3 pins used for the parallel interface TFT
#define TFT_CS 2 // 2

// Data Command control pin - must use a GPIO in the range 0-31
#define TFT_DC 32  // 4   // 32
#define TFT_RST 15 // 15

// Write strobe control pin - must use a GPIO in the range 0-31
#define TFT_WR 33 // 16 // 33
#define TFT_RD 4  // 17 // 4

// Must use GPIO in the range 0-31 for the data bus
#define TFT_D0 12 // 19
#define TFT_D1 13 // 21 // 13 // so a single register write sets/clears all bits
#define TFT_D2 26 // 32// 26
#define TFT_D3 25 // 35 // 25
#define TFT_D4 17 // 34 // 34
#define TFT_D5 16 // 25 // 35
#define TFT_D6 27 // 5  // 27
#define TFT_D7 14 // 18 // 14

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

