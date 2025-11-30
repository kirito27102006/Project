#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include <QString>
#include <QSharedPointer>
#include <stdexcept>
#include "Transport.h"
#include "Stop.h"
#include "TimeTransport.h"

// Исключения для маршрутов
class RouteException : public std::runtime_error {
public:
    explicit RouteException(const QString& message)
        : std::runtime_error(message.toStdString()) {}
};

class StopNotFoundException : public RouteException {
public:
    explicit StopNotFoundException(const QString& stopName)
        : RouteException(QString("Остановка '%1' не найдена в маршруте").arg(stopName)) {}
};

class InvalidRouteConfigurationException : public RouteException {
public:
    explicit InvalidRouteConfigurationException(const QString& message)
        : RouteException(QString("Неверная конфигурация маршрута: %1").arg(message)) {}
};

class RouteStop {
public:
    QSharedPointer<Stop> stop;
    TimeTransport arrivalTime;

    explicit RouteStop(QSharedPointer<Stop> stop, const TimeTransport& time);
    QString getName() const;
};

class Route {
public:
    explicit Route(const Transport& transport, QSharedPointer<Stop> start, QSharedPointer<Stop> end);

    void addStop(QSharedPointer<Stop> stop, int travelTimeFromPrevious);
    void addFinalTravelTime(int travelTime);
    void calculateArrivalTimes(const TimeTransport& startTime);
    TimeTransport getArrivalTimeAtStop(const QString& stopName) const;
    QVector<RouteStop> getStops() const;
    Transport getTransport() const;
    QSharedPointer<Stop> getStartStop() const;
    QSharedPointer<Stop> getEndStop() const;
    QStringList getDays() const;
    void setDays(const QStringList& days);
    QVector<int> getTravelTimes() const;
    int getRouteNumber() const;

private:
    Transport transport;
    QSharedPointer<Stop> startStop;
    QSharedPointer<Stop> endStop;
    QVector<RouteStop> stops;
    QVector<int> travelTimes;
    QStringList days;
};

#endif
