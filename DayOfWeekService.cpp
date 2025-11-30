#include "DayOfWeekService.h"
#include <QMap>
#include <algorithm>

// Определение констант
constexpr const char* DayOfWeekService::MONDAY;
constexpr const char* DayOfWeekService::TUESDAY;
constexpr const char* DayOfWeekService::WEDNESDAY;
constexpr const char* DayOfWeekService::THURSDAY;
constexpr const char* DayOfWeekService::FRIDAY;
constexpr const char* DayOfWeekService::SATURDAY;
constexpr const char* DayOfWeekService::SUNDAY;

QString DayOfWeekService::getCurrentDay()
{
    int dayOfWeek = QDate::currentDate().dayOfWeek();
    return getDayName(dayOfWeek);
}

QStringList DayOfWeekService::getAllDays()
{
    return { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY };
}

QStringList DayOfWeekService::parseDaysString(const QString& daysString)
{
    QStringList days;
    if (daysString.isEmpty()) return days;

    auto normalized = daysString;
    normalized = normalized.replace(';', ',');
    normalized = normalized.replace(' ', ',');
    days = normalized.split(',', Qt::SkipEmptyParts);

    for (auto &day : days) {
        day = translateDay(day.trimmed());
    }

    return days;
}

QString DayOfWeekService::formatDays(const QStringList& days)
{
    return days.join(", ");
}

bool DayOfWeekService::isRouteActiveToday(const QStringList& routeDays)
{
    QString currentDay = getCurrentDay();
    return isDayInList(currentDay, routeDays);
}

bool DayOfWeekService::isDayInList(const QString& day, const QStringList& daysList)
{
    QString normalizedDay = translateDay(day.toLower());

    auto it = std::ranges::find_if(daysList,
                                   [&normalizedDay](const QString& routeDay) {
                                       return routeDay.toLower() == normalizedDay;
                                   });

    return it != daysList.end();
}

QString DayOfWeekService::translateDay(const QString& day)
{
    auto lowerDay = day.toLower();

    if (lowerDay == "понедельник" || lowerDay == "понед" || lowerDay == "mon" || lowerDay == "monday")
        return MONDAY;
    else if (lowerDay == "вторник" || lowerDay == "втор" || lowerDay == "tue" || lowerDay == "tuesday")
        return TUESDAY;
    else if (lowerDay == "среда" || lowerDay == "сред" || lowerDay == "wed" || lowerDay == "wednesday")
        return WEDNESDAY;
    else if (lowerDay == "четверг" || lowerDay == "четв" || lowerDay == "thu" || lowerDay == "thursday")
        return THURSDAY;
    else if (lowerDay == "пятница" || lowerDay == "пятн" || lowerDay == "fri" || lowerDay == "friday")
        return FRIDAY;
    else if (lowerDay == "суббота" || lowerDay == "субб" || lowerDay == "sat" || lowerDay == "saturday")
        return SATURDAY;
    else if (lowerDay == "воскресенье" || lowerDay == "воскр" || lowerDay == "sun" || lowerDay == "sunday")
        return SUNDAY;
    else
        return day.left(2).toLower();
}

QString DayOfWeekService::getDayName(int dayOfWeek)
{
    switch(dayOfWeek) {
    case 1: return MONDAY;
    case 2: return TUESDAY;
    case 3: return WEDNESDAY;
    case 4: return THURSDAY;
    case 5: return FRIDAY;
    case 6: return SATURDAY;
    case 7: return SUNDAY;
    default: return MONDAY;
    }
}
