#pragma once

#include <cmath>

namespace geo {

inline constexpr double PI = 3.14159265358979323846;
inline constexpr double EARTH_RADIUS_KM = 6371.0;

inline double deg2rad(double deg) { return deg * PI / 180.0; }
inline double rad2deg(double rad) { return rad * 180.0 / PI; }

// Great-circle distance (haversine) in kilometres.
inline double distanceKm(double lat1, double lon1, double lat2, double lon2) {
    const double dLat = deg2rad(lat2 - lat1);
    const double dLon = deg2rad(lon2 - lon1);
    const double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
                     std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) *
                         std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    return EARTH_RADIUS_KM * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

}  // namespace geo
