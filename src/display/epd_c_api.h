#pragma once

// Exposes the C-only Waveshare e-Paper library to C++ translation units.
// DEV_Config.h / GUI_Paint.h / EPD_2in13_V4.h lack extern "C" guards, so wrap
// them here. fonts.h already has its own guards.

#ifdef __cplusplus
extern "C" {
#endif

#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include "fonts.h"

#ifdef __cplusplus
}
#endif
