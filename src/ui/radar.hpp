#pragma once

#include <vector>
#include "adsb/model.hpp"
#include "ui/canvas.hpp"

namespace ui {

// Left half of the landscape canvas (250x122).
inline constexpr int RADAR_X0 = 0;
inline constexpr int RADAR_Y0 = 0;
inline constexpr int RADAR_W = canvas::W / 2;  // 125
inline constexpr int RADAR_H = canvas::H;      // 122

// Draws the radar background (range rings, center cross) and all aircraft
// as oriented triangles. The closest plane is drawn larger.
void drawRadar(const std::vector<Aircraft>& planes,
               double obsLat, double obsLon, double rangeKm);

}  // namespace ui
