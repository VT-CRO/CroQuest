/*****************************************************************
 *  CroQuest – 480×320 single-buffer renderer (no scaling)
 *             ~30 fps, 8-bit RGB332 (≈153 kB) – fits SRAM
 *****************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <unordered_map>
#include "game.cpp"                       // Game / Scene / Entity

/* ---------- globals ------------------------------------------ */
Game        game;                         // world / logic
TFT_eSPI    tft;                          // 8-bit parallel (ILI9486)
TFT_eSprite fb(&tft);                     // full-screen framebuffer

/* ---------- constants ---------------------------------------- */
constexpr int  SCR_W = 480, SCR_H = 320;
constexpr uint16_t PLATFORM_COL = TFT_GREEN;
constexpr uint16_t DEFAULT_COL  = TFT_RED;

/* ---------- colour helpers ----------------------------------- */
static inline uint16_t hex565(const char *hex)
{
    auto h=[](char c){return (c<='9'?c-'0':(c|32)-'a'+10);};
    uint8_t r=(h(hex[1])<<4)|h(hex[2]),
            g=(h(hex[3])<<4)|h(hex[4]),
            b=(h(hex[5])<<4)|h(hex[6]);
    return tft.color565(r,g,b);
}
static inline uint16_t colourFromSkin(const std::string& s)
{
    if (s.rfind("color:#",0)==0 && s.size()==13)
        return hex565(s.c_str()+6);
    return DEFAULT_COL;
}

/* ---------- drawing routines --------------------------------- */
void paintBackground()
{
    fb.fillSprite(TFT_CYAN);
    for (auto &p : game.scenes[0].platforms)
        fb.fillRect(p.x, p.y, p.w, p.h, PLATFORM_COL);
}
void paintEntities()
{
    int camX = game.cameraX;
    for (auto &e : game.scenes[0].entities)
        fb.fillRect(e.x - camX, e.y, e.w, e.h, colourFromSkin(e.skin));
}

/* ---------- setup -------------------------------------------- */
void setup()
{
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);                 // 480×320 landscape
    tft.setSwapBytes(true);             // correct byte order

    /* 8-bit RGB332 sprite: 480×320×1 = 153 kB */
    fb.setColorDepth(8);
    if (!fb.createSprite(SCR_W, SCR_H)) {
        Serial.println("Framebuffer alloc FAILED!");
        while (true) {}
    }

    game.parseWorld("whatever");

    paintBackground();
    paintEntities();
    fb.pushSprite(0,0);                 // first frame
}

/* ---------- main loop ---------------------------------------- */
uint32_t lastRender = 0;                // ms
uint32_t fpsTimer   = 0;
uint16_t frames     = 0, fps = 0;

void loop()
{
    game.main_tick();                   // logic every tick

    uint32_t now = millis();
    if (now - lastRender >= 33) {       // ~30 fps cap
        lastRender = now;

        paintBackground();
        paintEntities();
        fb.pushSprite(0,0);             // single DMA burst

        if (++frames, now - fpsTimer >= 1000) {
            fps = frames; frames = 0; fpsTimer += 1000;
            Serial.printf("FPS: %u\n", fps);
        }
    }
}
