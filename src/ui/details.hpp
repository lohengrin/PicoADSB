#pragma once

#include "adsb/model.hpp"

namespace ui {

// Right half: X from DETAILS_X0 to DISPLAY_WIDTH-1 (60 px wide).
inline constexpr int DETAILS_X0 = 62;
inline constexpr int DETAILS_W = 60;
inline constexpr int DISPLAY_H = 250;

// Draws the closest plane info panel.
void drawDetails(const Aircraft& plane);

}  // namespace ui