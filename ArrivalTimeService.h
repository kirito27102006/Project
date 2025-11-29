#ifndef ARRIVALTIMESERVICE_H
#define ARRIVALTIMESERVICE_H

#include "Route.h"
#include "TimeTransport.h"
#include <QVector>

class ArrivalTimeService
{
public:
    static TimeTransport calculateArrivalTime(const Route& route, int stopIndex, const TimeTransport& startTime);
    static QVector<TimeTransport> calculateAllArrivalTimes(const Route& route, const TimeTransport& startTime);
    static int calculateTravelTimeToStop(const Route& route, int stopIndex);
    static TimeTransport addTravelTime(const TimeTransport& startTime, int travelMinutes);
    static int calculateWaitTime(const TimeTransport& currentTime, const TimeTransport& arrivalTime);
    static QString formatWaitTime(int waitMinutes);
};

#endif // ARRIVALTIMESERVICE_H
