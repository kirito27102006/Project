#ifndef COORDINATESERVICE_H
#define COORDINATESERVICE_H

#include <QString>
#include <QPointF>
#include <cmath>

class CoordinateService
{
public:
    // Константы для географических расчетов
    static constexpr double EARTH_RADIUS_METERS = 6371000.0;
    static constexpr double MAX_DISTANCE_CLOSE = 100.0; // метров
    static constexpr double MIN_LATITUDE = -90.0;
    static constexpr double MAX_LATITUDE = 90.0;
    static constexpr double MIN_LONGITUDE = -180.0;
    static constexpr double MAX_LONGITUDE = 180.0;
    static constexpr double DEGREES_TO_RADIANS = M_PI / 180.0;

    struct Coordinate {
        double latitude;
        double longitude;
        bool isValid;

        explicit Coordinate(double lat = 0, double lon = 0, bool valid = false)
            : latitude(lat), longitude(lon), isValid(valid) {}
    };

    static Coordinate parseCoordinate(const QString& coordinateString);
    static QString formatCoordinate(const Coordinate& coord);
    static bool isValidCoordinate(const QString& coordinateString);
    static double calculateDistance(const Coordinate& coord1, const Coordinate& coord2);
    static QPointF toPointF(const Coordinate& coord);
    static bool areCoordinatesClose(const Coordinate& coord1, const Coordinate& coord2, double maxDistance = MAX_DISTANCE_CLOSE);
};

#endif // COORDINATESERVICE_H
