#pragma once

#include <vector>
#include "adsb/model.hpp"

namespace ui {

// Radar occupies left half (0..RADAR_W-1) of the display.
inline constexpr int RADAR_W = 61;   // pixels
inline constexpr int RADAR_H = 250;  // full height

// Draws the radar background (range rings, center cross) and all aircraft
// as oriented triangles. The closest plane is drawn larger.
void drawRadar(const std::vector<Aircraft>& planes,
               double obsLat, double obsLon, double rangeKm);

}  // namespace ui