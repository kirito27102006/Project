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
