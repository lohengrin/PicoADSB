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
// The closest plane is drawn as a solid filled triangle.
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
    nx = clampi(nx, RADAR_X0, RADAR_X0 + RADAR_W - 1);
    ny = clampi(ny, RADAR_Y0, RADAR_Y0 + RADAR_H - 1);
    blx = clampi(blx, RADAR_X0, RADAR_X0 + RADAR_W - 1);
    bly = clampi(bly, RADAR_Y0, RADAR_Y0 + RADAR_H - 1);
    brx = clampi(brx, RADAR_X0, RADAR_X0 + RADAR_W - 1);
    bry = clampi(bry, RADAR_Y0, RADAR_Y0 + RADAR_H - 1);

    if (isClosest) {
        // Solid fill via scanline point-in-triangle test
        const int minx = std::min({nx, blx, brx});
        const int maxx = std::max({nx, blx, brx});
        const int miny = std::min({ny, bly, bry});
        const int maxy = std::max({ny, bly, bry});

        auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
            return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
        };

        for (int py = miny; py <= maxy; ++py) {
            for (int px = minx; px <= maxx; ++px) {
                const int d1 = edge(nx, ny, blx, bly, px, py);
                const int d2 = edge(blx, bly, brx, bry, px, py);
                const int d3 = edge(brx, bry, nx, ny, px, py);
                const bool neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
                const bool pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
                if (!(neg && pos)) {
                    Paint_DrawPoint(px, py, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
                }
            }
        }
    } else {
        drawTriangle(nx, ny, blx, bly, brx, bry, BLACK);
    }
}

void drawRadar(const std::vector<Aircraft>& planes,
               double obsLat, double obsLon, double rangeKm) {
    // Radar center and scale (limited by the smaller dimension)
    const int cx = RADAR_X0 + RADAR_W / 2;
    const int cy = RADAR_Y0 + RADAR_H / 2;
    const double scale = static_cast<double>(std::min(RADAR_W, RADAR_H) / 2) / rangeKm;  // px per km

    // Clear to white (already done by caller via clearFb)

    // Range rings
    for (int i = 1; i <= 3; ++i) {
        const int r = static_cast<int>(scale * rangeKm * i / 3.0 + 0.5);
        if (r > 0 && r < std::min(cx - RADAR_X0, cy - RADAR_Y0)) {
            Paint_DrawCircle(cx, cy, r, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        }
    }

    // Center cross
    Paint_DrawLine(cx - 5, cy, cx + 5, cy, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(cx, cy - 5, cx, cy + 5, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // N/S/E/W labels using Font12 (larger), near the radar box edges
    Paint_DrawString_EN(cx - 3, RADAR_Y0 + 2, "N", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(cx - 3, RADAR_Y0 + RADAR_H - 17, "S", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(RADAR_X0 + 1, cy - 8, "W", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(RADAR_X0 + RADAR_W - 13, cy - 8, "E", &Font12, BLACK, WHITE);

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

        if (px < RADAR_X0 || px >= RADAR_X0 + RADAR_W ||
            py < RADAR_Y0 || py >= RADAR_Y0 + RADAR_H) continue;

        const int size = (i == closestIdx) ? 7 : 5;
        drawAircraftTriangle(px, py, a.track, size, i == closestIdx);
    }
}

}  // namespace ui