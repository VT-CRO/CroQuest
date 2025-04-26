/*****************************************************************
 *  CroQuest – 480×320 single-buffer renderer (no scaling)
 *             ~30 fps, 8-bit RGB332 (≈153 kB) – fits SRAM
 *****************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

/* ---------- globals ------------------------------------------ */
TFT_eSPI    tft;                          // 8-bit parallel (ILI9486)

/* ---------- colour helpers ----------------------------------- */
static inline uint16_t hex565(const char *hex)
{
    auto h=[](char c){return (c<='9'?c-'0':(c|32)-'a'+10);};
    uint8_t r=(h(hex[1])<<4)|h(hex[2]),
            g=(h(hex[3])<<4)|h(hex[4]),
            b=(h(hex[5])<<4)|h(hex[6]);
    return tft.color565(r,g,b);
}


/* ---------- setup -------------------------------------------- */
void setup()
{
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);                 // 480×320 landscape
    tft.setSwapBytes(true);             // correct byte order

}

/* ---------- main loop ---------------------------------------- */
uint32_t lastRender = 0;                // ms
uint32_t fpsTimer   = 0;
uint16_t frames     = 0, fps = 0;

void loop()
{

    uint32_t now = millis();
    if (now - lastRender >= 33) {       // ~30 fps cap
        lastRender = now;


        if (++frames, now - fpsTimer >= 1000) {
            fps = frames; frames = 0; fpsTimer += 1000;
            Serial.printf("FPS: %u\n", fps);
        }
    }
}
