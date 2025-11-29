#include "ScheduleWriter.h"
#include "Transport.h"
#include "TransportType.h"
#include "TimeTransport.h"
#include <QDebug>

ScheduleWriter::ScheduleWriter(QObject *parent) : QObject(parent) {}

bool ScheduleWriter::writeToFile(const QString& filename, const QVector<Schedule>& schedules, const QVector<QSharedPointer<Stop>>& allStops)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for writing:" << filename;
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    writeStops(out, allStops);
    writeSchedules(out, schedules);

    file.close();
    qDebug() << "Successfully wrote" << schedules.size() << "schedules and" << allStops.size() << "stops to" << filename;
    return true;
}

void ScheduleWriter::writeStops(QTextStream& out, const QVector<QSharedPointer<Stop>>& allStops)
{
    out << "STOPS:" << allStops.size() << "\n";
    for (const auto& stop : allStops) {
        out << stop->getName() << "\n";
        out << stop->getCoordinate() << "\n";
    }
}

void ScheduleWriter::writeSchedules(QTextStream& out, const QVector<Schedule>& schedules)
{
    out << "SCHEDULES:" << schedules.size() << "\n";
    for (const auto& schedule : schedules) {
        writeSchedule(out, schedule);
    }
}

void ScheduleWriter::writeSchedule(QTextStream& out, const Schedule& schedule)
{
    const auto& route = schedule.getRoute();
    const auto& transport = route.getTransport();

    out << "ROUTE_START\n";
    out << transport.getType().getName() << "\n";
    out << transport.getId() << "\n";
    out << schedule.getStartTime().hours << " " << schedule.getStartTime().minutes << "\n";

    const auto& days = route.getDays();
    out << "DAYS:" << days.size() << "\n";
    for (const auto& day : days) {
        out << day << "\n";
    }

    const auto& stops = route.getStops();
    out << "ROUTE_STOPS:" << stops.size() << "\n";
    for (const auto& stop : stops) {
        out << stop.stop->getName() << "\n";
    }

    const auto& travelTimes = route.getTravelTimes();
    out << "TRAVEL_TIMES:" << travelTimes.size() << "\n";
    for (const auto& time : travelTimes) {
        out << time << "\n";
    }

    out << "ROUTE_END\n";
}
