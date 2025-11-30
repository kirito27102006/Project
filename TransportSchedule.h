#ifndef TRANSPORTSCHEDULE_H
#define TRANSPORTSCHEDULE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSharedPointer>
#include "Route.h"
#include "Schedule.h"
#include "ScheduleReader.h"
#include "ScheduleWriter.h"
#include "ArrivalTimeService.h"
#include "DayOfWeekService.h"
#include "ValidationService.h"
#include "SearchService.h"
#include "StatisticsService.h"

class TransportSchedule : public QObject
{
    Q_OBJECT

private:
    QVector<Schedule> schedules;
    QVector<QSharedPointer<Stop>> allStops;
    QString filename;
    mutable QVector<QSharedPointer<Stop>> activeStops;
    mutable bool stopsDirty;

    // Сервисы
    ScheduleReader* scheduleReader;
    ScheduleWriter* scheduleWriter;

public:
    explicit TransportSchedule(const QString& file, QObject* parent = nullptr);
    ~TransportSchedule() override = default;

    void addRoute(const Transport& transport, QSharedPointer<Stop> startStop,
                  QSharedPointer<Stop> endStop, const QVector<QSharedPointer<Stop>>& intermediateStops,
                  const QVector<int>& travelTimes, const QStringList& days, const TimeTransport& startTime);
    void removeRoute(int routeNumber);
    void updateRoute(int oldRouteNumber, const Transport& transport,
                     QSharedPointer<Stop> startStop, QSharedPointer<Stop> endStop,
                     const QVector<QSharedPointer<Stop>>& intermediateStops,
                     const QVector<int>& travelTimes, const QStringList& days,
                     const TimeTransport& startTime);
    QVector<Schedule> getSchedulesForDay(const QString& day) const;
    void updateRoute(int oldRouteNumber, const Route& newRoute, const TimeTransport& startTime);
    QVector<Schedule> getSchedulesForStop(const QString& stopName) const;
    QVector<Schedule> findNextTransport(const QString& stopName) const;
    void saveToFile();
    void loadFromFile();
    QVector<QSharedPointer<Stop>> getAllStops() const;
    QVector<Schedule> getAllSchedules() const;
    QStringList getAllRouteNumbers() const;

    TimeTransport getCurrentTime() const;
    QString getCurrentDayOfWeek() const;

    QSharedPointer<Stop> findOrCreateStop(const QString& name, const QString& coordinate = "");
    QVector<QSharedPointer<Stop>> getActiveStops() const;

    // Новые методы с использованием сервисов
    StatisticsService::RouteStats getRouteStatistics() const;
    StatisticsService::StopStats getStopStatistics() const;
    QMap<QString, int> getDailyScheduleCount() const;

private:
    void updateActiveStops() const;
};

#endif
