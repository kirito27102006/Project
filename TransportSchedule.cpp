#include "TransportSchedule.h"
#include "ScheduleReader.h"
#include "ScheduleWriter.h"
#include "ArrivalTimeService.h"
#include "DayOfWeekService.h"
#include "ValidationService.h"
#include "SearchService.h"
#include "StatisticsService.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>
#include <ranges>
#include <QDebug>
#include <climits>

TransportSchedule::TransportSchedule(const QString& file, QObject* parent)
    : QObject(parent), filename(file),
    scheduleReader(new ScheduleReader(this)),
    scheduleWriter(new ScheduleWriter(this))
{
    loadFromFile();
}

void TransportSchedule::addRoute(const RouteParams& params)
{
    // Создаем маршрут из параметров
    Route route = createRouteFromParams(params);

    // Создаем расписание и добавляем его
    Schedule schedule(route, params.startTime);
    schedules.push_back(schedule);

    stopsDirty = true;
    saveToFile();

    qDebug() << "Добавлен маршрут №" << params.transport.getId() << "с"
             << (params.intermediateStops.size() + 2) << "остановками";
}

void TransportSchedule::removeRoute(int routeNumber)
{
    auto [it, end] = std::ranges::remove_if(schedules,
                                            [routeNumber](const Schedule& s) {
                                                return s.getRoute().getRouteNumber() == routeNumber;
                                            });
    if (it != end) {
        schedules.erase(it, end);
        stopsDirty = true;
        saveToFile();
    } else {
        throw RouteNotFoundException(routeNumber);
    }
}

void TransportSchedule::updateRoute(int oldRouteNumber, const RouteParams& params)
{
    // Удаляем старый маршрут
    removeRoute(oldRouteNumber);

    // Добавляем обновленный маршрут
    addRoute(params);

    qDebug() << "Маршрут №" << oldRouteNumber << "обновлен на №" << params.transport.getId();
}

// Вспомогательный метод для создания маршрута из параметров
Route TransportSchedule::createRouteFromParams(const RouteParams& params) const
{
    // Валидация данных с использованием ValidationService
    QVector<QSharedPointer<Stop>> allRouteStops;
    allRouteStops.reserve(params.intermediateStops.size() + 2);
    allRouteStops << params.startStop << params.intermediateStops << params.endStop;

    if (auto validationResult = ValidationService::validateRouteData(
            params.transport.getId(),
            allRouteStops,
            params.travelTimes,
            params.days
            ); !validationResult.isValid) {
        throw InvalidRouteDataException(validationResult.errorMessage);
    }

    // Создаем маршрут
    Route route(params.transport, params.startStop, params.endStop);
    route.setDays(params.days);

    // Добавляем промежуточные остановки
    const int intermediateStopCount = allRouteStops.size() >= 2 ? static_cast<int>(allRouteStops.size()) - 2 : 0;
    for (int i = 0; i < intermediateStopCount; ++i) {
        const int stopIndex = i + 1;
        route.addStop(allRouteStops[stopIndex], params.travelTimes[i]);
    }

    // Добавляем время до конечной остановки
    route.addFinalTravelTime(params.travelTimes.last());

    // Рассчитываем время прибытия с использованием ArrivalTimeService
    route.calculateArrivalTimes(params.startTime);

    return route;
}

// Остальные методы без изменений
QVector<Schedule> TransportSchedule::getSchedulesForDay(const QString& day) const
{
    return SearchService::findSchedulesByDay(schedules, day);
}

QVector<Schedule> TransportSchedule::getSchedulesForStop(const QString& stopName) const
{
    return SearchService::findSchedulesByStop(schedules, stopName);
}

QVector<Schedule> TransportSchedule::findNextTransport(const QString& stopName) const
{
    TimeTransport currentTime = getCurrentTime();
    QString currentDay = DayOfWeekService::getCurrentDay();

    qDebug() << "Поиск транспорта для остановки:" << stopName;
    qDebug() << "Текущий день:" << currentDay;
    qDebug() << "Текущее время:" << currentTime.toString();

    QVector<Schedule> result;

    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();

        // Используем DayOfWeekService для проверки работы маршрута сегодня
        if (!DayOfWeekService::isRouteActiveToday(route.getDays())) {
            qDebug() << "Маршрут" << route.getRouteNumber() << "не работает сегодня";
            continue;
        }

        // Проверяем, проходит ли маршрут через указанную остановку
        bool hasStop = false;
        for (const auto& routeStop : route.getStops()) {
            if (routeStop.stop->getName().compare(stopName, Qt::CaseInsensitive) == 0) {
                hasStop = true;
                break;
            }
        }

        if (!hasStop) {
            qDebug() << "Маршрут" << route.getRouteNumber() << "не проходит через остановку" << stopName;
            continue;
        }

        // Получаем время прибытия на остановку
        try {
            TimeTransport arrivalTime = route.getArrivalTimeAtStop(stopName);

            // Используем ArrivalTimeService для расчета времени ожидания
            int waitMinutes = ArrivalTimeService::calculateWaitTime(currentTime, arrivalTime);

            // Включаем в результат только если время ожидания разумное (до 24 часов)
            if (waitMinutes >= 0 && waitMinutes <= 24 * 60) {
                result.push_back(schedule);

                qDebug() << "Найден маршрут:" << route.getRouteNumber()
                         << "Время прибытия:" << arrivalTime.toString()
                         << "Ожидание:" << waitMinutes << "мин";
            } else {
                qDebug() << "Маршрут" << route.getRouteNumber() << "исключен: время ожидания" << waitMinutes << "мин";
            }
        } catch (const StopNotFoundException& e) {
            qDebug() << "Остановка не найдена в маршруте" << route.getRouteNumber() << ":" << e.what();
            continue;
        }
    }

    // Сортируем по времени ожидания (от меньшего к большему)
    std::ranges::sort(result,
                      [stopName, currentTime](const Schedule& a, const Schedule& b) {
                          try {
                              TimeTransport timeA = a.getRoute().getArrivalTimeAtStop(stopName);
                              TimeTransport timeB = b.getRoute().getArrivalTimeAtStop(stopName);

                              int waitA = ArrivalTimeService::calculateWaitTime(currentTime, timeA);
                              int waitB = ArrivalTimeService::calculateWaitTime(currentTime, timeB);

                              return waitA < waitB;
                          } catch (const StopNotFoundException&) {
                              return false;
                          }
                      });

    qDebug() << "Всего найдено маршрутов:" << result.size();
    return result;
}

