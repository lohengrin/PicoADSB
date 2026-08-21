#include "display/display.hpp"

#include "display/epd_c_api.h"

namespace display {

uint8_t fb[FB_SIZE];

// Next refresh must do a full panel update (boot or after a deep clear);
// in between, gentle partial updates are used to avoid the black/white blink.
static bool s_fullRefresh = true;

bool init() {
    if (DEV_Module_Init() != 0) {
        return false;
    }
    EPD_2in13_V4_Init();
    Paint_NewImage(fb, WIDTH, HEIGHT, PAINT_ROTATE, WHITE);
    Paint_SelectImage(fb);
    s_fullRefresh = true;
    return true;
}

void clearFb() {
    Paint_Clear(WHITE);
}

bool refresh() {
    if (s_fullRefresh) {
        // Hardware reset also wakes the panel from deep sleep, then loads
        // both RAM planes and runs the full waveform (one blink),
        // establishing the base frame for subsequent partial updates.
        EPD_2in13_V4_Init();
        EPD_2in13_V4_Display_Base(fb);
        s_fullRefresh = false;
    } else {
        // Partial update: Display_Partial pulses RST itself, which also
        // wakes the panel from deep sleep, then reconfigures border/window.
        EPD_2in13_V4_Display_Partial(fb);
    }
    // Deep-sleep between cycles: e-paper holds its image with ~zero draw,
    // and the panel driver IC powers its internal regulators down.
    EPD_2in13_V4_Sleep();
    return true;
}

void clearScreen() {
    EPD_2in13_V4_Init();
    EPD_2in13_V4_Clear();
    s_fullRefresh = true;  // next refresh must rebuild the base frame
}

void sleep() {
    EPD_2in13_V4_Sleep();
}

}  // namespace display
