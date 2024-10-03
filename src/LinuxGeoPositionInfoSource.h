#pragma once

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class LinuxGeoPositionInfoSource : public QGeoPositionInfoSource {
  public:
    LinuxGeoPositionInfoSource(QObject* parent);
    ~LinuxGeoPositionInfoSource();
    void setUpdateInterval(int msec);
    void setPreferredPositioningMethods(PositioningMethods methods);
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const;
    PositioningMethods supportedPositioningMethods() const;
    int minimumUpdateInterval() const;
    Error error() const;

  public slots:
    void startUpdates();
    void stopUpdates();
    void requestUpdate(int timeout);

  private slots:
    void _handleReply(QNetworkReply* reply);

  private:
    QNetworkAccessManager* _network;
    QTimer* _timer;
    QGeoPositionInfo _info;
    QGeoPositionInfoSource::Error _error;
};
