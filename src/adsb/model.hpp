#pragma once

#include <string>

struct Aircraft {
    std::string hex;         // ICAO 24-bit transponder address
    std::string flight;      // callsign / flight number
    std::string type;        // aircraft type designator (A320, B738, ...)
    std::string destination; // route destination airport (may be empty)
    double lat = 0.0;
    double lon = 0.0;
    double track = 0.0;      // heading in degrees, 0 = north, clockwise
    double altBaro = 0.0;    // barometric altitude in feet
    double gs = 0.0;         // ground speed in knots
    double distanceKm = 0.0;
    bool hasPos = false;
};
