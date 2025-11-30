#ifndef DAYOFWEEKSERVICE_H
#define DAYOFWEEKSERVICE_H

#include <QString>
#include <QStringList>
#include <QDate>

class DayOfWeekService
{
public:
    // УДАЛЕНЫ неиспользуемые константы - они не используются в коде
    // static constexpr const char* MONDAY = "пн";
    // static constexpr const char* TUESDAY = "вт";
    // static constexpr const char* WEDNESDAY = "ср";
    // static constexpr const char* THURSDAY = "чт";
    // static constexpr const char* FRIDAY = "пт";
    // static constexpr const char* SATURDAY = "сб";
    // static constexpr const char* SUNDAY = "вс";

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
