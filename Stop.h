#ifndef STOP_H
#define STOP_H

#include <QString>

class Stop {
public:
    Stop(const QString& name = "", const QString& coordinate = "");

    QString getName() const;
    QString getCoordinate() const;
    void setName(const QString& name);
    void setCoordinate(const QString& coordinate);

    bool operator==(const Stop& other) const;

protected:
    QString name;
    QString coordinate;
};

class EndStop : public Stop {
public:
    EndStop(const QString& name = "", const QString& coordinate = "");

    bool isTerminal() const;
};

#endif
