#pragma once

#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QObject>

class QNetworkAccessManager;
class QTcpServer;

class MapTileProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)
    Q_PROPERTY(QString tileDirectory READ tileDirectory CONSTANT)
    Q_PROPERTY(bool tilesPresent READ tilesPresent NOTIFY customTilesChanged)
    Q_PROPERTY(int maxZoomLevel READ maxZoomLevel NOTIFY customTilesChanged)
    Q_PROPERTY(QGeoRectangle tileArea READ tileArea NOTIFY customTilesChanged)
  public:
    MapTileProvider(QObject* parent = nullptr);
    ~MapTileProvider();

    QString address() const;
    QString cacheDirectory() const;
    int maxZoomLevel() const;
    QGeoRectangle tileArea() const;
    QString tileDirectory() const;
    bool tilesPresent() const;

    Q_INVOKABLE void clearCache();
    Q_INVOKABLE void reloadCustomTiles();

  signals:
    void customTilesChanged();
    void cacheCleared();

  private slots:
    void _checkCustomTiles();
    void _handleConnection();
    void _handleRequest();

  private:
    QGeoCoordinate _slippyToCoordinate(const int z, const int x, const int y) const;

  private:
    QGeoRectangle _tileArea;
    bool _tilesPresent = false;
    int _maxZoomLevel = 19;
    QTcpServer* _server = nullptr;
    QNetworkAccessManager* _manager = nullptr;
    QByteArray _blankTileData;
    QString _tileDirectory = "/home/stuart/Desktop/MorenaTiles";
    QString _cacheDirectory;
    int _port = 12345;
};
