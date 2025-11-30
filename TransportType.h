#ifndef TRANSPORTTYPE_H
#define TRANSPORTTYPE_H

#include <QString>
#include <QStringList>
#include <utility>

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

    bool operator==(const TransportType& other) const = default;

    static QStringList getAllTypeNames();

private:
    Type type;
};

#endif
