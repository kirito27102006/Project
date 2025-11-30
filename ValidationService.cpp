#include "ValidationService.h"
#include "DayOfWeekService.h"
#include <QRegularExpression>
#include <ranges>

ValidationService::ValidationResult ValidationService::validateRouteData(int routeNumber,
                                                                         const QVector<QSharedPointer<Stop>>& stops,
                                                                         const QVector<int>& travelTimes,
                                                                         const QStringList& days)
{
    if (routeNumber <= 0 || routeNumber > 999) {
        return ValidationResult(false, "Номер маршрута должен быть в диапазоне 1-999");
    }

    if (stops.size() < 2) {
        return ValidationResult(false, "Маршрут должен содержать хотя бы 2 остановки");
    }

    if (travelTimes.size() != stops.size() - 1) {
        return ValidationResult(false,
                                QString("Количество временных интервалов (%1) должно соответствовать количеству перегонов (%2)")
                                    .arg(travelTimes.size())
                                    .arg(stops.size() - 1));
    }

    for (int i = 0; i < travelTimes.size(); ++i) {
        if (travelTimes[i] <= 0 || travelTimes[i] > 120) {
            return ValidationResult(false,
                                    QString("Время движения между остановками должно быть от 1 до 120 минут (сегмент %1)").arg(i + 1));
        }
    }

    if (auto daysValidation = validateDays(days); !daysValidation.isValid) {
        return daysValidation;
    }

    return ValidationResult(true);
}

ValidationService::ValidationResult ValidationService::validateStopData(const QString& stopName, const QString& coordinate)
{
    if (stopName.trimmed().isEmpty()) {
        return ValidationResult(false, "Название остановки не может быть пустым");
    }

    if (stopName.length() > 100) {
        return ValidationResult(false, "Название остановки слишком длинное");
    }

    if (!coordinate.isEmpty() && coordinate.length() > 50) {
        return ValidationResult(false, "Координаты слишком длинные");
    }

    return ValidationResult(true);
}

ValidationService::ValidationResult ValidationService::validateTimeData(int hours, int minutes)
{
    if (hours < 0 || hours > 23) {
        return ValidationResult(false, "Часы должны быть в диапазоне 0-23");
    }

    if (minutes < 0 || minutes > 59) {
        return ValidationResult(false, "Минуты должны быть в диапазоне 0-59");
    }

    return ValidationResult(true);
}

ValidationService::ValidationResult ValidationService::validateTravelTimes(const QVector<int>& travelTimes, int expectedCount)
{
    if (travelTimes.size() != expectedCount) {
        return ValidationResult(false,
                                QString("Ожидается %1 временных интервалов, получено %2")
                                    .arg(expectedCount)
                                    .arg(travelTimes.size()));
    }

    for (int time : travelTimes) {
        if (time <= 0 || time > 120) {
            return ValidationResult(false, "Время движения должно быть от 1 до 120 минут");
        }
    }

    return ValidationResult(true);
}

bool ValidationService::isRouteNumberUnique(int routeNumber, const QVector<Route>& existingRoutes)
{
    return !std::ranges::any_of(existingRoutes,
                                 [routeNumber](const Route& route) {
                                     return route.getRouteNumber() == routeNumber;
                                 });
}

ValidationService::ValidationResult ValidationService::validateTransportType(const QString& transportType)
{
    if (const QStringList validTypes = {"автобус", "троллейбус", "трамвай"}; !validTypes.contains(transportType.toLower())) {
        return ValidationResult(false, "Неверный тип транспорта. Допустимые значения: автобус, троллейбус, трамвай");
    }

    return ValidationResult(true);
}

ValidationService::ValidationResult ValidationService::validateDays(const QStringList& days)
{
    if (days.isEmpty()) {
        return ValidationResult(false, "Укажите дни работы маршрута");
    }

    QStringList validDays = DayOfWeekService::getAllDays();
    for (const QString& day : days) {
        if (!validDays.contains(day.toLower())) {
            return ValidationResult(false,
                                    QString("Неверный день недели: '%1'. Допустимые значения: %2")
                                        .arg(day)
                                        .arg(validDays.join(", ")));
        }
    }

    return ValidationResult(true);
}
