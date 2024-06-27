#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "MapHandler.h"
#include "Point.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    qmlRegisterType<Point>("com.example", 1, 0, "Point");

    QQmlApplicationEngine engine;
    MapHandler mapHandler;

    engine.rootContext()->setContextProperty("mapHandler", &mapHandler);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
