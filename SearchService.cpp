#include "SearchService.h"
#include <algorithm>

QVector<Schedule> SearchService::findSchedulesByStop(const QVector<Schedule>& schedules, const QString& stopName)
{
    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        for (const auto& routeStop : route.getStops()) {
            if (routeStop.stop->getName().compare(stopName, Qt::CaseInsensitive) == 0) {
                result.push_back(schedule);
                break;
            }
        }
    }

    return result;
}

QVector<Schedule> SearchService::findSchedulesByDay(const QVector<Schedule>& schedules, const QString& day)
{
    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& days = schedule.getRoute().getDays();
        for (const QString& routeDay : days) {
            if (routeDay.compare(day, Qt::CaseInsensitive) == 0) {
                result.push_back(schedule);
                break;
            }
        }
    }

    return result;
}

QVector<Schedule> SearchService::findSchedulesByTransportType(const QVector<Schedule>& schedules, const QString& transportType)
{
    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        if (route.getTransport().getType().getName().compare(transportType, Qt::CaseInsensitive) == 0) {
            result.push_back(schedule);
        }
    }

    return result;
}

QVector<QSharedPointer<Stop>> SearchService::findStopsByName(const QVector<QSharedPointer<Stop>>& stops, const QString& searchTerm)
{
    QVector<QSharedPointer<Stop>> result;

    if (searchTerm.isEmpty()) {
        return stops;
    }

    for (const auto& stop : stops) {
        if (stop->getName().contains(searchTerm, Qt::CaseInsensitive)) {
            result.push_back(stop);
        }
    }

    return result;
}

QVector<Schedule> SearchService::filterSchedulesByTime(const QVector<Schedule>& schedules,
                                                       const TimeTransport& fromTime,
                                                       const TimeTransport& toTime)
{
    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& startTime = schedule.getStartTime();
        if (startTime >= fromTime && startTime <= toTime) {
            result.push_back(schedule);
        }
    }

    return result;
}

QVector<Schedule> SearchService::findRoutesBetweenStops(const QVector<Schedule>& schedules,
                                                        const QString& fromStop,
                                                        const QString& toStop)
{
    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        const auto& stops = route.getStops();

        bool foundFrom = false;
        bool foundTo = false;

        for (const auto& routeStop : stops) {
            if (routeStop.stop->getName().compare(fromStop, Qt::CaseInsensitive) == 0) {
                foundFrom = true;
            }
            if (foundFrom && routeStop.stop->getName().compare(toStop, Qt::CaseInsensitive) == 0) {
                foundTo = true;
                break;
            }
        }

        if (foundFrom && foundTo) {
            result.push_back(schedule);
        }
    }

    return result;
}
