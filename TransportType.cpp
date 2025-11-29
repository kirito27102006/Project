#include "TransportType.h"

TransportType::TransportType(Type type) : type(type) {}

TransportType::TransportType(const QString& typeName) {
    if (typeName == "автобус" || typeName == "bus") {
        type = BUS;
    }
    else if (typeName == "троллейбус" || typeName == "trolleybus") {
        type = TROLLEYBUS;
    }
    else if (typeName == "трамвай" || typeName == "tram") {
        type = TRAM;
    }
    else {
        type = BUS;
    }
}

TransportType::Type TransportType::getType() const {
    return type;
}

QString TransportType::getName() const {
    switch (type) {
    case BUS: return "автобус";
    case TROLLEYBUS: return "троллейбус";
    case TRAM: return "трамвай";
    default: return "автобус";
    }
}

QString TransportType::getShortName() const {
    switch (type) {
    case BUS: return "А";
    case TROLLEYBUS: return "Тб";
    case TRAM: return "Тм";
    default: return "А";
    }
}

int TransportType::getId() const {
    return static_cast<int>(type);
}

bool TransportType::operator==(const TransportType& other) const {
    return type == other.type;
}

QStringList TransportType::getAllTypeNames() {
    return { "автобус", "троллейбус", "трамвай" };
}
