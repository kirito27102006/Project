#ifndef TIMETRANSPORT_H
#define TIMETRANSPORT_H

#include <QString>
#include <QDataStream>
#include <compare>

class TimeTransport {
public:
    // Константы для работы со временем
    static constexpr int MIN_HOURS = 0;
    static constexpr int MAX_HOURS = 23;
    static constexpr int MIN_MINUTES = 0;
    static constexpr int MAX_MINUTES = 59;
    static constexpr int MINUTES_PER_HOUR = 60;
    static constexpr int HOURS_PER_DAY = 24;

    int hours;
    int minutes;

    explicit TimeTransport(int h = 0, int m = 0);
    QString toString() const;
    auto operator<=>(const TimeTransport& other) const = default;

    TimeTransport addMinutes(int minutes) const;
    int toMinutes() const;

    friend QDataStream& operator<<(QDataStream& out, const TimeTransport& time) {
        out << time.hours << time.minutes;
        return out;
    }

    friend QDataStream& operator>>(QDataStream& in, TimeTransport& time) {
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
};

#endif
