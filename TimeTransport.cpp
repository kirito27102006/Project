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

bool TimeTransport::operator==(const TimeTransport& other) const {
    return hours == other.hours && minutes == other.minutes;
}

TimeTransport TimeTransport::addMinutes(int minutes) const {
    int totalMinutes = toMinutes() + minutes;
    return TimeTransport(totalMinutes / MINUTES_PER_HOUR, totalMinutes % MINUTES_PER_HOUR);
}

int TimeTransport::toMinutes() const {
    return hours * MINUTES_PER_HOUR + minutes;
}

QDataStream& operator<<(QDataStream& out, const TimeTransport& time) {
    out << time.hours << time.minutes;
    return out;
}

QDataStream& operator>>(QDataStream& in, TimeTransport& time) {
    in >> time.hours >> time.minutes;
    if (time.minutes >= TimeTransport::MINUTES_PER_HOUR) {
        time.hours += time.minutes / TimeTransport::MINUTES_PER_HOUR;
        time.minutes = time.minutes % TimeTransport::MINUTES_PER_HOUR;
    }
    if (time.hours >= TimeTransport::HOURS_PER_DAY) {
        time.hours = time.hours % TimeTransport::HOURS_PER_DAY;
    }
    return in;
}
