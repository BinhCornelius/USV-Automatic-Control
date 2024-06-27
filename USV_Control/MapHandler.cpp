#include "maphandler.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QDebug>

MapHandler::MapHandler(QObject *parent) : QObject(parent), udpSocket(new QUdpSocket(this))
{
    udpSocket->bind(12345, QUdpSocket::ShareAddress);
    connect(udpSocket, &QUdpSocket::readyRead, this, &MapHandler::processPendingDatagrams);
}

void MapHandler::addCoordinate(double latitude, double longitude)
{
    coordinates.append(QGeoCoordinate(latitude, longitude));
}

void MapHandler::sendCoordinates(double pathType, double rX, double rY, double omgX, double omgY)
{
    // if (coordinates.size() != 5) {
    //     qWarning() << "Not enough or too many coordinates selected!";
    //     return;
    // }

    QString coordinateString = "! ";

    coordinateString += QString::number(pathType, 'f', 1) + " ";
    coordinateString += " " + QString::number(coordinates[0].latitude(), 'f', 15) + " " + QString::number(coordinates[0].longitude(), 'f', 15) + " ";

    // for (int i = 0; i < 4; ++i) {
    //     coordinateString += QString::number(coordinates[i].latitude(), 'f', 15) + " " + QString::number(coordinates[i].longitude(), 'f', 15) + " ";
    // }

    coordinateString += " " + QString::number(coordinates[1].latitude(), 'f', 15) + " " + QString::number(coordinates[1].longitude(), 'f', 15) + " ";
    coordinateString += QString::number(rX, 'f', 2) + " ";
    coordinateString += QString::number(rY, 'f', 2) + " ";
    coordinateString += QString::number(omgX, 'f', 3) + " ";
    coordinateString += QString::number(omgY, 'f', 3);

    QByteArray datagram = coordinateString.toUtf8();
    udpSocket->writeDatagram(datagram, QHostAddress("192.168.41.100"), 4210); // Địa chỉ IP và cổng của ESP

    coordinates.clear(); // Xóa tọa độ sau khi gửi
    qDebug() << datagram;
}


void MapHandler::sendPwm(int pwmValue1, int pwmValue2)
{
    QString message = QString("PWM:%1,%2").arg(pwmValue1).arg(pwmValue2);
    QByteArray datagram_2 = message.toUtf8();
    udpSocket->writeDatagram(datagram_2, QHostAddress("192.168.41.100"), 4210); // Địa chỉ IP và cổng của ESP
    qDebug() << datagram_2;
}

// void MapHandler::requestGpsCoordinate()
// {
//     // Gửi yêu cầu nhận toạ độ GPS từ ESP8266
//     QByteArray datagram_3 = "request_gps";
//     udpSocket->writeDatagram(datagram_3, QHostAddress("192.168.41.100"), 4210);
// }


void MapHandler::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QByteArray data = datagram.data();
        QString gpsData(data);
        QStringList parts = gpsData.split(',');

        if (parts.size() == 2) {
            bool ok1, ok2;
            double lat = parts[0].toDouble(&ok1);
            double lng = parts[1].toDouble(&ok2);

            if (ok1 && ok2) {
                gpsCoordinate.setLatitude(lat);
                gpsCoordinate.setLongitude(lng);
                emit gpsCoordinateChanged();

            }
        }
    }
}
