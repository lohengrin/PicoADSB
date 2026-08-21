#pragma once

#include <cstdint>

namespace wallclock {

// Converts a UTC epoch timestamp to Paris local time (EU DST rules:
// CEST = UTC+2 between last Sunday of March and last Sunday of October,
// CET = UTC+1 otherwise). Returns false for out-of-range input.
bool localTime(int64_t utcSec, int& hour, int& min, int& sec);

}  // namespace wallclock
