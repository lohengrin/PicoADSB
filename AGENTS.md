# Pico ADSB

## Description

Firmware that fetches live ADS-B aircraft data from the free `https://adsb.lol/`
API and displays nearby traffic around the observer's position (default range
25 km, configurable in `src/config.h`).

Landscape layout on the e-Paper:
- Left half: radar view centered on the observer; one triangle per aircraft,
  oriented by heading; the closest aircraft is filled.
- Right half: closest-aircraft details (type decoded via
  `src/adsb/type_names.hpp`, flight, destination, distance, altitude, speed)
  plus the last-update clock (HH:MM:SS, top-right, taken from the API
  response timestamp — no NTP/RTC).

- C++20, CMake, pico-sdk 2.3.0 at `/home/lohengrin/PICO/pico-sdk`
- License: GPL-3.0 (`LICENSE.md`)

## Hardware

- Raspberry Pico W (RP2040 + Wireless)
- ePaper ink screen : Waveshare Pico-ePaper 2.13 (V4), plugged on the Pico header

## Documentation

**adsb.lol** `https://api.adsb.lol/api/openapi.json`
- `/v2/point/{lat}/{lon}/{radiusNm}` — aircraft list; body carries `now` /
  `ctime` epoch **milliseconds** after the `ac` array.
- `/api/0/route/{callsign}/{lat}/{lon}` — destination airport codes.

**Waveshare Pico-ePaper 2.13 (V4)**
 - Doc: `https://www.waveshare.com/wiki/Pico-ePaper-2.13?srsltid=AfmBOooa5fpbz7vxZWxV2gdXz5d5i0YEGMS8QjKrqKzSdAcFszFMwIBn` 
 - Sample code: `/home/lohengrin/PICO/Pico_ePaper_Code`

## Build

```sh
cp src/config.local.h.example src/config.local.h   # WiFi credentials (gitignored)
cmake -B build -S . -DPICO_BOARD=pico_w -DPICO_SDK_PATH=/home/lohengrin/PICO/pico-sdk
cmake --build build --target picoadsb -j
```

Gotchas:
- The shell may export `PICO_PLATFORM=rp2350`; prefix commands with
  `env -u PICO_PLATFORM` or configure fails / builds the wrong target.
- The SDK tree must stay pristine. mbedtls 3.6.6 added
  `psa_crypto_random.c`, missing from the SDK's pico_mbedtls file list; it is
  compiled directly into the project from `CMakeLists.txt` instead of patching
  the SDK.
- TLS config wrapper: `src/net/mbedtls_config_wrapper.h` (TLS 1.2,
  ECDHE_RSA/ECDHE_ECDSA enabled — adsb.lol needs ECDHE-RSA-AES256-GCM-SHA384).
  CA pin: ISRG Root X1 (`src/net/isrg_root_yr_pem.h`).

## Integration and Tests

- Flashing can be done using picotool: `picotool load -f -x <uf2 file>`
  (default artifact: `build/picoadsb.uf2`)
- Connection to running Pico: `minicom -b 115200 -o -D /dev/ttyACM0`
- There is no host unit-test target (removed); verify parsers with a small
  standalone g++ program against `src/adsb/*.cpp` if needed.
- Hardware behaviour is verified by the user on the device; do not flash or
  read the serial port unless explicitly asked.

## Code notes / pitfalls

- Waveshare GUI_Paint quirk: `Paint_DrawString_EN` swaps its colour arguments
  internally — effective order is (cell background, glyph colour); background
  == WHITE draws glyphs only (transparent). Panel polarity is normal:
  BLACK(0x00) renders black.
- Display refresh: full path = `Init()` + `Display_Base()` (one blink),
  partial path = `Display_Partial()` only; both end with `Sleep()`.
  `clearScreen()` forces a full refresh next cycle.
- HTTPS client (`src/net/https_client.cpp`): persistent keep-alive TLS
  connection reused across cycles; callbacks only set flags — requests are
  written from the main-loop context, never inside lwIP input processing;
  poll watchdog armed only while a request is in flight.
- Do not commit unless asked to.
