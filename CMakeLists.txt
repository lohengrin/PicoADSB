cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PicoADSB C CXX ASM)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Shared Pico-Toolset (git submodule by default; -DPICO_TOOLSET_DIR=... to use
# another checkout).
set(PICO_TOOLSET_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/pico-toolset" CACHE PATH "Pico-Toolset checkout")
if(NOT EXISTS "${PICO_TOOLSET_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Pico-Toolset not found in ${PICO_TOOLSET_DIR}: run "
                        "'git submodule update --init' or pass -DPICO_TOOLSET_DIR=<checkout>")
endif()

# pico-sdk reads PICO_MBEDTLS_CONFIG_FILE in pico_sdk_init(): set it first.
include(${PICO_TOOLSET_DIR}/cmake/pico_toolset_tls_config.cmake)

pico_sdk_init()

set(PICO_TOOLSET_BUILD_EPD_2IN13_V4   ON  CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_JSON           ON  CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_WIFI           ON  CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_HTTPS_CLIENT   ON  CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_SSD1306        OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_ILI9486        OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_ST7789         OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_ST7796         OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_XPT2046        OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_PSRAM          OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_USB_HID        OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_USB_COMPOSITE  OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_SDCARD         OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_FLASH_STORE    OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_RGB_LED        OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_RESET_BUTTONS  OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_DVI_HDMI       OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_FAULT_HANDLER  OFF CACHE BOOL "" FORCE)
set(PICO_TOOLSET_BUILD_SCREEN         OFF CACHE BOOL "" FORCE)
# EXCLUDE_FROM_ALL: don't build the toolset's example programs.
add_subdirectory(${PICO_TOOLSET_DIR} pico-toolset EXCLUDE_FROM_ALL)

add_executable(picoadsb
    src/main.cpp
    src/adsb/adsb_client.cpp
    src/ui/radar.cpp
    src/ui/details.cpp
    src/util/wallclock.cpp
)

target_include_directories(picoadsb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(picoadsb
    pico_stdlib
    pico_toolset_epd_2in13_v4
    pico_toolset_json
    pico_toolset_wifi
    pico_toolset_https_client
)

pico_enable_stdio_usb(picoadsb 1)
pico_enable_stdio_uart(picoadsb 0)

pico_add_extra_outputs(picoadsb)
