#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include "Schedule.h"
#include "Stop.h"
#include <QString>
#include <QVector>

class SearchService
{
public:
    static QVector<Schedule> findSchedulesByStop(const QVector<Schedule>& schedules, const QString& stopName);
    static QVector<Schedule> findSchedulesByDay(const QVector<Schedule>& schedules, const QString& day);
    static QVector<Schedule> findSchedulesByTransportType(const QVector<Schedule>& schedules, const QString& transportType);
    static QVector<QSharedPointer<Stop>> findStopsByName(const QVector<QSharedPointer<Stop>>& stops, const QString& searchTerm);
    static QVector<Schedule> filterSchedulesByTime(const QVector<Schedule>& schedules,
                                                   const TimeTransport& fromTime,
                                                   const TimeTransport& toTime);
    static QVector<Schedule> findRoutesBetweenStops(const QVector<Schedule>& schedules,
                                                    const QString& fromStop,
                                                    const QString& toStop);
};

#endif // SEARCHSERVICE_H
