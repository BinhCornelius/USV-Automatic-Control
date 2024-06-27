#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QGeoCoordinate>
#include <QList>

class MapHandler : public QObject
{
    Q_OBJECT
public:
    explicit MapHandler(QObject *parent = nullptr);
    Q_INVOKABLE void addCoordinate(double latitude, double longitude);
    Q_INVOKABLE void sendCoordinates(double pathType, double rX, double rY, double omgX, double omgY);
    //Q_INVOKABLE void requestGpsCoordinate();
    Q_INVOKABLE void sendPwm(int pwmValue1, int pwmValue2);

signals:
    void gpsCoordinateChanged();

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    QList<QGeoCoordinate> coordinates;
    QGeoCoordinate gpsCoordinate;
};

#endif // MAPHANDLER_H
