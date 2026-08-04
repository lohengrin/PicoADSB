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
    return true;
}

}  // namespace net