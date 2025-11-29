#include "Transport.h"

Transport::Transport(const TransportType& type, int id) : type(type), id(id) {}

TransportType::TransportType(const TransportType& other) : type(other.type) {}

TransportType Transport::getType() const {
    return type;
}

int Transport::getId() const {
    return id;
}

QString Transport::getFullName() const {
    return type.getName() + " №" + QString::number(id);
}

QString Transport::getShortName() const {
    return type.getShortName() + QString::number(id);
}
