#include "ArrivalTimeService.h"
#include <QString>

TimeTransport ArrivalTimeService::calculateArrivalTime(const Route& route, int stopIndex, const TimeTransport& startTime)
{
    if (stopIndex == 0) {
        return startTime;
    }

    auto travelTimes = route.getTravelTimes();
    TimeTransport currentTime = startTime;

    for (int i = 0; i < stopIndex && i < travelTimes.size(); ++i) {
        currentTime = currentTime.addMinutes(travelTimes[i]);
    }

    return currentTime;
}

QVector<TimeTransport> ArrivalTimeService::calculateAllArrivalTimes(const Route& route, const TimeTransport& startTime)
{
    QVector<TimeTransport> arrivalTimes;
    auto stops = route.getStops();

    for (int i = 0; i < stops.size(); ++i) {
        arrivalTimes.append(calculateArrivalTime(route, i, startTime));
    }

    return arrivalTimes;
}

int ArrivalTimeService::calculateTravelTimeToStop(const Route& route, int stopIndex)
{
    if (stopIndex <= 0) return 0;

    auto travelTimes = route.getTravelTimes();
    int totalTime = 0;

    for (int i = 0; i < stopIndex && i < travelTimes.size(); ++i) {
        totalTime += travelTimes[i];
    }

    return totalTime;
}

TimeTransport ArrivalTimeService::addTravelTime(const TimeTransport& startTime, int travelMinutes)
{
    return startTime.addMinutes(travelMinutes);
}

int ArrivalTimeService::calculateWaitTime(const TimeTransport& currentTime, const TimeTransport& arrivalTime)
{
    int currentMinutes = currentTime.toMinutes();
    int arrivalMinutes = arrivalTime.toMinutes();
    int waitMinutes = arrivalMinutes - currentMinutes;

    if (waitMinutes < 0) {
        waitMinutes += 24 * 60;
    }

    return waitMinutes;
}

QString ArrivalTimeService::formatWaitTime(int waitMinutes)
{
    if (waitMinutes >= 60) {
        return QString("%1 ч %2 мин").arg(waitMinutes / 60).arg(waitMinutes % 60);
    } else {
        return QString("%1 мин").arg(waitMinutes);
    }
}
