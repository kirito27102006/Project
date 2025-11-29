#ifndef COORDINATESERVICE_H
#define COORDINATESERVICE_H

#include <QString>
#include <QPointF>
#include <cmath>

class CoordinateService
{
public:
    struct Coordinate {
        double latitude;
        double longitude;
        bool isValid;

        Coordinate(double lat = 0, double lon = 0, bool valid = false)
            : latitude(lat), longitude(lon), isValid(valid) {}
    };

    static Coordinate parseCoordinate(const QString& coordinateString);
    static QString formatCoordinate(const Coordinate& coord);
    static bool isValidCoordinate(const QString& coordinateString);
    static double calculateDistance(const Coordinate& coord1, const Coordinate& coord2);
    static QPointF toPointF(const Coordinate& coord);
    static bool areCoordinatesClose(const Coordinate& coord1, const Coordinate& coord2, double maxDistance = 100);
};

#endif // COORDINATESERVICE_H
