#include "net/wifi.hpp"

#include <cstdio>
#include "config.h"
#include "pico/cyw43_arch.h"

namespace net {

bool wifiInitAndConnect() {
    if (cyw43_arch_init()) {
        printf("cyw43 init failed\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                            CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("wifi connect failed\n");
        return false;
    }
    printf("wifi connected\n");

    // Modem power saving (PM2): the radio sleeps between traffic bursts
    // and wakes on beacons/data. Big idle-current win on Pico W.
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_PM2_POWERSAVE_MODE, 200, 1, 1, 1));

    return true;
}

}  // namespace net