#ifndef TIMETRANSPORT_H
#define TIMETRANSPORT_H

#include <QString>
#include <QDataStream>

class TimeTransport {
public:
    int hours;
    int minutes;

    TimeTransport(int h = 0, int m = 0);
    QString toString() const;
    bool operator<(const TimeTransport& other) const;
    bool operator>(const TimeTransport& other) const;
    bool operator==(const TimeTransport& other) const;
    bool operator<=(const TimeTransport& other) const;
    bool operator>=(const TimeTransport& other) const;

    TimeTransport addMinutes(int minutes) const;
    int toMinutes() const;

    friend QDataStream& operator<<(QDataStream& out, const TimeTransport& time);
    friend QDataStream& operator>>(QDataStream& in, TimeTransport& time);
};

#endif
