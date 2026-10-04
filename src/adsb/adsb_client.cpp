#include "adsb/adsb_client.hpp"

#include <cstdio>

#include "adsb/geo.hpp"
#include "adsb/json.hpp"

namespace adsb {

std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
    return s.substr(b, e - b);
}

std::string buildPointPath(double lat, double lon, int radiusNm) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "/v2/point/%.4f/%.4f/%d", lat, lon, radiusNm);
    return buf;
}

std::string buildRoutePath(const std::string& callsign, double lat, double lon) {
    char buf[96];
    std::snprintf(buf, sizeof(buf), "/api/0/route/%s/%.4f/%.4f", callsign.c_str(), lat, lon);
    return buf;
}

// Parses a /v2/point response body. The server already limits results to the
// requested radius, so every aircraft is accepted; distanceKm is still
// annotated for radar drawing and closest-plane selection.
bool parseAircraftResponse(const char* body, size_t len, double obsLat,
                           double obsLon, std::vector<Aircraft>& out) {
    json::Reader r(body, len);
    out.clear();

    std::string key;
    if (!r.beginObject()) return false;

    while (r.nextObjectMember(key)) {
        if (key != "ac") {
            r.skipValue();
            continue;
        }
        if (!r.beginArray()) return false;

        std::string member;
        while (r.nextArrayElement()) {
            if (!r.beginObject()) return false;

            Aircraft a;
            bool hasLat = false, hasLon = false;
            while (r.nextObjectMember(member)) {
                if (member == "hex") {
                    r.parseString(a.hex);
                } else if (member == "flight") {
                    r.parseString(a.flight);
                } else if (member == "lat") {
                    hasLat = r.parseNumber(a.lat);
                } else if (member == "lon") {
                    hasLon = r.parseNumber(a.lon);
                } else if (member == "track") {
                    r.parseNumber(a.track);
                } else if (member == "t") {
                    r.parseString(a.type);
                } else if (member == "alt_baro") {
                    r.parseNumber(a.altBaro);
                } else if (member == "gs") {
                    r.parseNumber(a.gs);
                } else {
                    r.skipValue();
                }
            }
            if (!r.valid()) return false;

            a.hex = trim(a.hex);
            a.flight = trim(a.flight);
            a.type = trim(a.type);
            a.hasPos = hasLat && hasLon;
            if (a.hasPos) {
                a.distanceKm = geo::distanceKm(obsLat, obsLon, a.lat, a.lon);
                out.push_back(std::move(a));
            }
        }
        if (!r.valid()) return false;
    }
    return r.valid();
}

bool parseRouteDestination(const char* body, size_t len, std::string& dest) {
    dest.clear();
    json::Reader r(body, len);

    std::string key;
    if (!r.beginObject()) return false;

    // Route response has "airport_codes": "LEAL-EBBR" and
    // "_airport_codes_iata": "ALC-BRU". Prefer the IATA code.
    std::string iata, icao;
    while (r.nextObjectMember(key)) {
        if (key == "airport_codes") {
            r.parseString(icao);
        } else if (key == "_airport_codes_iata") {
            r.parseString(iata);
        } else {
            r.skipValue();
        }
    }
    if (!r.valid()) return false;

    const std::string& src = !iata.empty() ? iata : icao;
    if (src.empty() || src == "unknown") return false;

    const size_t dash = src.find_last_of('-');
    dest = (dash == std::string::npos) ? src : src.substr(dash + 1);
    dest = trim(dest);
    return !dest.empty();
}

bool parseResponseTime(const char* body, size_t len, long long& epochSec) {
    json::Reader r(body, len);
    std::string key;

    if (!r.beginObject()) return false;

    bool gotNow = false, gotCtime = false;
    double nowVal = 0, ctimeVal = 0;
    while (r.nextObjectMember(key)) {
        if (key == "now") {
            if (!r.parseNumber(nowVal)) return false;
            gotNow = true;
        } else if (key == "ctime") {
            if (!r.parseNumber(ctimeVal)) return false;
            gotCtime = true;
        } else {
            r.skipValue();
        }
        if (gotNow && gotCtime) break;  // enough, don't scan the whole "ac" array
    }
    if (!r.valid()) return false;

    const double v = gotNow ? nowVal : (gotCtime ? ctimeVal : 0.0);
    if (v <= 0) return false;
    // readsb-style timestamps are milliseconds.
    epochSec = static_cast<long long>(v >= 1e11 ? v / 1000.0 : v);
    return true;
}

}  // namespace adsb
