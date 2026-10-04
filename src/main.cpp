#include <cstdio>
#include <vector>
#include <string>

#include "pico/stdlib.h"

#include "config.h"
#include "epd_paint.h"
#include "pico_toolset/epd_2in13_v4.h"
#include "pico_toolset/epd_2in13_v4_configs.h"
#include "pico_toolset/https_client.h"
#include "pico_toolset/tls_roots.h"
#include "pico_toolset/wifi.h"
#include "adsb/adsb_client.hpp"
#include "adsb/model.hpp"
#include "ui/canvas.hpp"
#include "ui/radar.hpp"
#include "ui/details.hpp"

static uint8_t fb[pico_toolset::Epd2in13V4::kFramebufferSize];

int main() {
    stdio_init_all();

    pico_toolset::Epd2in13V4 epd;
    if (!epd.init(pico_toolset::configs::epd_2in13_v4::kWavesharePicoEpaper213)) {
        for (;;) tight_loop_contents();
    }
    Paint_NewImage(fb, pico_toolset::Epd2in13V4::kWidth, pico_toolset::Epd2in13V4::kHeight,
                   canvas::PAINT_ROTATE, WHITE);
    Paint_SelectImage(fb);
    Paint_Clear(WHITE);
    epd.update(fb);

    pico_toolset::WifiConfig wifi;
    wifi.ssid = WIFI_SSID;
    wifi.password = WIFI_PASSWORD;
    if (!pico_toolset::wifi_connect(wifi)) {
        Paint_Clear(WHITE);
        Paint_DrawString_EN(10, 120, "NO WIFI", &Font16, BLACK, WHITE);
        epd.update(fb);
        for (;;) tight_loop_contents();
    }

    pico_toolset::HttpsClientConfig httpsCfg;
    httpsCfg.ca_pem = pico_toolset::tls_roots::kIsrgRootYrPem;
    httpsCfg.ca_pem_len = sizeof(pico_toolset::tls_roots::kIsrgRootYrPem);
    httpsCfg.max_body = HTTP_MAX_BODY;
    httpsCfg.user_agent = "PicoADSB/1.0";
    pico_toolset::HttpsClient https;
    https.init(httpsCfg);

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
        if (https.get("api.adsb.lol", path.c_str(), body)) {
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
                    if (https.get("api.adsb.lol", rpath.c_str(), rbody)) {
                        adsb::parseRouteDestination(rbody.data(), rbody.size(), dest);
                    }
                }

                // Render
                Paint_Clear(WHITE);
                ui::drawRadar(planes, OBSERVER_LAT, OBSERVER_LON, RANGE_KM);
                if (closest) {
                    Aircraft c = *closest;  // copy to add destination
                    if (!dest.empty()) c.destination = dest;
                    ui::drawDetails(c, lastUpdateUtc);
                }
                refreshCount++;
                if (refreshCount % CLEAR_EVERY_N == 0) {
                    epd.clear_screen();  // full clear to prevent ghosting
                }
                epd.update(fb);
            }
        }

        if (!ok) {
            printf("fetch/parse failed, keeping last frame\n");
        }

        sleep_ms(REFRESH_SEC * 1000);
    }
}