#include "ui/details.hpp"

#include <cstdio>

#include "adsb/type_names.hpp"
#include "display/display.hpp"
#include "display/epd_c_api.h"

namespace ui {

// Colour mapping for this panel (verified on hardware): WHITE (0xFF)
// renders white, BLACK (0x00) renders black. Note the Waveshare library's
// Paint_DrawString_EN internally swaps its colour arguments, so its
// effective order is (cell background, glyph colour) — and a background
// equal to FONT_BACKGROUND (= WHITE) draws glyphs only, which is what we
// want on top of the pane we fill ourselves.
constexpr UWORD InkBlack = BLACK;
constexpr UWORD PaperWhite = WHITE;

static inline void drawLabel(int x, int y, const char* label) {
    Paint_DrawString_EN(x, y, label, &Font8, PaperWhite, InkBlack);
}

static inline void drawValue(int x, int y, const char* value) {
    Paint_DrawString_EN(x, y, value, &Font12, PaperWhite, InkBlack);
}

void drawDetails(const Aircraft& plane) {
    // Paper-white background for the whole pane
    Paint_DrawRectangle(DETAILS_X0, 0, DETAILS_X0 + DETAILS_W - 1,
                        DISPLAY_H - 1, PaperWhite, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    const int colLabel = DETAILS_X0 + 3;
    const int colValue = DETAILS_X0 + 38;

    int y = 3;
    Paint_DrawString_EN(colLabel, y, "CLOSEST", &Font12, PaperWhite, InkBlack);
    y += 15;

    // Separator line
    Paint_DrawLine(DETAILS_X0 + 1, y, DETAILS_X0 + DETAILS_W - 2, y,
                   InkBlack, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    y += 4;

    auto field = [&](const char* label, const char* value) {
        if (y + 13 > DISPLAY_H - 2) return;
        drawLabel(colLabel, y + 2, label);
        drawValue(colValue, y, value);
        y += 15;
    };

    // Type (decoded to the usual name, e.g. A339 -> A330-900neo)
    field("TYPE", plane.type.empty() ? "---" : adsb::typeName(plane.type));

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
