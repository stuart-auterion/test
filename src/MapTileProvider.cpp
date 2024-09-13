#include "MapTileProvider.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHostAddress>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QtMath>

#include "qjsonobject.h"

MapTileProvider::MapTileProvider(QObject* parent) : QObject(parent) {
    /* Initialize */
    _cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    qDebug() << "Cache at" << _cacheDirectory;
    QFile thisFile(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + __FILE__));
    // _tileDirectory = QDir::cleanPath(QFileInfo(thisFile).path() + "/../tiles");
    _tileDirectory = "/home/stuart/Apps/MapTilesDownloader/src/output/1726242151296";
    qDebug() << "Tiles at" << _tileDirectory;
    /* Register QML */
    qmlRegisterSingletonType<MapTileProvider>(
        "Map", 1, 0, "MapTileProvider",
        [this](QQmlEngine*, QJSEngine*) -> QObject* { return this; });
    /* Set up cache */
    QDir().mkdir(_cacheDirectory);
    /* Read in blank tile data */
    QFile file(":/img/blankTile.png");
    if (file.open(QFile::ReadOnly)) {
        _blankTileData = file.readAll();
        file.close();
    }
    /* Set up server */
    _server = new QTcpServer(this);
    if (!_server->listen(QHostAddress::LocalHost, 0)) {
        qDebug().noquote() << "Error starting map tile provider";
    } else {
        _port = _server->serverPort();
        qDebug().noquote() << QString("Tile server running locally on port %1").arg(_port);
    }
    QObject::connect(_server, &QTcpServer::newConnection, this,
                     &MapTileProvider::_handleConnection);
    /* Get custom tiles info */
    _checkCustomTiles();
}

void MapTileProvider::_handleConnection() {
    QTcpSocket* client = _server->nextPendingConnection();
    QObject::connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);
    QObject::connect(client, &QTcpSocket::readyRead, this, &MapTileProvider::_handleRequest);
}

MapTileProvider::~MapTileProvider() {}

QString MapTileProvider::address() const {
    QUrl address;
    address.setScheme("http");
    address.setHost(QHostAddress(QHostAddress::LocalHost).toString());
    address.setPort(_port);
    /* Trailing slash must be present */
    return address.toString() + "/";
}

QString MapTileProvider::cacheDirectory() const {
    return _cacheDirectory;
}

int MapTileProvider::maxZoomLevel() const {
    return _maxZoomLevel;
}

QGeoRectangle MapTileProvider::tileArea() const {
    return _tileArea;
}

QString MapTileProvider::tileDirectory() const {
    return _tileDirectory;
}

bool MapTileProvider::tilesPresent() const {
    return _tilesPresent;
}

void MapTileProvider::clearCache() {
    qDebug().noquote() << "Cleared cache" << _cacheDirectory;
    QString path = QDir::cleanPath(_cacheDirectory);
    QDir(path).removeRecursively();
    QDir().mkdir(path);
    emit cacheCleared();
    emit reloadCustomTiles();
}

void MapTileProvider::reloadCustomTiles() {
    _checkCustomTiles();
}

void MapTileProvider::_checkCustomTiles() {
    _maxZoomLevel = -1;
    QDirIterator zoomDirIt(_tileDirectory);
    while (zoomDirIt.hasNext()) {
        QDir dir = zoomDirIt.next();
        bool success = false;
        const int z = dir.dirName().toInt(&success);
        if (success) {
            _maxZoomLevel = qMax(_maxZoomLevel, z);
        }
    }
    _tilesPresent = (_maxZoomLevel > -1);
    QDirIterator xDirIt(QDir::cleanPath(_tileDirectory + "/" + QString::number(_maxZoomLevel)));
    int minX = INT_MAX;
    int maxX = INT_MIN;
    while (xDirIt.hasNext()) {
        QDir dir = xDirIt.next();
        bool success = false;
        const int x = dir.dirName().toInt(&success);
        if (success) {
            minX = qMin(minX, x);
            maxX = qMax(maxX, x);
        }
    }
    QDirIterator yDirIt(QDir::cleanPath(_tileDirectory + "/" + QString::number(_maxZoomLevel) +
                                        "/" + QString::number(maxX)));
    int minY = INT_MAX;
    int maxY = INT_MIN;
    while (yDirIt.hasNext()) {
        QFileInfo file(yDirIt.next());
        bool success = false;
        const int y = file.baseName().toInt(&success);
        if (success) {
            minY = qMin(minY, y);
            maxY = qMax(maxY, y);
        }
    }
    QList<QGeoCoordinate> coordinates = {_slippyToCoordinate(_maxZoomLevel, minX, minY),
                                         _slippyToCoordinate(_maxZoomLevel, maxX, maxY)};
    _tileArea = QGeoRectangle(coordinates);
    emit customTilesChanged();
}

void MapTileProvider::_handleRequest() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!client) {
        return;
    }
    /* Interpret request */
    const QString request = QUrl::fromPercentEncoding(client->readLine());
    static const QRegularExpression tileRegex("\\/\\d+\\/\\d+\\/\\d+\\.\\w+");
    QRegularExpressionMatch tileMatch = tileRegex.match(request);
    static const QRegularExpression providerRegex("\\/[a-z]+");
    QRegularExpressionMatch providerMatch = providerRegex.match(request);
    /* Initialize data */
    QByteArray data;
    QByteArray header;
    header.append("HTTP/1.1 200 OK\r\n");
    /* Tile request */
    if (tileMatch.hasMatch()) {
        /* Read file */
        bool success = false;
        QFile file(QDir::cleanPath(_tileDirectory + "/" + tileMatch.captured(0)));
        if (file.open(QFile::ReadOnly)) {
            data = file.readAll();
            if (!data.isEmpty()) {
                success = true;
                qDebug().noquote() << QString("Serving tile %1").arg(file.fileName());
            }
            file.close();
        }
        if (!success) {
            qDebug().noquote() << QString("Serving blank tile for %1")
                                      .arg(tileMatch.hasMatch() ? tileMatch.captured(0)
                                                                : "unknown");
            data = _blankTileData;
            header.append("Content-Type: image/png\r\n");
        }
    }
    /* Provider info request */
    else if (providerMatch.hasMatch()) {
        const QString type =
            providerMatch.hasMatch() ? providerMatch.captured(0).replace("/", "") : "unknown";
        qDebug().noquote() << QString("Serving provider info for map type: %1").arg(type);
        QJsonObject json;
        json["Enabled"] = "true";
        json["UrlTemplate"] = address() + "%z/%x/%y.png";
        json["ImageFormat"] = "png";
        json["MaximumZoomLevel"] = _maxZoomLevel;
        json["ID"] = QString("custom-%1").arg(type);
        json["MapCopyRight"] = "";
        json["DataCopyRight"] = "";
        data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    }
    /* Unknown request */
    else {
        client->close();
        return;
    }
    /* Send data */
    header.append("\r\n");
    client->write(header + data);
    client->close();
}

QGeoCoordinate MapTileProvider::_slippyToCoordinate(const int z, const int x, const int y) const {
    /* https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Pseudo-code */
    const int n = std::pow(2, z);
    const double longitude = static_cast<double>(x) / static_cast<double>(n) * 360.0 - 180.0;
    const double latitude = qRadiansToDegrees(std::atan(
        std::sinh(M_PI * (1.0 - (static_cast<double>(2 * y) / static_cast<double>((n)))))));
    return QGeoCoordinate(latitude, longitude);
}
