#include <LovyanGFX.hpp>

// Valid ESP32-S3 GPIOs
#define TFT_RST    2 // 32 = NC ?
#define TFT_DC     15 // 18 = GPIO10

#define TFT_WR     16 // 4 = GPIO36
#define TFT_RD      17 // 2 = 3V3

#define TFT_CS      18 // 33 = GPIO21

#define TFT_D0     4 // 12 = GPIO27
#define TFT_D1     5 // 13 = GPIO14
#define TFT_D2     6 // 26 = GPIO4
#define TFT_D3     7 // 25 = GPIO0
#define TFT_D4     8 // 21 = GPIO7
#define TFT_D5     9 // 5 = GPIO39
#define TFT_D6     10 // 27 = GPIO16
#define TFT_D7     11 // 14 = GPIO12

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9486 _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;

public:
  LGFX(void) {
    { // Bus config (Parallel8)
      auto cfg = _bus_instance.config();

      cfg.freq_write = 20000000;
      cfg.pin_wr = TFT_WR;
      cfg.pin_rd = TFT_RD;
      cfg.pin_rs = TFT_DC;

      cfg.pin_d0 = TFT_D0;
      cfg.pin_d1 = TFT_D1;
      cfg.pin_d2 = TFT_D2;
      cfg.pin_d3 = TFT_D3;
      cfg.pin_d4 = TFT_D4;
      cfg.pin_d5 = TFT_D5;
      cfg.pin_d6 = TFT_D6;
      cfg.pin_d7 = TFT_D7;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Panel config
      auto cfg = _panel_instance.config();

      cfg.pin_cs  = TFT_CS;        // If not used, set to -1
      cfg.pin_rst = TFT_RST;   // Only available in panel config

      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.memory_width = 320;
      cfg.memory_height = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

LGFX display;

void setup() {
  display.init();
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_GREEN);
  display.setTextSize(2);
  display.setCursor(40, 110);
  display.print("Hello S3!");
}

void loop() {
}

