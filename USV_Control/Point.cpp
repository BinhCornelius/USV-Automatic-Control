#include "Point.h"

Point::Point(QObject *parent) : QObject(parent), m_latitude(0), m_longitude(0) {}

Point::Point(double latitude, double longitude, QObject *parent)
    : QObject(parent), m_latitude(latitude), m_longitude(longitude) {}

double Point::latitude() const {
    return m_latitude;
}

void Point::setLatitude(double latitude) {
    if (m_latitude != latitude) {
        m_latitude = latitude;
        emit latitudeChanged();
    }
}

double Point::longitude() const {
    return m_longitude;
}

void Point::setLongitude(double longitude) {
    if (m_longitude != longitude) {
        m_longitude = longitude;
        emit longitudeChanged();
    }
}
