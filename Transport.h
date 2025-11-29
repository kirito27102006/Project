#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QString>
#include "TransportType.h"

class Transport {
public:
    Transport(const TransportType& type, int id);

    TransportType getType() const;
    int getId() const;
    QString getFullName() const;
    QString getShortName() const;

private:
    TransportType type;
    int id;
};

#endif
