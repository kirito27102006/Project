#include "TimeTransport.h"
#include <QString>

// Определение констант
constexpr int TimeTransport::MIN_HOURS;
constexpr int TimeTransport::MAX_HOURS;
constexpr int TimeTransport::MIN_MINUTES;
constexpr int TimeTransport::MAX_MINUTES;
constexpr int TimeTransport::MINUTES_PER_HOUR;
constexpr int TimeTransport::HOURS_PER_DAY;

TimeTransport::TimeTransport(int h, int m) : hours(h), minutes(m) {
    if (minutes >= MINUTES_PER_HOUR) {
        hours += minutes / MINUTES_PER_HOUR;
        minutes = minutes % MINUTES_PER_HOUR;
    }
    if (hours >= HOURS_PER_DAY) {
        hours = hours % HOURS_PER_DAY;
    }
}

QString TimeTransport::toString() const {
    return QString("%1:%2")
    .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'));
}

TimeTransport TimeTransport::addMinutes(int newMinutes) const {
    int totalMinutes = toMinutes() + newMinutes;
    return TimeTransport(totalMinutes / MINUTES_PER_HOUR, totalMinutes % MINUTES_PER_HOUR);
}

int TimeTransport::toMinutes() const {
    return hours * MINUTES_PER_HOUR + minutes;
}
