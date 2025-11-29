#ifndef DAYOFWEEKSERVICE_H
#define DAYOFWEEKSERVICE_H

#include <QString>
#include <QStringList>
#include <QDate>

class DayOfWeekService
{
public:
    static QString getCurrentDay();
    static QStringList getAllDays();
    static QStringList parseDaysString(const QString& daysString);
    static QString formatDays(const QStringList& days);
    static bool isRouteActiveToday(const QStringList& routeDays);
    static bool isDayInList(const QString& day, const QStringList& daysList);
    static QString translateDay(const QString& day);
    static QString getDayName(int dayOfWeek);
};

#endif // DAYOFWEEKSERVICE_H
