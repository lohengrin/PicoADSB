#pragma once

// Observer position (home position). Latitude / longitude in decimal degrees.
#define OBSERVER_LAT 48.8287032
#define OBSERVER_LON 2.2668162

// Radar range in kilometers. The adsb.lol API takes nautical miles
// (1 NM = 1.852 km), so 25 km is requested as 13 NM.
#define RANGE_KM 25

// Full refresh period in seconds.
#define REFRESH_SEC 30

// Full clear every N refreshes to prevent e-paper ghosting.
#define CLEAR_EVERY_N 10

// Maximum HTTP response body size in bytes (the JSON payload).
#define HTTP_MAX_BODY 16384

// Include private WiFi credentials (src/config.local.h, gitignored).
#ifndef PICOADSB_HOST_TESTS
#include "config.local.h"

#ifndef WIFI_SSID
#error "WIFI_SSID must be defined in src/config.local.h (see config.local.h.example)"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD must be defined in src/config.local.h (see config.local.h.example)"
#endif
#endif
