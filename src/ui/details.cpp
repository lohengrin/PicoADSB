#include "ui/details.hpp"

#include <cstdio>

#include "display/display.hpp"
#include "display/epd_c_api.h"

namespace ui {

static inline void drawLabel(int x, int y, const char* label) {
    Paint_DrawString_EN(x, y, label, &Font8, BLACK, WHITE);
}

static inline void drawValue(int x, int y, const char* value) {
    Paint_DrawString_EN(x, y, value, &Font12, BLACK, WHITE);
}

static inline void drawValue(int x, int y, const std::string& value) {
    drawValue(x, y, value.c_str());
}

void drawDetails(const Aircraft& plane) {
    int x0 = DETAILS_X0;
    int col1 = x0;
    int col2 = x0 + 4;  // value starts 4px after label

    int y = 4;
    // Title
    Paint_DrawString_EN(col1, y, "CLOSEST", &Font12, BLACK, WHITE);
    y += 16;

    // Separator line
    Paint_DrawLine(x0, y, x0 + DETAILS_W - 1, y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    y += 4;

    auto field = [&](const char* label, const char* value) {
        if (y + 12 > DISPLAY_H) return;
        drawLabel(col1, y, label);
        y += 10;
        if (y + 12 > DISPLAY_H) return;
        drawValue(col2, y, value);
        y += 14;
    };

    // Type
    field("TYPE", plane.type.empty() ? "---" : plane.type.c_str());

    // Flight (callsign)
    field("FLIGHT", plane.flight.empty() ? "---" : plane.flight.c_str());

    // Destination
    field("DEST", plane.destination.empty() ? "---" : plane.destination.c_str());

    // Distance
    if (plane.hasPos) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f km", plane.distanceKm);
        field("DIST", buf);
    } else {
        field("DIST", "---");
    }

    // Altitude
    if (plane.altBaro > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f ft", plane.altBaro);
        field("ALT", buf);
    } else {
        field("ALT", "---");
    }

    // Ground speed
    if (plane.gs > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f kt", plane.gs);
        field("SPD", buf);
    } else {
        field("SPD", "---");
    }
}

}  // namespace ui