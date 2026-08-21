#include "display/display.hpp"

#include "display/epd_c_api.h"

namespace display {

uint8_t fb[FB_SIZE];

bool init() {
    if (DEV_Module_Init() != 0) {
        return false;
    }
    EPD_2in13_V4_Init();
    Paint_NewImage(fb, WIDTH, HEIGHT, PAINT_ROTATE, WHITE);
    Paint_SelectImage(fb);
    return true;
}

void clearFb() {
    Paint_Clear(WHITE);
}

bool refresh() {
    // The panel needs a re-init after a deep-sleep cycle.
    EPD_2in13_V4_Init();
    EPD_2in13_V4_Display(fb);
    EPD_2in13_V4_Sleep();
    return true;
}

void clearScreen() {
    EPD_2in13_V4_Init();
    EPD_2in13_V4_Clear();
    EPD_2in13_V4_Sleep();
}

void sleep() {
    EPD_2in13_V4_Sleep();
}

}  // namespace display
