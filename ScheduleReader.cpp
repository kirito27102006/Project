#include "ScheduleReader.h"
#include "Transport.h"
#include "TransportType.h"
#include "TimeTransport.h"
#include <QDebug>

ScheduleReader::ScheduleReader(QObject *parent) : QObject(parent) {}

ScheduleReader::ReadResult ScheduleReader::readFromFile(const QString& filename,
                                                        const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const
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
                               const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const
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

        auto stop = findOrCreateStopCallback(name, coordinate);
        allStops.push_back(stop);
    }
    return true;
}

bool ScheduleReader::readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                                   const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const
{
    for (int i = 0; i < scheduleCount; ++i) {
        if (in.readLine() != "ROUTE_START") {
            return false;
        }

        try {
            Schedule schedule = readSingleSchedule(in, findOrCreateStopCallback);
            schedules.push_back(schedule);
        } catch (const FileFormatException& e) {
            qDebug() << "Error reading schedule:" << e.what();
            return false;
        } catch (const std::exception& e) {
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

Schedule ScheduleReader::readSingleSchedule(QTextStream& in,
                                            const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const
{
    QString transportTypeStr = in.readLine();
    if (transportTypeStr.isNull()) {
        throw FileFormatException("Unexpected end of file while reading transport type");
    }

    QString idLine = in.readLine();
    if (idLine.isNull()) {
        throw FileFormatException("Unexpected end of file while reading transport ID");
    }
    int transportId = idLine.toInt();

    QString timeLine = in.readLine();
    if (timeLine.isNull()) {
        throw FileFormatException("Unexpected end of file while reading time");
    }

    QStringList timeParts = timeLine.split(" ");
    if (timeParts.size() < 2) {
        throw TimeFormatException("Invalid time format in schedule");
    }

    bool ok1;
    bool ok2;
    int startHour = timeParts[0].toInt(&ok1);
    int startMinute = timeParts[1].toInt(&ok2);

    if (!ok1 || !ok2) {
        throw TimeFormatException("Invalid time values in schedule");
    }

    TransportType transportType(transportTypeStr);
    Transport transport(transportType, transportId);

    // Read days
    QString line = in.readLine();
    QStringList days;
    if (!line.isNull() && line.startsWith("DAYS:")) {
        int dayCount = line.mid(5).toInt();
        for (int j = 0; j < dayCount; ++j) {
            QString day = in.readLine();
            if (day.isNull()) {
                throw FileFormatException("Unexpected end of file while reading days");
            }
            days.push_back(day);
        }
        line = in.readLine(); // Read next line
    }

    // Read route stops
    QVector<QSharedPointer<Stop>> routeStops;
    if (!line.isNull() && line.startsWith("ROUTE_STOPS:")) {
        int stopCount = line.mid(12).toInt();
        for (int j = 0; j < stopCount; ++j) {
            QString stopName = in.readLine();
            if (stopName.isNull()) {
                throw FileFormatException("Unexpected end of file while reading stops");
            }
            routeStops.push_back(findOrCreateStopCallback(stopName, ""));
        }
        line = in.readLine(); // Read next line
    }

    // Read travel times
    QVector<int> travelTimes;
    if (!line.isNull() && line.startsWith("TRAVEL_TIMES:")) {
        int timeCount = line.mid(13).toInt();
        for (int j = 0; j < timeCount; ++j) {
            QString timeStr = in.readLine();
            if (timeStr.isNull()) {
                throw FileFormatException("Unexpected end of file while reading travel times");
            }
            bool ok;
            int time = timeStr.toInt(&ok);
            if (!ok) {
                throw FileFormatException("Invalid travel time value");
            }
            travelTimes.push_back(time);
        }
    }

    // Create route
    if (routeStops.size() < 2) {
        throw RouteDataException("Invalid route data: not enough stops");
    }

    Route route(transport, routeStops[0], routeStops.last());
    route.setDays(days);

    // Add intermediate stops
    const int intermediateStopCount = routeStops.size() >= 2 ? static_cast<int>(routeStops.size()) - 2 : 0;
    for (int j = 0; j < intermediateStopCount; ++j) {
        const int stopIndex = j + 1;
        int travelTime = (j < travelTimes.size()) ? travelTimes[j] : 5; // default 5 minutes
        route.addStop(routeStops[stopIndex], travelTime);
    }

    // Add final travel time
    if (!travelTimes.empty() && travelTimes.size() >= routeStops.size() - 1) {
        route.addFinalTravelTime(travelTimes.last());
    } else if (!travelTimes.empty()) {
        route.addFinalTravelTime(travelTimes.last());
    }

    // Calculate arrival times
    TimeTransport startTime(startHour, startMinute);
    route.calculateArrivalTimes(startTime);

    return Schedule(route, startTime);
}
