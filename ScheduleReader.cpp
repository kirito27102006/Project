#include "ScheduleReader.h"
#include "Transport.h"
#include "TransportType.h"
#include "TimeTransport.h"
#include <QDebug>

ScheduleReader::ScheduleReader(QObject *parent) : QObject(parent) {}

ScheduleReader::ReadResult ScheduleReader::readFromFile(const QString& filename,
                                                        std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback)
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
        if (!readStops(in, stopCount, result.allStops, findOrCreateStopCallback)) {
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
        if (!readSchedules(in, scheduleCount, result.schedules, findOrCreateStopCallback)) {
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

bool ScheduleReader::readStops(QTextStream& in, int stopCount, QVector<QSharedPointer<Stop>>& allStops,
                               std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback)
{
    for (int i = 0; i < stopCount; ++i) {
        QString name = in.readLine();
        QString coordinate = in.readLine();

        if (name.isNull()) {
            return false;
        }

        auto stop = findOrCreateStopCallback(name, coordinate);
        allStops.push_back(stop);
    }
    return true;
}

bool ScheduleReader::readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                                   std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback)
{
    for (int i = 0; i < scheduleCount; ++i) {
        if (in.readLine() != "ROUTE_START") {
            return false;
        }

        try {
            Schedule schedule = readSingleSchedule(in, findOrCreateStopCallback);
            schedules.push_back(schedule);
        } catch (const std::exception& e) {
            qDebug() << "Error reading schedule:" << e.what();
            return false;
        }

        if (in.readLine() != "ROUTE_END") {
            return false;
        }
    }
    return true;
}

Schedule ScheduleReader::readSingleSchedule(QTextStream& in,
                                            std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback)
{
    QString transportType = in.readLine();
    int transportId = in.readLine().toInt();

    QStringList timeParts = in.readLine().split(" ");
    if (timeParts.size() < 2) {
        throw std::runtime_error("Invalid time format");
    }
    int startHour = timeParts[0].toInt();
    int startMinute = timeParts[1].toInt();

    TransportType type(transportType);
    Transport transport(type, transportId);

    // Read days
    QString line = in.readLine();
    QStringList days;
    if (line.startsWith("DAYS:")) {
        int dayCount = line.mid(5).toInt();
        for (int j = 0; j < dayCount; ++j) {
            days.push_back(in.readLine());
        }
    }

    // Read route stops
    line = in.readLine();
    QVector<QSharedPointer<Stop>> routeStops;
    if (line.startsWith("ROUTE_STOPS:")) {
        int stopCount = line.mid(12).toInt();
        for (int j = 0; j < stopCount; ++j) {
            QString stopName = in.readLine();
            if (stopName.isNull()) {
                throw std::runtime_error("Unexpected end of file while reading stops");
            }
            routeStops.push_back(findOrCreateStopCallback(stopName, ""));
        }
    }

    // Read travel times
    line = in.readLine();
    QVector<int> travelTimes;
    if (line.startsWith("TRAVEL_TIMES:")) {
        int timeCount = line.mid(13).toInt();
        for (int j = 0; j < timeCount; ++j) {
            QString timeStr = in.readLine();
            if (timeStr.isNull()) {
                throw std::runtime_error("Unexpected end of file while reading travel times");
            }
            travelTimes.push_back(timeStr.toInt());
        }
    }

    // Create route
    if (routeStops.size() >= 2) {
        Route route(transport, routeStops[0], routeStops.last());
        route.setDays(days);

        // Add intermediate stops
        for (int j = 1; j < routeStops.size() - 1; ++j) {
            int travelTime = (j - 1 < travelTimes.size()) ? travelTimes[j - 1] : 0;
            route.addStop(routeStops[j], travelTime);
        }

        // Add final travel time
        if (!travelTimes.empty() && travelTimes.size() >= routeStops.size() - 1) {
            route.addFinalTravelTime(travelTimes.last());
        }

        return Schedule(route, TimeTransport(startHour, startMinute));
    }

    throw std::runtime_error("Invalid route data: not enough stops");
}
