#ifndef VALIDATIONSERVICE_H
#define VALIDATIONSERVICE_H

#include <QString>
#include <QVector>
#include <QSharedPointer>
#include "Route.h"
#include "Stop.h"

class ValidationService
{
public:
    // Константы для валидации
    static constexpr int MIN_ROUTE_NUMBER = 1;
    static constexpr int MAX_ROUTE_NUMBER = 999;
    static constexpr int MIN_TRAVEL_TIME = 1;
    static constexpr int MAX_TRAVEL_TIME = 120;
    static constexpr int MAX_STOP_NAME_LENGTH = 100;
    static constexpr int MAX_COORDINATE_LENGTH = 50;

    struct ValidationResult {
        bool isValid;
        QString errorMessage;

        explicit ValidationResult(bool valid = true, const QString& message = "")
            : isValid(valid), errorMessage(message) {}
    };

    static ValidationResult validateRouteData(int routeNumber,
                                              const QVector<QSharedPointer<Stop>>& stops,
                                              const QVector<int>& travelTimes,
                                              const QStringList& days);

    static ValidationResult validateStopData(const QString& stopName, const QString& coordinate);
    static ValidationResult validateTimeData(int hours, int minutes);
    static ValidationResult validateTravelTimes(const QVector<int>& travelTimes, int expectedCount);
    static bool isRouteNumberUnique(int routeNumber, const QVector<Route>& existingRoutes);
    static ValidationResult validateTransportType(const QString& transportType);
    static ValidationResult validateDays(const QStringList& days);
};

#endif // VALIDATIONSERVICE_H
