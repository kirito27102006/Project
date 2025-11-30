#include "TransportType.h"

TransportType::TransportType(Type type) : type(type) {}

TransportType::TransportType(const QString& typeName) {
    using enum Type;
    if (typeName == TRANSPORT_BUS_NAME || typeName == "bus") {
        type = BUS;
    }
    else if (typeName == TRANSPORT_TROLLEYBUS_NAME || typeName == "trolleybus") {
        type = TROLLEYBUS;
    }
    else if (typeName == TRANSPORT_TRAM_NAME || typeName == "tram") {
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
    using enum Type;
    switch (type) {
    case BUS: return TRANSPORT_BUS_NAME;
    case TROLLEYBUS: return TRANSPORT_TROLLEYBUS_NAME;
    case TRAM: return TRANSPORT_TRAM_NAME;
    default: return TRANSPORT_BUS_NAME;
    }
}

QString TransportType::getShortName() const {
    using enum Type;
    switch (type) {
    case BUS: return TRANSPORT_BUS_SHORT;
    case TROLLEYBUS: return TRANSPORT_TROLLEYBUS_SHORT;
    case TRAM: return TRANSPORT_TRAM_SHORT;
    default: return TRANSPORT_BUS_SHORT;
    }
}

int TransportType::getId() const {
    return static_cast<int>(type);
}

bool TransportType::operator==(const TransportType& other) const {
    return type == other.type;
}

QStringList TransportType::getAllTypeNames() {
    return { TRANSPORT_BUS_NAME, TRANSPORT_TROLLEYBUS_NAME, TRANSPORT_TRAM_NAME };
}
