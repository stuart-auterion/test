#include "MapTileProvider.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

MapTileProvider::MapTileProvider(QObject* parent) : QObject(parent) {
    /* Initialize */
    _cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    qDebug() << "Cache at" << _cacheDirectory;
    QFile thisFile(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + __FILE__));
    _tileDirectory = QDir::cleanPath(QFileInfo(thisFile).path() + "/../tiles");
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
        qDebug() << "Error starting map tile provider";
    } else {
        _port= _server->serverPort();
        qDebug() << QString("Tile server running locally on port %1").arg(_port);
    }
    QObject::connect(_server, &QTcpServer::newConnection, this,
                     &MapTileProvider::_handleConnection);
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

QString MapTileProvider::tileDirectory() const {
    return _tileDirectory;
}

void MapTileProvider::clearCache() {
    qDebug() << "Cleared cache" << _cacheDirectory;
    QString path = QDir::cleanPath(_cacheDirectory);
    QDir(path).removeRecursively();
    QDir().mkdir(path);
}

void MapTileProvider::_handleRequest() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!client) {
        return;
    }
    bool success = false;
    QByteArray data;
    /* Look up tile path ("slippy" format) */
    const QString request = QUrl::fromPercentEncoding(client->readLine());
    static const QRegularExpression regex("\\/\\d+\\/\\d+\\/\\d+\\.\\w+");
    QRegularExpressionMatch match = regex.match(request);
    if (match.hasMatch()) {
        /* Read file */
        QFile file(QDir::cleanPath(_tileDirectory + "/" + match.captured(0)));
        if (file.open(QFile::ReadOnly)) {
            data = file.readAll();
            if (!data.isEmpty()) {
                success = true;
                qDebug().noquote() << QString("Serving tile %1").arg(file.fileName());
            }
            file.close();
        }
    }
    if (!success) {
        qDebug().noquote() << QString("Serving blank tile for %1")
                                  .arg(match.hasMatch() ? match.captured(0) : "unknown");
        data = _blankTileData;
    }
    /* Write header */
    QByteArray header;
    header.append("HTTP/1.1 200 OK\r\n");
    header.append("Content-Type: image/png\r\n");
    header.append("\r\n");
    /* Send data */
    client->write(header + data);
    client->close();
}
