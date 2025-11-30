#ifndef TRANSPORTSCHEDULE_H
#define TRANSPORTSCHEDULE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSharedPointer>
#include <stdexcept>
#include "Route.h"
#include "Schedule.h"
#include "ScheduleReader.h"
#include "ScheduleWriter.h"
#include "ArrivalTimeService.h"
#include "DayOfWeekService.h"
#include "ValidationService.h"
#include "SearchService.h"
#include "StatisticsService.h"

// Пользовательские исключения для проекта
class TransportScheduleException : public std::runtime_error {
public:
    explicit TransportScheduleException(const QString& message)
        : std::runtime_error(message.toStdString()) {}
};

class RouteNotFoundException : public TransportScheduleException {
public:
    explicit RouteNotFoundException(int routeNumber)
        : TransportScheduleException(QString("Маршрут №%1 не найден").arg(routeNumber)) {}
};

class InvalidRouteDataException : public TransportScheduleException {
public:
    explicit InvalidRouteDataException(const QString& message)
        : TransportScheduleException(QString("Неверные данные маршрута: %1").arg(message)) {}
};

class FileOperationException : public TransportScheduleException {
public:
    explicit FileOperationException(const QString& message)
        : TransportScheduleException(QString("Ошибка работы с файлом: %1").arg(message)) {}
};

class TransportSchedule : public QObject
{
    Q_OBJECT

public:
    // Структура для группировки параметров маршрута
    struct RouteParams {
        Transport transport;
        QSharedPointer<Stop> startStop;
        QSharedPointer<Stop> endStop;
        QVector<QSharedPointer<Stop>> intermediateStops;
        QVector<int> travelTimes;
        QStringList days;
        TimeTransport startTime;

        RouteParams(const Transport& transport,
                    QSharedPointer<Stop> startStop,
                    QSharedPointer<Stop> endStop,
                    const QVector<QSharedPointer<Stop>>& intermediateStops,
                    const QVector<int>& travelTimes,
                    const QStringList& days,
                    const TimeTransport& startTime)
            : transport(transport), startStop(std::move(startStop)), endStop(std::move(endStop)),
            intermediateStops(intermediateStops), travelTimes(travelTimes),
            days(days), startTime(startTime) {}
    };

private:
    QVector<Schedule> schedules;
    QVector<QSharedPointer<Stop>> allStops;
    QString filename;
    mutable QVector<QSharedPointer<Stop>> activeStops;
    mutable bool stopsDirty = true;

    // Сервисы
    ScheduleReader* scheduleReader;
    ScheduleWriter* scheduleWriter;

public:
    explicit TransportSchedule(const QString& file, QObject* parent = nullptr);
    ~TransportSchedule() override = default;

    // Основные методы с использованием RouteParams
    void addRoute(const RouteParams& params);
    void removeRoute(int routeNumber);
    void updateRoute(int oldRouteNumber, const RouteParams& params);

    // Остальные методы без изменений
    void updateRoute(int oldRouteNumber, const Route& newRoute, const TimeTransport& startTime);
    QVector<Schedule> getSchedulesForDay(const QString& day) const;
    QVector<Schedule> getSchedulesForStop(const QString& stopName) const;
    QVector<Schedule> findNextTransport(const QString& stopName) const;
    void saveToFile() const;
    void loadFromFile();
    QVector<QSharedPointer<Stop>> getAllStops() const;
    QVector<Schedule> getAllSchedules() const;
    QStringList getAllRouteNumbers() const;

    TimeTransport getCurrentTime() const;
    QString getCurrentDayOfWeek() const;

    QSharedPointer<Stop> findOrCreateStop(const QString& name, const QString& coordinate = "");
    QVector<QSharedPointer<Stop>> getActiveStops() const;

    StatisticsService::RouteStats getRouteStatistics() const;
    StatisticsService::StopStats getStopStatistics() const;
    QMap<QString, int> getDailyScheduleCount() const;

private:
    void updateActiveStops() const;
    Route createRouteFromParams(const RouteParams& params) const;
};

#endif
