#ifndef TRANSPORTTYPE_H
#define TRANSPORTTYPE_H

#include <QString>
#include <QStringList>

class TransportType {
public:
    enum class Type { BUS, TROLLEYBUS, TRAM };

    explicit TransportType(Type type);
    TransportType(const QString& typeName);
    TransportType(const TransportType& other) = default; // Используем default implementation

    Type getType() const;
    QString getName() const;
    QString getShortName() const;
    int getId() const;

    bool operator==(const TransportType& other) const;

    static QStringList getAllTypeNames();

private:
    Type type;
};

#endif
