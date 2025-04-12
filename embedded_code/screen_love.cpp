#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_SPI
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;

      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 15;
      cfg.pin_rst = 4;
      cfg.pin_busy = -1;

      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

LGFX tft;

#define MAX_SQUARES 10
int squareSize = 20;

struct Square {
  int x, y;
  int dx, dy;
  uint32_t color;
};

Square squares[MAX_SQUARES];

int currentPhase = 0;
int phaseLengths[] = {1, 5, 10};
int numPhases = sizeof(phaseLengths) / sizeof(phaseLengths[0]);

unsigned long phaseStartTime = 0;

void initSquares(int count) {
  for (int i = 0; i < count; i++) {
    squares[i].x = random(0, tft.width() - squareSize);
    squares[i].y = random(0, tft.height() - squareSize - 30);
    squares[i].dx = 2 + random(3);
    squares[i].dy = 2 + random(3);
    squares[i].color = tft.color888(random(256), random(256), random(256));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(21, OUTPUT); // TFT_LED
  digitalWrite(21, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  initSquares(phaseLengths[currentPhase]);
  phaseStartTime = millis();
}

void drawSquare(int x, int y, uint32_t color) {
  tft.fillRect(x, y, squareSize, squareSize, color);
}

void eraseSquare(int x, int y) {
  tft.fillRect(x, y, squareSize, squareSize, TFT_BLACK);
}

void moveSquares(int count) {
  for (int i = 0; i < count; i++) {
    int oldX = squares[i].x;
    int oldY = squares[i].y;

    squares[i].x += squares[i].dx;
    squares[i].y += squares[i].dy;

    if (squares[i].x < 0 || squares[i].x + squareSize > tft.width()) squares[i].dx *= -1;
    if (squares[i].y < 0 || squares[i].y + squareSize > tft.height() - 30) squares[i].dy *= -1;

    eraseSquare(oldX, oldY);
    drawSquare(squares[i].x, squares[i].y, squares[i].color);
  }
}

void loop() {
  unsigned long now = millis();
  int count = phaseLengths[currentPhase];

  moveSquares(count);

  if (now - phaseStartTime >= 5000) {
    currentPhase++;
    if (currentPhase >= numPhases) {
      showFinalResults();
      while (true);
    } else {
      tft.fillScreen(TFT_BLACK);
      initSquares(phaseLengths[currentPhase]);
      phaseStartTime = millis();
    }
  }
}

void showFinalResults() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(TFT_GREEN);
  tft.print("Benchmark Results:");

  for (int i = 0; i < numPhases; i++) {
    tft.setCursor(20, 80 + i * 30);
    tft.setTextColor(TFT_WHITE);
    tft.print("Phase ");
    tft.print(i + 1);
    tft.print(" (");
    tft.print(phaseLengths[i]);
    tft.print(" squares)");
  }
}

