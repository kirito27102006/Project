#ifndef STATISTICSSERVICE_H
#define STATISTICSSERVICE_H

#include "Schedule.h"
#include "Stop.h"
#include <QVector>
#include <QMap>

class StatisticsService
{
public:
    struct RouteStats {
        int totalRoutes;
        int busCount;
        int trolleybusCount;
        int tramCount;
        QMap<QString, int> routesByType;
        int averageStopsPerRoute;
        int maxStopsInRoute;
        int minStopsInRoute;
    };

    struct StopStats {
        int totalStops;
        int activeStops;
        QVector<QSharedPointer<Stop>> mostPopularStops;
        QMap<QString, int> stopUsageCount;
    };

    static RouteStats calculateRouteStatistics(const QVector<Schedule>& schedules);
    static StopStats calculateStopStatistics(const QVector<Schedule>& schedules,
                                             const QVector<QSharedPointer<Stop>>& allStops);
    static QMap<QString, int> calculateDailyScheduleCount(const QVector<Schedule>& schedules);
    static double calculateAverageStopsPerRoute(const QVector<Schedule>& schedules);
    static QMap<QString, int> calculateTransportTypeDistribution(const QVector<Schedule>& schedules);
};

#endif // STATISTICSSERVICE_H