void TransportSchedule::saveToFile() const
{
    if (!scheduleWriter) {
        throw TransportScheduleException("ScheduleWriter не инициализирован");
    }

    if (!scheduleWriter->writeToFile(filename, schedules, allStops)) {
        throw FileOperationException(QString("Не удалось сохранить расписание в файл: %1").arg(filename));
    } else {
        qDebug() << "Schedule successfully saved to:" << filename;
    }
}

void TransportSchedule::loadFromFile() {
    if (!scheduleReader) {
        throw TransportScheduleException("ScheduleReader не инициализирован");
    }

    // Используем лямбду с правильной сигнатурой
    auto stopCreator = [this](const QString& name, const QString& coordinate) {
        return findOrCreateStop(name, coordinate);
    };

    auto result = scheduleReader->readFromFile(filename, stopCreator);

    if (result.success) {
        schedules = result.schedules;
        allStops = result.allStops;
        stopsDirty = true;
        qDebug() << "Successfully loaded" << schedules.size() << "schedules and" << allStops.size() << "stops from" << filename;
    } else {
        throw FileOperationException(QString("Не удалось загрузить расписание из файла: %1").arg(result.errorMessage));
    }
}

void TransportSchedule::updateRoute(int oldRouteNumber, const Route& newRoute, const TimeTransport& startTime)
{
    // Удаляем старый маршрут
    removeRoute(oldRouteNumber);

    // Создаем новое расписание
    Schedule newSchedule(newRoute, startTime);
    schedules.push_back(newSchedule);

    stopsDirty = true;
    saveToFile();

    qDebug() << "Маршрут №" << oldRouteNumber << "обновлен на №" << newRoute.getRouteNumber();
}

QSharedPointer<Stop> TransportSchedule::findOrCreateStop(const QString& name, const QString& coordinate)
{
    // Валидация данных остановки
    if (auto validationResult = ValidationService::validateStopData(name, coordinate); !validationResult.isValid) {
        throw InvalidRouteDataException(validationResult.errorMessage);
    }

    // Ищем остановку по имени (без учета регистра)
    for (const auto& stop : allStops) {
        if (stop->getName().compare(name, Qt::CaseInsensitive) == 0) {
            // Если нашли остановку с таким именем, обновляем координату если нужно
            if (!coordinate.isEmpty() && stop->getCoordinate() != coordinate) {
                stop->setCoordinate(coordinate);
            }
            return stop;
        }
    }

    auto newStop = QSharedPointer<Stop>::create(name, coordinate);
    allStops.push_back(newStop);
    stopsDirty = true;
    return newStop;
}

QVector<QSharedPointer<Stop>> TransportSchedule::getAllStops() const
{
    return allStops;
}

QVector<QSharedPointer<Stop>> TransportSchedule::getActiveStops() const
{
    if (stopsDirty) {
        updateActiveStops();
    }
    return activeStops;
}

void TransportSchedule::updateActiveStops() const {
    QSet<QString> usedStopNames;

    // Собираем все остановки, которые используются в активных маршрутах
    for (const auto& schedule : schedules) {
        const auto& route = schedule.getRoute();
        for (const auto& routeStop : route.getStops()) {
            usedStopNames.insert(routeStop.getName().toLower());
        }
    }

    // Создаем список активных остановок (уникальных по имени)
    activeStops.clear();
    QSet<QString> addedNames;

    for (const auto& stop : allStops) {
        auto stopNameLower = stop->getName().toLower();
        if (usedStopNames.contains(stopNameLower) && !addedNames.contains(stopNameLower)) {
            activeStops.push_back(stop);
            addedNames.insert(stopNameLower);
        }
    }

    stopsDirty = false;
}

QVector<Schedule> TransportSchedule::getAllSchedules() const
{
    return schedules;
}

QStringList TransportSchedule::getAllRouteNumbers() const
{
    QStringList result;
    for (const auto& schedule : schedules) {
        QString routeNum = QString::number(schedule.getRoute().getRouteNumber());
        if (!result.contains(routeNum)) {
            result.push_back(routeNum);
        }
    }
    return result;
}

TimeTransport TransportSchedule::getCurrentTime() const
{
    QTime currentTime = QTime::currentTime();
    return TimeTransport(currentTime.hour(), currentTime.minute());
}

QString TransportSchedule::getCurrentDayOfWeek() const
{
    return DayOfWeekService::getCurrentDay();
}

StatisticsService::RouteStats TransportSchedule::getRouteStatistics() const
{
    return StatisticsService::calculateRouteStatistics(schedules);
}

StatisticsService::StopStats TransportSchedule::getStopStatistics() const
{
    return StatisticsService::calculateStopStatistics(schedules, allStops);
}

QMap<QString, int> TransportSchedule::getDailyScheduleCount() const
{
    return StatisticsService::calculateDailyScheduleCount(schedules);
}
