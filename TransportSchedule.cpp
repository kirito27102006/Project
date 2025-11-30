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
#include <QException>
#include <algorithm>
#include <ranges>
#include <QDebug>
#include <climits>

TransportSchedule::TransportSchedule(const QString& file, QObject* parent)
    : QObject(parent), filename(file), stopsDirty(true),
    scheduleReader(new ScheduleReader(this)),
    scheduleWriter(new ScheduleWriter(this))
{
    loadFromFile();
}

void TransportSchedule::addRoute(const Transport& transport, QSharedPointer<Stop> startStop,
                                 QSharedPointer<Stop> endStop, const QVector<QSharedPointer<Stop>>& intermediateStops,
                                 const QVector<int>& travelTimes, const QStringList& days, const TimeTransport& startTime)
{
    // Валидация данных с использованием ValidationService
    if (auto validationResult = ValidationService::validateRouteData(
            transport.getId(),
            QVector<QSharedPointer<Stop>>() << startStop << intermediateStops << endStop,
            travelTimes,
            days
            ); !validationResult.isValid) {
        throw std::invalid_argument(validationResult.errorMessage.toStdString());
    }

    // Создаем маршрут
    Route route(transport, startStop, endStop);
    route.setDays(days);

    // Добавляем промежуточные остановки
    QVector<QSharedPointer<Stop>> allStops;
    allStops.push_back(startStop);
    allStops.append(intermediateStops);
    allStops.push_back(endStop);

    // Добавляем остановки в маршрут
    const int intermediateStopCount = allStops.size() >= 2 ? static_cast<int>(allStops.size()) - 2 : 0;
    for (int i = 0; i < intermediateStopCount; ++i) {
        const int stopIndex = i + 1;
        route.addStop(allStops[stopIndex], travelTimes[i]);
    }

    // Добавляем время до конечной остановки
    route.addFinalTravelTime(travelTimes.last());

    // Рассчитываем время прибытия с использованием ArrivalTimeService
    route.calculateArrivalTimes(startTime);

    // Создаем расписание и добавляем его
    Schedule schedule(route, startTime);
    schedules.push_back(schedule);

    stopsDirty = true;
    saveToFile();

    qDebug() << "Добавлен маршрут №" << transport.getId() << "с" << allStops.size() << "остановками";
}

void TransportSchedule::removeRoute(int routeNumber)
{
    auto it = std::remove_if(schedules.begin(), schedules.end(),
                             [routeNumber](const Schedule& s) {
                                 return s.getRoute().getRouteNumber() == routeNumber;
                             });

    if (it != schedules.end()) {
        schedules.erase(it, schedules.end());
        stopsDirty = true;
        saveToFile();
    }
}

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
        } catch (const std::out_of_range& e) {
            qDebug() << "Остановка не найдена в маршруте" << route.getRouteNumber() << ":" << e.what();
            continue;
        } catch (const std::exception& e) {
            qDebug() << "Ошибка получения времени прибытия для маршрута"
                     << route.getRouteNumber() << ":" << e.what();
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
                          } catch (const std::out_of_range&) {
                              return false;
                          } catch (const std::exception&) {
                              return false;
                          }
                      });

    qDebug() << "Всего найдено маршрутов:" << result.size();
    return result;
}

void TransportSchedule::saveToFile()
{
    if (!scheduleWriter) {
        qDebug() << "ScheduleWriter is not initialized";
        return;
    }

    if (!scheduleWriter->writeToFile(filename, schedules, allStops)) {
        qDebug() << "Failed to save schedule to file:" << filename;
    } else {
        qDebug() << "Schedule successfully saved to:" << filename;
    }
}

void TransportSchedule::loadFromFile() {
    if (!scheduleReader) {
        qDebug() << "ScheduleReader is not initialized";
        return;
    }

    // Используем лямбду с правильной сигнатураой
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
        qDebug() << "Failed to load schedule from file:" << result.errorMessage;
        schedules.clear();
        allStops.clear();
    }
}

void TransportSchedule::updateRoute(int oldRouteNumber, const Transport& transport,
                                    QSharedPointer<Stop> startStop, QSharedPointer<Stop> endStop,
                                    const QVector<QSharedPointer<Stop>>& intermediateStops,
                                    const QVector<int>& travelTimes, const QStringList& days,
                                    const TimeTransport& startTime)
{
    // Удаляем старый маршрут
    removeRoute(oldRouteNumber);

    // Добавляем обновленный маршрут
    addRoute(transport, startStop, endStop, intermediateStops, travelTimes, days, startTime);

    qDebug() << "Маршрут №" << oldRouteNumber << "обновлен";
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
        throw std::invalid_argument(validationResult.errorMessage.toStdString());
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

// ВОССТАНОВЛЕНЫ методы статистики - они используются в mainwindow.cpp
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
