// Host unit tests for json parser and geo functions
#define PICOADSB_HOST_TESTS

#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>
#include <string>

#include "../src/adsb/json.hpp"
#include "../src/adsb/adsb_client.hpp"
#include "../src/adsb/geo.hpp"
#include "../src/adsb/model.hpp"

using namespace json;
using namespace adsb;
using namespace geo;

bool testParseNumber() {
    const char* j = "123.45";
    Reader r(j, 6);
    double val;
    if (!r.parseNumber(val)) return false;
    return std::abs(val - 123.45) < 0.001;
}

bool testParseString() {
    const char* j = "\"hello world\"";
    Reader r(j, 13);
    std::string s;
    if (!r.parseString(s)) return false;
    return s == "hello world";
}

bool testParseNullNumber() {
    const char* j = "null";
    Reader r(j, 4);
    double val = 999;
    if (!r.parseNumber(val)) return false;
    return val == 999;  // unchanged
}

bool testParseNullString() {
    const char* j = "null";
    Reader r(j, 4);
    std::string s = "test";
    if (!r.parseString(s)) return false;
    return s == "test";  // unchanged
}

bool testObjectNavigation() {
    const char* j = "{\"a\":1,\"b\":{\"c\":2},\"d\":[3,4]}";
    Reader r(j, 31);
    if (!r.beginObject()) return false;

    std::string key;
    int count = 0;
    while (r.nextObjectMember(key)) {
        count++;
        if (key == "a") {
            double v; r.parseNumber(v);
        } else if (key == "b") {
            r.beginObject();
            while (r.nextObjectMember(key)) {
                double v; r.parseNumber(v);
            }
        } else if (key == "d") {
            r.beginArray();
            while (r.nextArrayElement()) {
                double v; r.parseNumber(v);
            }
        }
    }
    return count == 3 && r.valid();
}

bool testDistanceKm() {
    // Paris to London ~344 km
    double d = distanceKm(48.8566, 2.3522, 51.5074, -0.1278);
    return d > 340 && d < 350;
}

bool testAircraftParsing() {
    // Simulated /v2/point response
    const char* json = R"({"ac":[{"hex":"4ca87c","flight":"BEL6DN  ","lat":48.86824,"lon":1.670415,"track":28.26,"t":"A320","alt_baro":34025,"gs":483.7,"r":"OO-SNE"},{"hex":"3c6663","flight":"DLH44C  ","lat":48.606914,"lon":1.906706,"track":231.63,"t":"A321","alt_baro":35000,"gs":397.9,"r":"D-AISC"}],"now":1234567890})";

    std::vector<Aircraft> planes;
    bool ok = parseAircraftResponse(json, strlen(json), 48.8566, 2.3522, 50.0, planes);
    if (!ok) return false;
    if (planes.size() != 2) return false;
    if (planes[0].flight != "BEL6DN") return false;
    if (planes[0].type != "A320") return false;
    if (!planes[0].hasPos) return false;
    if (planes[0].distanceKm > 50.0) return false;
    if (planes[0].altBaro != 34025.0) return false;
    if (planes[0].gs != 483.7) return false;
    return true;
}

bool testRouteDestination() {
    const char* json = R"({"airport_codes":"LEAL-EBBR","_airport_codes_iata":"ALC-BRU"})";
    std::string dest;
    bool ok = parseRouteDestination(json, strlen(json), dest);
    return ok && dest == "BRU";
}

bool testRouteDestinationUnknown() {
    const char* json = R"({"airport_codes":"unknown"})";
    std::string dest;
    bool ok = parseRouteDestination(json, strlen(json), dest);
    return !ok;
}

int main() {
    int passed = 0, failed = 0;

    auto run = [&](const char* name, bool (*fn)()) {
        if (fn()) {
            printf("[PASS] %s\n", name);
            passed++;
        } else {
            printf("[FAIL] %s\n", name);
            failed++;
        }
    };

    run("parseNumber", testParseNumber);
    run("parseString", testParseString);
    run("parseNullNumber", testParseNullNumber);
    run("parseNullString", testParseNullString);
    run("objectNavigation", testObjectNavigation);
    run("distanceKm", testDistanceKm);
    run("aircraftParsing", testAircraftParsing);
    run("routeDestination", testRouteDestination);
    run("routeDestinationUnknown", testRouteDestinationUnknown);

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}