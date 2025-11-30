#include "StatisticsService.h"
#include <algorithm>
#include <ranges>
#include <QMap>
#include <climits>

StatisticsService::RouteStats StatisticsService::calculateRouteStatistics(const QVector<Schedule>& schedules)
{
    RouteStats stats;
    stats.totalRoutes = schedules.size();
    stats.busCount = 0;
    stats.trolleybusCount = 0;
    stats.tramCount = 0;
    stats.averageStopsPerRoute = 0;
    stats.maxStopsInRoute = 0;
    stats.minStopsInRoute = 0;

    if (schedules.isEmpty()) {
        return stats;
    }

    int totalStops = 0;
    stats.minStopsInRoute = INT_MAX;

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        QString transportType = route.getTransport().getType().getName();

        // Подсчет по типам транспорта
        if (transportType == "автобус") stats.busCount++;
        else if (transportType == "троллейбус") stats.trolleybusCount++;
        else if (transportType == "трамвай") stats.tramCount++;

        // Статистика по остановкам
        int stopCount = route.getStops().size();
        totalStops += stopCount;
        stats.maxStopsInRoute = std::max(stats.maxStopsInRoute, stopCount);
        stats.minStopsInRoute = std::min(stats.minStopsInRoute, stopCount);

        // Распределение по типам
        stats.routesByType[transportType]++;
    }

    stats.averageStopsPerRoute = totalStops / schedules.size();

    return stats;
}

StatisticsService::StopStats StatisticsService::calculateStopStatistics(const QVector<Schedule>& schedules,
                                                                        const QVector<QSharedPointer<Stop>>& allStops)
{
    StopStats stats;
    stats.totalStops = allStops.size();

    // Подсчет использования остановок
    QMap<QString, int> usageCount;
    QMap<QString, QSharedPointer<Stop>> stopMap;

    for (const auto& stop : allStops) {
        stopMap[stop->getName().toLower()] = stop;
    }

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        for (const auto& routeStop : route.getStops()) {
            QString stopName = routeStop.stop->getName().toLower();
            usageCount[stopName]++;
        }
    }

    stats.activeStops = usageCount.size();
    stats.stopUsageCount = usageCount;

    // Находим самые популярные остановки
    QVector<QPair<QString, int>> sortedStops;
    sortedStops.reserve(usageCount.size());
    for (auto it = usageCount.constBegin(); it != usageCount.constEnd(); ++it) {
        sortedStops.append(qMakePair(it.key(), it.value()));
    }

    std::ranges::sort(sortedStops,
                      [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                          return a.second > b.second;
                      });

    // Берем топ-10 остановок - ИСПРАВЛЕНА ОШИБКА
    int count = std::min(10, static_cast<int>(sortedStops.size()));
    for (int i = 0; i < count; ++i) {
        if (stopMap.contains(sortedStops[i].first)) {
            stats.mostPopularStops.append(stopMap[sortedStops[i].first]);
        }
    }

    return stats;
}

QMap<QString, int> StatisticsService::calculateDailyScheduleCount(const QVector<Schedule>& schedules)
{
    QMap<QString, int> dailyCount;

    QStringList days = {"пн", "вт", "ср", "чт", "пт", "сб", "вс"};
    for (const QString& day : days) {
        dailyCount[day] = 0;
    }

    for (const auto& schedule : schedules) {
        const auto& routeDays = schedule.getRoute().getDays();
        for (const QString& day : routeDays) {
            dailyCount[day]++;
        }
    }

    return dailyCount;
}

double StatisticsService::calculateAverageStopsPerRoute(const QVector<Schedule>& schedules)
{
    if (schedules.isEmpty()) return 0;

    int totalStops = 0;
    for (const auto& schedule : schedules) {
        totalStops += schedule.getRoute().getStops().size();
    }

    return static_cast<double>(totalStops) / schedules.size();
}

QMap<QString, int> StatisticsService::calculateTransportTypeDistribution(const QVector<Schedule>& schedules)
{
    QMap<QString, int> distribution;

    for (const auto& schedule : schedules) {
        QString type = schedule.getRoute().getTransport().getType().getName();
        distribution[type]++;
    }

    return distribution;
}
