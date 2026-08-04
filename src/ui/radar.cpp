#include "ui/radar.hpp"

#include <cmath>
#include <algorithm>

#include "display/display.hpp"
#include "display/epd_c_api.h"
#include "adsb/geo.hpp"

namespace ui {

// Small helper to clamp a value into [lo, hi].
inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Draw a filled triangle by drawing three lines between vertices.
static void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, UWORD color) {
    Paint_DrawLine(x1, y1, x2, y2, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(x2, y2, x3, y3, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(x3, y3, x1, y1, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

// Draw an aircraft triangle at (cx, cy) oriented by track degrees.
// size = radius of the triangle (nose distance from center).
static void drawAircraftTriangle(int cx, int cy, double trackDeg, int size, bool isClosest) {
    const double h = trackDeg * geo::PI / 180.0;
    // Nose
    int nx = cx + static_cast<int>(std::sin(h) * size + 0.5);
    int ny = cy - static_cast<int>(std::cos(h) * size + 0.5);
    // Back left/right at ~120 degrees from nose, length size*0.6
    const double backAngle = 2.0944;  // 120 deg
    const double backLen = size * 0.6;
    int blx = cx + static_cast<int>(std::sin(h + backAngle) * backLen + 0.5);
    int bly = cy - static_cast<int>(std::cos(h + backAngle) * backLen + 0.5);
    int brx = cx + static_cast<int>(std::sin(h - backAngle) * backLen + 0.5);
    int bry = cy - static_cast<int>(std::cos(h - backAngle) * backLen + 0.5);

    // Clamp to radar box
    nx = clampi(nx, 0, RADAR_W - 1);
    ny = clampi(ny, 0, RADAR_H - 1);
    blx = clampi(blx, 0, RADAR_W - 1);
    bly = clampi(bly, 0, RADAR_H - 1);
    brx = clampi(brx, 0, RADAR_W - 1);
    bry = clampi(bry, 0, RADAR_H - 1);

    UWORD color = isClosest ? BLACK : BLACK;
    drawTriangle(nx, ny, blx, bly, brx, bry, color);

    // For closest plane, draw a small circle around it
    if (isClosest) {
        Paint_DrawCircle(cx, cy, size + 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    }
}

void drawRadar(const std::vector<Aircraft>& planes,
               double obsLat, double obsLon, double rangeKm) {
    // Radar center and scale
    const int cx = RADAR_W / 2;
    const int cy = RADAR_H / 2;
    const double scale = static_cast<double>(cx) / rangeKm;  // px per km

    // Clear to white (already done by caller via clearFb)

    // Range rings
    for (int i = 1; i <= 3; ++i) {
        const int r = static_cast<int>(scale * rangeKm * i / 3.0 + 0.5);
        if (r > 0) {
            Paint_DrawCircle(cx, cy, r, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        }
    }

    // Center cross
    Paint_DrawLine(cx - 5, cy, cx + 5, cy, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(cx, cy - 5, cx, cy + 5, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // N/S/E/W labels using Font8 (small)
    Paint_DrawString_EN(cx + 2, cy - 5 - 8, "N", &Font8, BLACK, WHITE);
    Paint_DrawString_EN(cx + 2, cy + 5, "S", &Font8, BLACK, WHITE);
    Paint_DrawString_EN(cx - 5 - 6, cy - 4, "W", &Font8, BLACK, WHITE);
    Paint_DrawString_EN(cx + 5 + 2, cy - 4, "E", &Font8, BLACK, WHITE);

    // Find closest plane index
    size_t closestIdx = SIZE_MAX;
    double minDist = 1e9;
    for (size_t i = 0; i < planes.size(); ++i) {
        if (planes[i].hasPos && planes[i].distanceKm < minDist) {
            minDist = planes[i].distanceKm;
            closestIdx = i;
        }
    }

    // Draw all aircraft
    for (size_t i = 0; i < planes.size(); ++i) {
        const Aircraft& a = planes[i];
        if (!a.hasPos) continue;

        // Project lat/lon to radar coordinates (equirectangular)
        const double cosLat = std::cos(obsLat * geo::PI / 180.0);
        const double dxKm = (a.lon - obsLon) * 111.32 * cosLat;
        const double dyKm = (a.lat - obsLat) * 111.32;

        int px = cx + static_cast<int>(dxKm * scale + 0.5);
        int py = cy - static_cast<int>(dyKm * scale + 0.5);

        if (px < 0 || px >= RADAR_W || py < 0 || py >= RADAR_H) continue;

        const int size = (i == closestIdx) ? 7 : 5;
        drawAircraftTriangle(px, py, a.track, size, i == closestIdx);
    }
}

}  // namespace ui