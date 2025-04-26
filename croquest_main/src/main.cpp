/*****************************************************************
 *  CroQuest – 480×320 single-buffer renderer (no scaling)
 *             ~30 fps, 8-bit RGB332 (≈153 kB) – fits SRAM
 *****************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "btTester.hpp"

/* ---------- globals ------------------------------------------ */
TFT_eSPI    tft;                          // 8-bit parallel (ILI9486)
btTester*   tester = nullptr;              // Bluetooth tester instance

// Set to true for host mode, false for guest mode
const bool IS_HOST = true;                 // Change this to false for guest mode

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
    Serial.println("CroQuest Bluetooth Tester");
    
    tft.init();
    tft.setRotation(1);                 // 480×320 landscape
    tft.setSwapBytes(true);             // correct byte order
    
    // Create btTester instance
    tester = new btTester(&tft, IS_HOST);
    
    Serial.println(IS_HOST ? "Host Mode" : "Guest Mode");
    Serial.println("Host Code: 292");
}

/* ---------- main loop ---------------------------------------- */
uint32_t lastRender = 0;                // ms
uint32_t fpsTimer   = 0;
uint16_t frames     = 0, fps = 0;

void loop()
{
    // Update and draw btTester
    if (tester != nullptr) {
        tester->update();
        tester->draw();
    }

    uint32_t now = millis();
    if (now - lastRender >= 33) {       // ~30 fps cap
        lastRender = now;

        if (++frames, now - fpsTimer >= 1000) {
            fps = frames; frames = 0; fpsTimer += 1000;
            Serial.printf("FPS: %u\n", fps);
        }
    }
}
