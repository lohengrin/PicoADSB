cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PicoADSB C CXX ASM)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Use our mbedtls config wrapper that properly includes the standard config
set(PICO_MBEDTLS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/net/mbedtls_config_wrapper.h")

pico_sdk_init()

add_subdirectory(third_party/epd)

add_executable(picoadsb
    src/main.cpp
    src/adsb/adsb_client.cpp
    src/adsb/json.cpp
    src/net/https_client.cpp
    src/net/wifi.cpp
    src/net/mbedtls_platform_time.c
    src/display/display.cpp
    src/ui/radar.cpp
    src/ui/details.cpp
)

target_include_directories(picoadsb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/epd
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net
)

target_link_libraries(picoadsb
    epd
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
    hardware_spi
)

# mbedtls 3.6.6 added psa_crypto_random.c but SDK 2.3.0's pico_mbedtls source
# list predates it; compile it here rather than patching the SDK
target_sources(picoadsb PRIVATE ${PICO_SDK_PATH}/lib/mbedtls/library/psa_crypto_random.c)

pico_enable_stdio_usb(picoadsb 1)
pico_enable_stdio_uart(picoadsb 0)

pico_add_extra_outputs(picoadsb)

# ---------- host unit tests (native) ----------
option(PICOADSB_HOST_TESTS "Build host-side unit tests" OFF)
if(PICOADSB_HOST_TESTS)
    enable_testing()
    add_executable(unit_tests
        tests/unit_tests.cpp
        src/adsb/json.cpp
        src/adsb/adsb_client.cpp
        src/adsb/geo.hpp
    )
    target_include_directories(unit_tests PRIVATE src)
    target_compile_definitions(unit_tests PRIVATE PICOADSB_HOST_TESTS)
    add_test(NAME unit_tests COMMAND unit_tests)
endif()
