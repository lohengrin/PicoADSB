# Waveshare Pico-ePaper 2.13 (V4) C library, vendored from
# /home/lohengrin/PICO/Pico_ePaper_Code/c/lib (MIT license).
# Compiled as C and exposed to C++ via src/display/epd_c_api.h.

add_library(epd STATIC
    Config/DEV_Config.c
    e-Paper/EPD_2in13_V4.c
    Fonts/font8.c
    Fonts/font12.c
    Fonts/font16.c
    Fonts/font20.c
    Fonts/font24.c
    GUI/GUI_Paint.c
)

target_include_directories(epd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/e-Paper
    ${CMAKE_CURRENT_SOURCE_DIR}/Fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/GUI
)

target_link_libraries(epd pico_stdlib hardware_spi)
