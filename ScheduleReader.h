#ifndef SCHEDULEREADER_H
#define SCHEDULEREADER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QSharedPointer>
#include <functional>
#include <stdexcept>
#include <utility>
#include "Schedule.h"
#include "Stop.h"
#include "Transport.h"
#include "TransportType.h"
#include "TimeTransport.h"
#include <QDebug>

class ScheduleReader : public QObject
{
    Q_OBJECT

public:
    explicit ScheduleReader(QObject *parent = nullptr);

    struct ReadResult {
        QVector<Schedule> schedules;
        QVector<QSharedPointer<Stop>> allStops;
        bool success;
        QString errorMessage;
    };

    class FileFormatException : public std::runtime_error {
    public:
        explicit FileFormatException(const QString& message)
            : std::runtime_error(message.toStdString()) {}
    };

    class TimeFormatException : public FileFormatException {
    public:
        using FileFormatException::FileFormatException;
    };

    class RouteDataException : public FileFormatException {
    public:
        using FileFormatException::FileFormatException;
    };

    // Основной метод с шаблонным параметром
    template<typename StopCreator>
    ReadResult readFromFile(const QString& filename, StopCreator&& findOrCreateStopCallback) const;

private:
    // Приватные методы также становятся шаблонными
    template<typename StopCreator>
    bool readStops(QTextStream& in, int stopCount, QVector<QSharedPointer<Stop>>& allStops,
                   StopCreator&& findOrCreateStopCallback) const;

    template<typename StopCreator>
    bool readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                       StopCreator&& findOrCreateStopCallback) const;

    template<typename StopCreator>
    Schedule readSingleSchedule(QTextStream& in, StopCreator&& findOrCreateStopCallback) const;

    QStringList readDays(QTextStream& in) const;

    QVector<QSharedPointer<Stop>> readRouteStops(
        QTextStream& in,
        int& stopCount,
        auto&& findOrCreateStopCallback) const;

    QVector<int> readTravelTimes(QTextStream& in, int& timeCount) const;

    std::pair<int, int> readStartTime(const QString& timeLine) const;

    void addIntermediateStops(Route& route, const QVector<QSharedPointer<Stop>>& routeStops,
                                              const QVector<int>& travelTimes) const;
};

// Реализация шаблонных методов прямо в header-файле
template<typename StopCreator>
ScheduleReader::ReadResult ScheduleReader::readFromFile(const QString& filename, StopCreator&& findOrCreateStopCallback) const
{
    ReadResult result;
    result.success = false;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = "Cannot open file for reading: " + filename;
        return result;
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    QString line = in.readLine();
    if (line.isNull()) {
        result.errorMessage = "File is empty";
        file.close();
        return result;
    }

    // Read stops
    if (line.startsWith("STOPS:")) {
        int stopCount = line.mid(6).toInt();
        if (!readStops(in, stopCount, result.allStops, std::forward<StopCreator>(findOrCreateStopCallback))) {
            result.errorMessage = "Error reading stops";
            file.close();
            return result;
        }
    } else {
        result.errorMessage = "Invalid file format: STOPS section not found";
        file.close();
        return result;
    }

    // Read schedules
    line = in.readLine();
    if (line.startsWith("SCHEDULES:")) {
        int scheduleCount = line.mid(10).toInt();
        if (!readSchedules(in, scheduleCount, result.schedules, std::forward<StopCreator>(findOrCreateStopCallback))) {
            result.errorMessage = "Error reading schedules";
            file.close();
            return result;
        }
    } else {
        result.errorMessage = "Invalid file format: SCHEDULES section not found";
        file.close();
        return result;
    }

    file.close();
    result.success = true;
    return result;
}

template<typename StopCreator>
bool ScheduleReader::readStops(QTextStream& in, int stopCount, QVector<QSharedPointer<Stop>>& allStops,
                               StopCreator&& findOrCreateStopCallback) const
{
    for (int i = 0; i < stopCount; ++i) {
        QString name = in.readLine();
        if (name.isNull()) {
            return false;
        }

        QString coordinate = in.readLine();
        if (coordinate.isNull()) {
            return false;
        }

        auto stop = std::forward<StopCreator>(findOrCreateStopCallback)(name, coordinate);
        allStops.push_back(stop);
    }
    return true;
}

template<typename StopCreator>
bool ScheduleReader::readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                                   StopCreator&& findOrCreateStopCallback) const
{
    for (int i = 0; i < scheduleCount; ++i) {
        if (in.readLine() != "ROUTE_START") {
            return false;
        }

        try {
            Schedule schedule = readSingleSchedule(in, std::forward<StopCreator>(findOrCreateStopCallback));
            schedules.push_back(schedule);
        } catch (const FileFormatException& e) {
            qDebug() << "Error reading schedule:" << e.what();
            return false;
        }

        QString endLine = in.readLine();
        if (endLine != "ROUTE_END") {
            qDebug() << "Expected ROUTE_END, got:" << endLine;
            return false;
        }
    }
    return true;
}


template<typename StopCreator>
QVector<QSharedPointer<Stop>> ScheduleReader::readRouteStops(
    QTextStream& in,
    int& stopCount,
    StopCreator&& findOrCreateStopCallback) const
{
    QVector<QSharedPointer<Stop>> stops;

    QString line = in.readLine();
    if (!line.startsWith("ROUTE_STOPS:"))
        return stops;

    stopCount = line.mid(12).toInt();

    for (int i = 0; i < stopCount; ++i) {
        QString name = in.readLine();
        if (name.isNull())
            throw FileFormatException("Unexpected end of file while reading route stop");
        stops.push_back(findOrCreateStopCallback(name, ""));
    }

    return stops;
}

template<typename StopCreator>
Schedule ScheduleReader::readSingleSchedule(QTextStream& in, StopCreator&& findOrCreateStopCallback) const
{
    // Transport type
    QString transportTypeStr = in.readLine();
    if (transportTypeStr.isNull())
        throw FileFormatException("Unexpected end of file while reading transport type");

    // Transport ID
    QString idLine = in.readLine();
    if (idLine.isNull())
        throw FileFormatException("Unexpected end of file while reading transport ID");
    int transportId = idLine.toInt();

    // Start time
    QString timeLine = in.readLine();
    if (timeLine.isNull())
        throw FileFormatException("Unexpected end of file while reading time");
    auto[startHour, startMinute] = readStartTime(timeLine);

    TransportType transportType(transportTypeStr);
    Transport transport(transportType, transportId);

    // DAYS:
    QStringList days = readDays(in);

    // ROUTE_STOPS:
    int routeStopCount = 0;
    QVector<QSharedPointer<Stop>> routeStops =
        readRouteStops(in, routeStopCount, std::forward<StopCreator>(findOrCreateStopCallback));

    // TRAVEL_TIMES:
    int travelTimeCount = 0;
    QVector<int> travelTimes = readTravelTimes(in, travelTimeCount);

    // Validate
    if (routeStops.size() < 2)
        throw RouteDataException("Invalid route data: not enough stops");

    // Build route
    Route route(transport, routeStops[0], routeStops.last());
    route.setDays(days);

    addIntermediateStops(route, routeStops, travelTimes);

    if (!travelTimes.isEmpty())
        route.addFinalTravelTime(travelTimes.last());

    TimeTransport startTime(startHour, startMinute);
    route.calculateArrivalTimes(startTime);

    return Schedule(route, startTime);
}




#endif // SCHEDULEREADER_H
