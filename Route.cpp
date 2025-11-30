#include "Route.h"
#include <QDebug>

RouteStop::RouteStop(QSharedPointer<Stop> stop, const TimeTransport& time)
    : stop(stop), arrivalTime(time) {}

QString RouteStop::getName() const {
    return stop->getName();
}

Route::Route(const Transport& transport, QSharedPointer<Stop> start, QSharedPointer<Stop> end)
    : transport(transport), startStop(start), endStop(end) {
    stops.push_back(RouteStop(start, TimeTransport(0, 0)));
}

void Route::addStop(QSharedPointer<Stop> stop, int travelTimeFromPrevious) {
    if (!stop) {
        throw InvalidRouteConfigurationException("Попытка добавить пустую остановку");
    }

    stops.push_back(RouteStop(stop, TimeTransport(0, 0)));
    travelTimes.push_back(travelTimeFromPrevious);
}

void Route::addFinalTravelTime(int travelTime) {
    stops.push_back(RouteStop(endStop, TimeTransport(0, 0)));
    travelTimes.push_back(travelTime);
}

void Route::calculateArrivalTimes(const TimeTransport& startTime) {
    if (stops.empty() || travelTimes.empty()) {
        throw InvalidRouteConfigurationException("Недостаточно данных для расчета времени прибытия");
    }

    // Начальная остановка - время отправления
    stops[0].arrivalTime = startTime;

    TimeTransport currentTime = startTime;

    // Рассчитываем время прибытия для каждой следующей остановки
    for (int i = 1; i < stops.size(); i++) {
        if (i - 1 < travelTimes.size()) {
            currentTime = currentTime.addMinutes(travelTimes[i - 1]);
            stops[i].arrivalTime = currentTime;
        }
    }

    // Отладочный вывод
    qDebug() << "Маршрут №" << getRouteNumber() << "расписание остановок:";
    for (const auto& stop : stops) {
        qDebug() << "  " << stop.getName() << "->" << stop.arrivalTime.toString();
    }
}

TimeTransport Route::getArrivalTimeAtStop(const QString& stopName) const {
    for (const auto& routeStop : stops) {
        if (routeStop.stop->getName().compare(stopName, Qt::CaseInsensitive) == 0) {
            return routeStop.arrivalTime;
        }
    }
    throw StopNotFoundException(stopName);
}

QVector<RouteStop> Route::getStops() const {
    return stops;
}

Transport Route::getTransport() const {
    return transport;
}

QSharedPointer<Stop> Route::getStartStop() const {
    return startStop;
}

QSharedPointer<Stop> Route::getEndStop() const {
    return endStop;
}

QStringList Route::getDays() const {
    return days;
}

void Route::setDays(const QStringList& newDays) {
    days = newDays;
}

QVector<int> Route::getTravelTimes() const {
    return travelTimes;
}

int Route::getRouteNumber() const {
    return transport.getId();
}
