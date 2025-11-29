#include "Stop.h"

Stop::Stop(const QString& name, const QString& coordinate)
    : name(name), coordinate(coordinate) {}

QString Stop::getName() const {
    return name;
}

QString Stop::getCoordinate() const {
    return coordinate;
}

void Stop::setName(const QString& name) {
    this->name = name;
}

void Stop::setCoordinate(const QString& coordinate) {
    this->coordinate = coordinate;
}

bool Stop::operator==(const Stop& other) const {
    return name == other.name && coordinate == other.coordinate;
}

EndStop::EndStop(const QString& name, const QString& coordinate)
    : Stop(name, coordinate) {}

bool EndStop::isTerminal() const {
    return true;
}
