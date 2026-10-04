#include <cstdio>
#include <vector>
#include <string>

#include "pico/stdlib.h"

#include "config.h"
#include "display/display.hpp"
#include "display/epd_c_api.h"
#include "net/wifi.hpp"
#include "net/https_client.hpp"
#include "adsb/adsb_client.hpp"
#include "adsb/model.hpp"
#include "ui/radar.hpp"
#include "ui/details.hpp"

int main() {
    stdio_init_all();

    // Display init
    if (!display::init()) {
        for (;;) tight_loop_contents();
    }
    display::clearFb();
    display::refresh();

    // WiFi
    if (!net::wifiInitAndConnect()) {
        display::clearFb();
        Paint_DrawString_EN(10, 120, "NO WIFI", &Font16, BLACK, WHITE);
        display::refresh();
        for (;;) tight_loop_contents();
    }

    int refreshCount = 0;
    long long lastUpdateUtc = 0;
    std::string body;
    std::vector<Aircraft> planes;

    while (true) {
        bool ok = false;
        const int radiusNm = static_cast<int>(RANGE_KM / 1.852 + 0.5);  // km -> NM

        // Build /v2/point URL
        const std::string path = adsb::buildPointPath(OBSERVER_LAT, OBSERVER_LON, radiusNm);

        body.clear();
        if (net::httpsGet("api.adsb.lol", path.c_str(), body)) {
            planes.clear();
            if (adsb::parseAircraftResponse(body.data(), body.size(),
                                            OBSERVER_LAT, OBSERVER_LON, planes)) {
                ok = true;

                // Wall-clock anchor: the API response carries its own
                // timestamp ("now"/"ctime"), so no NTP is needed.
                long long respSec = 0;
                if (adsb::parseResponseTime(body.data(), body.size(), respSec)) {
                    lastUpdateUtc = respSec;
                }

                // Find closest plane
                const Aircraft* closest = nullptr;
                for (const auto& a : planes) {
                    if (!closest || a.distanceKm < closest->distanceKm) closest = &a;
                }

                // Fetch destination for closest via /api/0/route
                std::string dest;
                if (closest && !closest->flight.empty()) {
                    const std::string callsign = adsb::trim(closest->flight);
                    const std::string rpath = adsb::buildRoutePath(callsign, closest->lat, closest->lon);
                    std::string rbody;
                    if (net::httpsGet("api.adsb.lol", rpath.c_str(), rbody)) {
                        adsb::parseRouteDestination(rbody.data(), rbody.size(), dest);
                    }
                }

                // Render
                display::clearFb();
                ui::drawRadar(planes, OBSERVER_LAT, OBSERVER_LON, RANGE_KM);
                if (closest) {
                    Aircraft c = *closest;  // copy to add destination
                    if (!dest.empty()) c.destination = dest;
                    ui::drawDetails(c, lastUpdateUtc);
                }
                refreshCount++;
                if (refreshCount % CLEAR_EVERY_N == 0) {
                    display::clearScreen();  // full clear to prevent ghosting
                }
                display::refresh();
            }
        }

        if (!ok) {
            printf("fetch/parse failed, keeping last frame\n");
        }

        sleep_ms(REFRESH_SEC * 1000);
    }
}