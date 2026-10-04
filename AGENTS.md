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
- License: MIT (`LICENSE.md`)

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
git submodule update --init                        # third_party/pico-toolset
cp src/config.local.h.example src/config.local.h   # WiFi credentials (gitignored)
cmake -B build -S . -DPICO_BOARD=pico_w -DPICO_SDK_PATH=/home/lohengrin/PICO/pico-sdk
cmake --build build --target picoadsb -j
```

Gotchas:
- The shell may export `PICO_PLATFORM=rp2350`; prefix commands with
  `env -u PICO_PLATFORM` or configure fails / builds the wrong target.
- Shared code lives in the Pico-Toolset submodule (`third_party/pico-toolset`,
  `-DPICO_TOOLSET_DIR=<checkout>` to use another): e-paper driver + GUI_Paint
  (`pico_toolset_epd_2in13_v4`), JSON reader, Wi-Fi, HTTPS client. Fix those
  there, not here. Only the ADS-B code, UI and `src/util/wallclock` stay local.
- The SDK tree must stay pristine. The mbedtls 3.6.6 `psa_crypto_random.c`
  workaround, the TLS 1.2 mbedtls config (ECDHE_RSA/ECDHE_ECDSA — adsb.lol
  needs ECDHE-RSA-AES256-GCM-SHA384) and `lwipopts.h` ship with the toolset's
  `pico_toolset_https_client`; `CMakeLists.txt` includes its
  `cmake/pico_toolset_tls_config.cmake` before `pico_sdk_init()`.
- CA pin: ISRG Root YR (`pico_toolset::tls_roots::kIsrgRootYrPem`).

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
- Display refresh (`Epd2in13V4::update()`): full path = `init_panel()` +
  `display_base()` (one blink), partial path = `display_partial()` only; both
  end with `sleep()`. `clear_screen()` forces a full refresh next cycle.
- HTTPS client (toolset `pico_toolset::HttpsClient`): persistent keep-alive TLS
  connection reused across cycles; callbacks only set flags — requests are
  written from the main-loop context, never inside lwIP input processing;
  poll watchdog armed only while a request is in flight.
- Do not commit unless asked to.
