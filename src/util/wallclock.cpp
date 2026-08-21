#include "util/wallclock.hpp"

#include <ctime>

namespace wallclock {

namespace {

// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm).
int64_t daysFromCivil(int64_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

// Last Sunday of a 31-day month (March or October), as day-of-month.
int lastSunday(int year, int month) {
    // weekday of the 31st, with 0 = Sunday: 1970-01-01 was a Thursday (=4).
    const int w = static_cast<int>((daysFromCivil(year, month, 31) + 4) % 7);
    return 31 - w;
}

}  // namespace

bool localTime(int64_t utcSec, int& hour, int& min, int& sec) {
    if (utcSec < 1735689600) return false;  // before 2025-01-01 -> not plausible

    time_t now = static_cast<time_t>(utcSec);

    // EU DST switch happens at 01:00 UTC.
    struct tm utc;
    gmtime_r(&now, &utc);
    const int year = utc.tm_year + 1900;
    const int64_t dstStart =
        daysFromCivil(year, 3, lastSunday(year, 3)) * 86400 + 3600;
    const int64_t dstEnd =
        daysFromCivil(year, 10, lastSunday(year, 10)) * 86400 + 3600;
    const int offsetSec = (utcSec >= dstStart && utcSec < dstEnd) ? 7200 : 3600;

    now += offsetSec;
    gmtime_r(&now, &utc);
    hour = utc.tm_hour;
    min = utc.tm_min;
    sec = utc.tm_sec;
    return true;
}

}  // namespace wallclock
