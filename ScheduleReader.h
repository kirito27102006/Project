#ifndef SCHEDULEREADER_H
#define SCHEDULEREADER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QSharedPointer>
#include <functional>
#include "Schedule.h"
#include "Stop.h"

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

    ReadResult readFromFile(const QString& filename,
                            std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback);

private:
    bool readStops(QTextStream& in, int stopCount, QVector<QSharedPointer<Stop>>& allStops,
                   std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback);
    bool readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                       std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback);
    Schedule readSingleSchedule(QTextStream& in,
                                std::function<QSharedPointer<Stop>(const QString&, const QString&)> findOrCreateStopCallback);
};

#endif // SCHEDULEREADER_H
