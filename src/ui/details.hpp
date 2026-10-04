#pragma once

#include "adsb/model.hpp"
#include "ui/canvas.hpp"

namespace ui {

// Right half of the landscape canvas (250x122).
inline constexpr int DETAILS_X0 = canvas::W / 2 + 1;  // 126
inline constexpr int DETAILS_W = canvas::W - DETAILS_X0 - 1;  // 123
inline constexpr int DISPLAY_H = canvas::H;  // 122

// Draws the closest plane info panel. lastUpdateUtc is the UTC epoch of the
// last successful fetch (taken from the API response; shown as HH:MM:SS in
// the header, right-aligned); pass 0 when unknown.
void drawDetails(const Aircraft& plane, long long lastUpdateUtc);

}  // namespace ui
