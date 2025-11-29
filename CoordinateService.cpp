#include "CoordinateService.h"
#include <QRegularExpression>

CoordinateService::Coordinate CoordinateService::parseCoordinate(const QString& coordinateString)
{
    if (coordinateString.isEmpty()) {
        return Coordinate(0, 0, false);
    }

    // Поддержка форматов: "55.7558,37.6173" или "55.7558;37.6173"
    QString normalized = coordinateString;
    normalized = normalized.replace(';', ',');

    QStringList parts = normalized.split(',');
    if (parts.size() != 2) {
        return Coordinate(0, 0, false);
    }

    bool ok1;
    bool ok2;
    double lat = parts[0].trimmed().toDouble(&ok1);
    double lon = parts[1].trimmed().toDouble(&ok2);

    if (!ok1 || !ok2) {
        return Coordinate(0, 0, false);
    }

    // Проверка допустимых диапазонов координат
    if (lat < -90 || lat > 90 || lon < -180 || lon > 180) {
        return Coordinate(0, 0, false);
    }

    return Coordinate(lat, lon, true);
}

QString CoordinateService::formatCoordinate(const Coordinate& coord)
{
    if (!coord.isValid) {
        return "";
    }
    return QString("%1, %2").arg(coord.latitude, 0, 'f', 6).arg(coord.longitude, 0, 'f', 6);
}

bool CoordinateService::isValidCoordinate(const QString& coordinateString)
{
    return parseCoordinate(coordinateString).isValid;
}

double CoordinateService::calculateDistance(const Coordinate& coord1, const Coordinate& coord2)
{
    if (!coord1.isValid || !coord2.isValid) {
        return -1;
    }

    // Формула гаверсинусов для расчета расстояния на сфере
    const double R = 6371000; // Радиус Земли в метрах

    double lat1 = coord1.latitude * M_PI / 180;
    double lon1 = coord1.longitude * M_PI / 180;
    double lat2 = coord2.latitude * M_PI / 180;
    double lon2 = coord2.longitude * M_PI / 180;

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = sin(dlat/2) * sin(dlat/2) +
               cos(lat1) * cos(lat2) *
                   sin(dlon/2) * sin(dlon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c; // Расстояние в метрах
}

QPointF CoordinateService::toPointF(const Coordinate& coord)
{
    return QPointF(coord.longitude, coord.latitude);
}

bool CoordinateService::areCoordinatesClose(const Coordinate& coord1, const Coordinate& coord2, double maxDistance)
{
    double distance = calculateDistance(coord1, coord2);
    return distance >= 0 && distance <= maxDistance;
}
