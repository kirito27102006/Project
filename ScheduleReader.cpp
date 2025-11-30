#include "ScheduleReader.h"

ScheduleReader::ScheduleReader(QObject *parent) : QObject(parent) {}

std::pair<int, int> ScheduleReader::readStartTime(const QString& timeLine) const
{
    QStringList timeParts = timeLine.split(" ");
    if (timeParts.size() < 2)
        throw TimeFormatException("Invalid time format in schedule");

    bool ok1;
    bool ok2;
    int hour = timeParts[0].toInt(&ok1);
    int minute = timeParts[1].toInt(&ok2);

    if (!ok1 || !ok2)
        throw TimeFormatException("Invalid time values");

    return {hour, minute};
}

QStringList ScheduleReader::readDays(QTextStream& in) const
{
    QStringList days;
    QString line = in.readLine();

    if (!line.startsWith("DAYS:"))
        return days;

    int dayCount = line.mid(5).toInt();
    for (int i = 0; i < dayCount; ++i) {
        QString d = in.readLine();
        if (d.isNull())
            throw FileFormatException("Unexpected end of file while reading days");
        days.push_back(d);
    }

    return days;
}

QVector<int> ScheduleReader::readTravelTimes(QTextStream& in, int& timeCount) const
{
    QVector<int> times;

    QString line = in.readLine();
    if (!line.startsWith("TRAVEL_TIMES:"))
        return times;

    timeCount = line.mid(13).toInt();

    for (int i = 0; i < timeCount; ++i) {
        QString ts = in.readLine();
        if (ts.isNull())
            throw FileFormatException("Unexpected end of file while reading travel time");

        bool ok;
        int t = ts.toInt(&ok);
        if (!ok)
            throw FileFormatException("Invalid travel time value");

        times.push_back(t);
    }

    return times;
}

void ScheduleReader::addIntermediateStops(Route& route,
                                          const QVector<QSharedPointer<Stop>>& routeStops,
                                          const QVector<int>& travelTimes) const
{
    const int totalStops = routeStops.size();
    const int firstIntermediateIndex = 1;
    const int lastIntermediateIndex = totalStops - 2; // -2 потому что последняя остановка уже добавлена

    for (int stopIndex = firstIntermediateIndex; stopIndex <= lastIntermediateIndex; ++stopIndex) {
        const int timeIndex = stopIndex - firstIntermediateIndex;
        const int travelTime = (timeIndex < travelTimes.size()) ? travelTimes[timeIndex] : 5;

        route.addStop(routeStops[stopIndex], travelTime);
    }
}
