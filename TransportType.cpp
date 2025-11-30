#include "TransportType.h"

TransportType::TransportType(Type type) : type(type) {}

TransportType::TransportType(const QString& typeName) {
    if (typeName == "автобус" || typeName == "bus") {
        type = Type::BUS;
    }
    else if (typeName == "троллейбус" || typeName == "trolleybus") {
        type = Type::TROLLEYBUS;
    }
    else if (typeName == "трамвай" || typeName == "tram") {
        type = Type::TRAM;
    }
    else {
        type = Type::BUS;
    }
}

// УДАЛЕНО: Конструктор копирования, так как используется default implementation

TransportType::Type TransportType::getType() const {
    return type;
}

QString TransportType::getName() const {
    switch (type) {
    case Type::BUS: return "автобус";
    case Type::TROLLEYBUS: return "троллейбус";
    case Type::TRAM: return "трамвай";
    default: return "автобус";
    }
}

QString TransportType::getShortName() const {
    switch (type) {
    case Type::BUS: return "А";
    case Type::TROLLEYBUS: return "Тб";
    case Type::TRAM: return "Тм";
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
