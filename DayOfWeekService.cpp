#include "DayOfWeekService.h"
#include <QMap>
#include <algorithm>

// УДАЛЕНЫ неиспользуемые определения констант

QString DayOfWeekService::getCurrentDay()
{
    int dayOfWeek = QDate::currentDate().dayOfWeek();
    return getDayName(dayOfWeek);
}

QStringList DayOfWeekService::getAllDays()
{
    return {"пн", "вт", "ср", "чт", "пт", "сб", "вс"};
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
        return "пн";
    else if (lowerDay == "вторник" || lowerDay == "втор" || lowerDay == "tue" || lowerDay == "tuesday")
        return "вт";
    else if (lowerDay == "среда" || lowerDay == "сред" || lowerDay == "wed" || lowerDay == "wednesday")
        return "ср";
    else if (lowerDay == "четверг" || lowerDay == "четв" || lowerDay == "thu" || lowerDay == "thursday")
        return "чт";
    else if (lowerDay == "пятница" || lowerDay == "пятн" || lowerDay == "fri" || lowerDay == "friday")
        return "пт";
    else if (lowerDay == "суббота" || lowerDay == "субб" || lowerDay == "sat" || lowerDay == "saturday")
        return "сб";
    else if (lowerDay == "воскресенье" || lowerDay == "воскр" || lowerDay == "sun" || lowerDay == "sunday")
        return "вс";
    else
        return day.left(2).toLower();
}

QString DayOfWeekService::getDayName(int dayOfWeek)
{
    switch(dayOfWeek) {
    case 1: return "пн";
    case 2: return "вт";
    case 3: return "ср";
    case 4: return "чт";
    case 5: return "пт";
    case 6: return "сб";
    case 7: return "вс";
    default: return "пн";
    }
}
