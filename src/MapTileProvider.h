#pragma once

#include <QObject>

class QNetworkAccessManager;
class QTcpServer;

class MapTileProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)
    Q_PROPERTY(QString tileDirectory READ tileDirectory CONSTANT)
  public:
    MapTileProvider(QObject* parent = nullptr);
    ~MapTileProvider();

    QString address() const;
    QString cacheDirectory() const;
    QString tileDirectory() const;

    Q_INVOKABLE void clearCache();

  private slots:
    void _handleConnection();
    void _handleRequest();

  private:
    QTcpServer* _server;
    QNetworkAccessManager* _manager;
    QByteArray _blankTileData;
    QString _tileDirectory = "/home/stuart/Desktop/MorenaTiles";
    QString _cacheDirectory = "/home/stuart/Desktop/cache/customTiles";
    int _port = 12345;
};
