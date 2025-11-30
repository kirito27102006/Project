#ifndef STOP_H
#define STOP_H

#include <QString>

class Stop {
public:
    explicit Stop(const QString& name = "", const QString& coordinate = "");

    QString getName() const;
    QString getCoordinate() const;
    void setName(const QString& newName);
    void setCoordinate(const QString& newCoordinate);

    bool operator==(const Stop& other) const;

private:
    QString name;
    QString coordinate;
};

class EndStop : public Stop {
public:
    explicit EndStop(const QString& name = "", const QString& coordinate = "");

    bool isTerminal() const;
};

#endif
