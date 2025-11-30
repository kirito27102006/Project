#include "TimeTransport.h"
#include <QString>

TimeTransport::TimeTransport(int h, int m) : hours(h), minutes(m) {
    if (minutes >= 60) {
        hours += minutes / 60;
        minutes = minutes % 60;
    }
    if (hours >= 24) {
        hours = hours % 24;
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
    return TimeTransport(totalMinutes / 60, totalMinutes % 60);
}

int TimeTransport::toMinutes() const {
    return hours * 60 + minutes;
}

QDataStream& operator<<(QDataStream& out, const TimeTransport& time) {
    out << time.hours << time.minutes;
    return out;
}

QDataStream& operator>>(QDataStream& in, TimeTransport& time) {
    in >> time.hours >> time.minutes;
    if (time.minutes >= 60) {
        time.hours += time.minutes / 60;
        time.minutes = time.minutes % 60;
    }
    if (time.hours >= 24) {
        time.hours = time.hours % 24;
    }
    return in;
}
