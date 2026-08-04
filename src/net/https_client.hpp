#pragma once

#include <string>

namespace net {

// Initialise CYW43 and connect to Wi-Fi using credentials from config.local.h.
// Returns true on success.
bool wifiInitAndConnect();

// Performs HTTPS GET to api.adsb.lol. Returns false on any error.
// On success, outBody contains the response body (without headers).
bool httpsGet(const char* host, const char* path, std::string& outBody);

}  // namespace net