#ifndef SCHEDULEWRITER_H
#define SCHEDULEWRITER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QSharedPointer>
#include "Schedule.h"
#include "Stop.h"

class ScheduleWriter : public QObject
{
    Q_OBJECT

public:
    explicit ScheduleWriter(QObject *parent = nullptr);
    bool writeToFile(const QString& filename, const QVector<Schedule>& schedules, const QVector<QSharedPointer<Stop>>& allStops) const;

private:
    void writeStops(QTextStream& out, const QVector<QSharedPointer<Stop>>& allStops) const;
    void writeSchedules(QTextStream& out, const QVector<Schedule>& schedules) const;
    void writeSchedule(QTextStream& out, const Schedule& schedule) const;
};

#endif // SCHEDULEWRITER_H
