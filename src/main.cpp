#include <QGeoPositionInfoSource>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>

// #include "LinuxGeoPositionInfoSource.h"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    engine.addImportPath("qrc:/src/");
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

// #ifdef Q_OS_ANDROID
//     QGeoPositionInfoSource* source = QGeoPositionInfoSource::createDefaultSource(nullptr);
// #else
//     QGeoPositionInfoSource* source = new LinuxGeoPositionInfoSource(nullptr);
// #endif
//     if (source) {
//         qDebug() << source->availableSources();
//         qDebug() << source->sourceName();
//         qDebug() << source->supportedPositioningMethods();
//         qDebug() << source->preferredPositioningMethods();
//         source->setUpdateInterval(60000);
//         source->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);
//         QObject::connect(
//             source, QOverload<QGeoPositionInfoSource::Error>::of(&QGeoPositionInfoSource::error),
//             source, [](QGeoPositionInfoSource::Error error) { qDebug() << error; });
//         QObject::connect(source, &QGeoPositionInfoSource::positionUpdated, source,
//                          [](const QGeoPositionInfo& update) { qDebug() << update; });
//         source->stopUpdates();
//         source->startUpdates();
//     }

    engine.load(url);

    return app.exec();
}
