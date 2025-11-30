#include "ArrivalTimeService.h"
#include <QString>
#include <algorithm>

// Определение констант
constexpr int ArrivalTimeService::MINUTES_IN_HOUR;
constexpr int ArrivalTimeService::HOURS_IN_DAY;
constexpr int ArrivalTimeService::MINUTES_IN_DAY;

TimeTransport ArrivalTimeService::calculateArrivalTime(const Route& route, int stopIndex, const TimeTransport& startTime)
{
    if (stopIndex == 0) {
        return startTime;
    }

    auto travelTimes = route.getTravelTimes();
    TimeTransport currentTime = startTime;

    const auto maxIndex = std::min(stopIndex, static_cast<int>(travelTimes.size()));
    for (int i = 0; i < maxIndex; ++i) {
        currentTime = currentTime.addMinutes(travelTimes[i]);
    }

    return currentTime;
}

QVector<TimeTransport> ArrivalTimeService::calculateAllArrivalTimes(const Route& route, const TimeTransport& startTime)
{
    QVector<TimeTransport> arrivalTimes;
    auto stops = route.getStops();

    arrivalTimes.reserve(stops.size());
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

    const auto maxIndex = std::min(stopIndex, static_cast<int>(travelTimes.size()));
    for (int i = 0; i < maxIndex; ++i) {
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
        waitMinutes += MINUTES_IN_DAY;
    }

    return waitMinutes;
}

QString ArrivalTimeService::formatWaitTime(int waitMinutes)
{
    if (waitMinutes >= MINUTES_IN_HOUR) {
        return QString("%1 ч %2 мин")
            .arg(waitMinutes / MINUTES_IN_HOUR)
            .arg(waitMinutes % MINUTES_IN_HOUR);
    } else {
        return QString("%1 мин").arg(waitMinutes);
    }
}
