#include "ValidationService.h"
#include "DayOfWeekService.h"
#include <QRegularExpression>
#include <ranges>

// Определение констант
constexpr int ValidationService::MIN_ROUTE_NUMBER;
constexpr int ValidationService::MAX_ROUTE_NUMBER;
constexpr int ValidationService::MIN_TRAVEL_TIME;
constexpr int ValidationService::MAX_TRAVEL_TIME;
constexpr int ValidationService::MAX_STOP_NAME_LENGTH;
constexpr int ValidationService::MAX_COORDINATE_LENGTH;

ValidationService::ValidationResult ValidationService::validateRouteData(int routeNumber,
                                                                         const QVector<QSharedPointer<Stop>>& stops,
                                                                         const QVector<int>& travelTimes,
                                                                         const QStringList& days)
{
    if (routeNumber < MIN_ROUTE_NUMBER || routeNumber > MAX_ROUTE_NUMBER) {
        return ValidationResult(false,
                                QString("Номер маршрута должен быть в диапазоне %1-%2")
                                    .arg(MIN_ROUTE_NUMBER)
                                    .arg(MAX_ROUTE_NUMBER));
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
        if (travelTimes[i] < MIN_TRAVEL_TIME || travelTimes[i] > MAX_TRAVEL_TIME) {
            return ValidationResult(false,
                                    QString("Время движения между остановками должно быть от %1 до %2 минут (сегмент %3)")
                                        .arg(MIN_TRAVEL_TIME)
                                        .arg(MAX_TRAVEL_TIME)
                                        .arg(i + 1));
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

    if (stopName.length() > MAX_STOP_NAME_LENGTH) {
        return ValidationResult(false,
                                QString("Название остановки слишком длинное (максимум %1 символов)")
                                    .arg(MAX_STOP_NAME_LENGTH));
    }

    if (!coordinate.isEmpty() && coordinate.length() > MAX_COORDINATE_LENGTH) {
        return ValidationResult(false,
                                QString("Координаты слишком длинные (максимум %1 символов)")
                                    .arg(MAX_COORDINATE_LENGTH));
    }

    return ValidationResult(true);
}

ValidationService::ValidationResult ValidationService::validateTimeData(int hours, int minutes)
{
    constexpr int MIN_HOURS = 0;
    constexpr int MAX_HOURS = 23;
    constexpr int MIN_MINUTES = 0;
    constexpr int MAX_MINUTES = 59;

    if (hours < MIN_HOURS || hours > MAX_HOURS) {
        return ValidationResult(false,
                                QString("Часы должны быть в диапазоне %1-%2")
                                    .arg(MIN_HOURS)
                                    .arg(MAX_HOURS));
    }

    if (minutes < MIN_MINUTES || minutes > MAX_MINUTES) {
        return ValidationResult(false,
                                QString("Минуты должны быть в диапазоне %1-%2")
                                    .arg(MIN_MINUTES)
                                    .arg(MAX_MINUTES));
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
        if (time < MIN_TRAVEL_TIME || time > MAX_TRAVEL_TIME) {
            return ValidationResult(false,
                                    QString("Время движения должно быть от %1 до %2 минут")
                                        .arg(MIN_TRAVEL_TIME)
                                        .arg(MAX_TRAVEL_TIME));
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
    const QStringList validTypes = {"автобус", "троллейбус", "трамвай"};
    if (!validTypes.contains(transportType.toLower())) {
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
