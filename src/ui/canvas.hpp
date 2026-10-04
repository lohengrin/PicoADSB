#pragma once

#include "pico_toolset/epd_2in13_v4.h"

namespace canvas {

// The panel is used in landscape orientation: its native portrait buffer
// (122x250) is rotated by PAINT_ROTATE so the virtual canvas becomes 250x122.
// Switch to 270 if the picture appears upside down on the hardware.
inline constexpr int PAINT_ROTATE = 90;
inline constexpr int W = pico_toolset::Epd2in13V4::kHeight;  // 250
inline constexpr int H = pico_toolset::Epd2in13V4::kWidth;   // 122

}  // namespace canvas
