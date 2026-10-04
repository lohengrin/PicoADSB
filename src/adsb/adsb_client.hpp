#pragma once

#include <string>
#include <vector>

#include "adsb/model.hpp"

namespace adsb {

// Path portion of the /v2/point URL (radius in nautical miles, max 250).
std::string buildPointPath(double lat, double lon, int radiusNm);

// Path for a single-plane route lookup, used to derive the destination.
std::string buildRoutePath(const std::string& callsign, double lat, double lon);

// Parses a /v2/point response body (the server already limits results to the
// requested radius), annotating each aircraft with distanceKm to the
// observer. Returns false on error.
bool parseAircraftResponse(const char* body, size_t len, double obsLat,
                           double obsLon, std::vector<Aircraft>& out);

// Parses a /api/0/route response body and stores the destination airport code
// in dest. Returns false if unknown/unavailable.
bool parseRouteDestination(const char* body, size_t len, std::string& dest);

// Extracts the response timestamp ("now", falling back to "ctime") from a
// /v2/point body as UTC epoch seconds. Used as wall-clock source since the
// Pico has no RTC and we do not run SNTP.
bool parseResponseTime(const char* body, size_t len, long long& epochSec);

// Strips leading/trailing whitespace (adsb.lol pads callsigns to 8 chars).
std::string trim(const std::string& s);

}  // namespace adsb
