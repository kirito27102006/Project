#include "Schedule.h"

Schedule::Schedule(const Route& route, const TimeTransport& startTime)
    : route(route), startTime(startTime) {
    calculateRouteTimes();
}

Route Schedule::getRoute() const {
    return route;
}

TimeTransport Schedule::getStartTime() const {
    return startTime;
}

void Schedule::setStartTime(const TimeTransport& time) {
    startTime = time;
    calculateRouteTimes();
}

void Schedule::calculateRouteTimes() {
    route.calculateArrivalTimes(startTime);
}
