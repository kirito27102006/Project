#ifndef TRANSPORTTYPE_H
#define TRANSPORTTYPE_H

#include <QString>
#include <QStringList>

class TransportType {
public:
    enum class Type { BUS, TROLLEYBUS, TRAM };

    explicit TransportType(Type type);
    explicit TransportType(const QString& typeName);
    TransportType(const TransportType& other) = default;

    Type getType() const;
    QString getName() const;
    QString getShortName() const;
    int getId() const;

    bool operator==(const TransportType& other) const;

    static QStringList getAllTypeNames();

private:
    Type type;
};

// УДАЛЕНЫ неиспользуемые константы - они нигде не используются
// constexpr const char* TRANSPORT_BUS_NAME = "автобус";
// constexpr const char* TRANSPORT_TROLLEYBUS_NAME = "троллейбус";
// constexpr const char* TRANSPORT_TRAM_NAME = "трамвай";
// constexpr const char* TRANSPORT_BUS_SHORT = "А";
// constexpr const char* TRANSPORT_TROLLEYBUS_SHORT = "Тб";
// constexpr const char* TRANSPORT_TRAM_SHORT = "Тм";

#endif
