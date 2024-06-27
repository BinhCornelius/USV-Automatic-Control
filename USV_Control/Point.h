#ifndef POINT_H
#define POINT_H

#include <QObject>

class Point : public QObject {
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)

public:
    explicit Point(QObject *parent = nullptr);
    Point(double latitude, double longitude, QObject *parent = nullptr);

    double latitude() const;
    void setLatitude(double latitude);

    double longitude() const;
    void setLongitude(double longitude);

signals:
    void latitudeChanged();
    void longitudeChanged();

private:
    double m_latitude;
    double m_longitude;
};

#endif // POINT_H
