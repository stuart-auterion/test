#include "LinuxGeoPositionInfoSource.h"

#include <QGeoPositionInfoSourceFactory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

LinuxGeoPositionInfoSource::LinuxGeoPositionInfoSource(QObject* parent)
    : QGeoPositionInfoSource(parent) {
    _network = new QNetworkAccessManager(this);
    _timer = new QTimer(this);
    _timer->setInterval(60000);
    connect(_timer, &QTimer::timeout, this, [this]() { requestUpdate(-1); });
    connect(_network, &QNetworkAccessManager::finished, this,
            &LinuxGeoPositionInfoSource::_handleReply);
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for (int nIter = 0; nIter < list.count(); nIter++)

    {
        if (!list[nIter].isLoopback())
            if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol)
                qDebug() << list[nIter].toString();
    }
}

LinuxGeoPositionInfoSource::~LinuxGeoPositionInfoSource() {}

void LinuxGeoPositionInfoSource::setUpdateInterval(int msec) {
    msec = qMax(minimumUpdateInterval(), msec);
    _timer->setInterval(msec);
}

void LinuxGeoPositionInfoSource::setPreferredPositioningMethods(PositioningMethods methods) {
    Q_UNUSED(methods)
}

QGeoPositionInfo LinuxGeoPositionInfoSource::lastKnownPosition(
    bool fromSatellitePositioningMethodsOnly) const {
    Q_UNUSED(fromSatellitePositioningMethodsOnly)
    return _info;
}

QGeoPositionInfoSource::PositioningMethods LinuxGeoPositionInfoSource::supportedPositioningMethods()
    const {
    return QGeoPositionInfoSource::AllPositioningMethods;
}

int LinuxGeoPositionInfoSource::minimumUpdateInterval() const {
    // https://members.ip-api.com/
    // Documentation says limit is 45 requests/minute
    // We'll double that as a safety margin
    constexpr int minimumUpdateInterval = static_cast<int>(45.0f / 60.0f * 1000.0f * 2.0f);
    return minimumUpdateInterval;
}

QGeoPositionInfoSource::Error LinuxGeoPositionInfoSource::error() const {
    return _error;
}

void LinuxGeoPositionInfoSource::startUpdates() {
    requestUpdate(-1);
    _timer->start();
}

void LinuxGeoPositionInfoSource::stopUpdates() {
    _timer->stop();
}

void LinuxGeoPositionInfoSource::requestUpdate(int timeout) {
    timeout = qMax(minimumUpdateInterval() / 2, timeout);
    QNetworkRequest request;
    request.setUrl(QUrl("http://ip-api.com/json/"));
    request.setTransferTimeout(timeout);
    _network->get(request);
}

void LinuxGeoPositionInfoSource::_handleReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        _error = QGeoPositionInfoSource::Error::AccessError;
        emit QGeoPositionInfoSource::error(_error);
        if (reply->error() == QNetworkReply::TimeoutError) {
            emit updateTimeout();
        }
    } else {
        _error = QGeoPositionInfoSource::Error::NoError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
        if (!jsonDoc.isNull() && !jsonDoc.isEmpty()) {
            QGeoCoordinate coordinate(jsonDoc.object()["lat"].toDouble(),
                                      jsonDoc.object()["lon"].toDouble());
            if (coordinate.isValid()) {
                _info.setCoordinate(coordinate);
                _info.setTimestamp(QDateTime::currentDateTime());
                emit positionUpdated(_info);
            }
        }
    }
    reply->deleteLater();
}
