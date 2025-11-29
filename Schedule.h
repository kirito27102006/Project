#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "Route.h"
#include "TimeTransport.h"

class Schedule {
public:
    Schedule(const Route& route, const TimeTransport& startTime);

    Route getRoute() const;
    TimeTransport getStartTime() const;
    void setStartTime(const TimeTransport& time);
    void calculateRouteTimes();

private:
    Route route;
    TimeTransport startTime;
};

#endif
