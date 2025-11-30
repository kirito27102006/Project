#ifndef SCHEDULEREADER_H
#define SCHEDULEREADER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QSharedPointer>
#include <functional>
#include <stdexcept>
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

    ReadResult readFromFile(const QString& filename,
                            const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const;

private:
    bool readStops(QTextStream& in, int stopCount, QVector<QSharedPointer<Stop>>& allStops,
                   const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const;

    bool readSchedules(QTextStream& in, int scheduleCount, QVector<Schedule>& schedules,
                       const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const;

    Schedule readSingleSchedule(QTextStream& in,
                                const std::function<QSharedPointer<Stop>(const QString&, const QString&)>& findOrCreateStopCallback) const;
};

#endif // SCHEDULEREADER_H
