#pragma once

#include <cstdint>

// Waveshare Pico-ePaper 2.13 (V4): 122 x 250, 1 bit per pixel.
namespace display {

inline constexpr int WIDTH = 122;
inline constexpr int HEIGHT = 250;
inline constexpr int BYTES_PER_ROW = (WIDTH + 7) / 8;  // 16
inline constexpr int FB_SIZE = BYTES_PER_ROW * HEIGHT;  // 4000

// Framebuffer, laid out row-major with 8 pixels packed per byte (LSB leftmost).
extern uint8_t fb[FB_SIZE];

// Initialises SPI/GPIO and the panel, and binds the Paint library to `fb`.
bool init();

// Clears the framebuffer to white (does not touch the panel).
void clearFb();

// Uploads `fb` to the panel with a full refresh. Keeps the display
// initialised after sleeping in between refresh cycles.
bool refresh();

// Full white panel clear (used periodically to prevent ghosting).
void clearScreen();

// Powers down the panel (deep sleep).
void sleep();

}  // namespace display
