#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <LovyanGFX.hpp>

// TFT display pins - using SPI2_HOST (previously known as VSPI)
#define TFT_MISO  12
#define TFT_LED   21
#define TFT_SCK   14
#define TFT_MOSI  13
#define TFT_DC     2
#define TFT_RESET  4
#define TFT_CS    15

// Pin definitions and constants
#define UP 8
#define DOWN 18
#define LEFT 3
#define RIGHT 46
#define A 16
#define B 17
#define HOST_CODE_SIZE 6

#define SD_CS     5  // SD Card CS pin

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel;
    lgfx::Bus_SPI _bus;
  
  public:
    LGFX(void) {
      {
        auto cfg = _bus.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;  // Reduced SPI frequency for increased stability
        cfg.freq_read  = 16000000;
        cfg.spi_3wire  = false;
        cfg.use_lock   = true;
        cfg.dma_channel = 1;
        cfg.pin_sclk = TFT_SCK;
        cfg.pin_mosi = TFT_MOSI;
        cfg.pin_miso = TFT_MISO;
        cfg.pin_dc   = TFT_DC;
  
        _bus.config(cfg);
        _panel.setBus(&_bus);
      }
  
      {
        auto cfg = _panel.config();
        cfg.pin_cs   = TFT_CS;
        cfg.pin_rst  = TFT_RESET;
        cfg.pin_busy = -1;
  
        cfg.memory_width  = 240;
        cfg.memory_height = 320;
        cfg.panel_width   = 240;
        cfg.panel_height  = 320;
        cfg.offset_x      = 0;
        cfg.offset_y      = 0;
        cfg.offset_rotation = 0;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits  = 1;
        cfg.readable      = true;
        cfg.invert        = false;
        cfg.rgb_order    = false;
        cfg.dlen_16bit  = false;
        cfg.bus_shared  = true;
  
        _panel.config(cfg);
      }
  
      setPanel(&_panel);
    }
  };

typedef struct {
int y;
bool paddle_mod;
} Prev_Paddle;