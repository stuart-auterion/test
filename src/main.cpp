
// #include <gst/gst.h>
#include <zlib.h>

#include <QDataStream>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "microtar.h"
// #include "videoManager.h"

// Extracts a .tar.gz file
bool extract(const QString& inputFile, const QString& outputDirectory) {
    /* Make sure input file exists */
    QFile file(inputFile);
    if (!file.open(QFile::ReadOnly)) {
        qDebug().noquote() << QString("Unable to open %1").arg(inputFile);
        return false;
    }
    /* Create a temporary file for holding the uncompressed data */
    QTemporaryFile temp;
    if (!temp.open()) {
        qDebug().noquote() << QString("Unable to create temporary file %1").arg(temp.fileName());
        return false;
    }
    /* Initialize zlib */
    z_stream gzipData;
    gzipData.zalloc = nullptr;
    gzipData.zfree = nullptr;
    gzipData.opaque = nullptr;
    gzipData.avail_in = 0;
    gzipData.next_in = nullptr;
    int result;
    constexpr int bufferSize = 1024 * 20;
    unsigned char inputBuffer[bufferSize];
    unsigned char outputBuffer[bufferSize];
    result = inflateInit2(&gzipData, 16 + MAX_WBITS);
    if (result != Z_OK) {
        qDebug().noquote() << "Unable to initialize zlib";
        return false;
    }
    /* Un-gzip the file using zlib */
    do {
        gzipData.avail_in = static_cast<unsigned>(file.read((char*)inputBuffer, bufferSize));
        if (gzipData.avail_in == 0) {
            break;
        }
        gzipData.next_in = inputBuffer;
        do {
            gzipData.avail_out = bufferSize;
            gzipData.next_out = outputBuffer;
            result = inflate(&gzipData, Z_NO_FLUSH);
            if (result != Z_OK && result != Z_STREAM_END) {
                qDebug().noquote()
                    << QString("Unable to extract data with zlib (error %1)").arg(result);
                return false;
            }
            uint bytesInflated = bufferSize - gzipData.avail_out;
            qint64 bytesWritten = temp.write((char*)outputBuffer, static_cast<int>(bytesInflated));
            if (bytesWritten != bytesInflated) {
                qDebug().noquote()
                    << QString("Unable to write extracted data to temporary file %1 (error %2)")
                           .arg(temp.fileName(), temp.errorString());
                return false;
            }
        } while (gzipData.avail_out == 0);
    } while (result != Z_STREAM_END);
    temp.close();
    /* Make output directory */
    QDir().mkdir(outputDirectory);
    if (!QDir(outputDirectory).exists()) {
        qDebug().noquote() << QString("Unable to create output directory %1").arg(outputDirectory);
        return false;
    }
    /* Use microtar to detect files in the extracted tarball */
    mtar_t tarData;
    mtar_header_t tarHeaderData;
    QList<QString> files;
    if (mtar_open(&tarData, temp.fileName().toLocal8Bit(), "r") != MTAR_ESUCCESS) {
        qDebug().noquote() << QString("Unable to un-tar temporary file %1 created from %2")
                                  .arg(temp.fileName(), inputFile);
        return false;
    }
    while ((mtar_read_header(&tarData, &tarHeaderData)) != MTAR_ENULLRECORD) {
        files.append(tarHeaderData.name);
        mtar_next(&tarData);
    }
    /* For each file in the tarball, extract its data and write to a file in the output directory */
    for (const QString& file : files) {
        if (mtar_find(&tarData, file.toLocal8Bit(), &tarHeaderData) != MTAR_ESUCCESS) {
            continue;
        }
        QFile outputFile(outputDirectory + QDir::separator() + file);
        if (!outputFile.open(QFile::WriteOnly)) {
            qDebug().noquote()
                << QString("Unable to create output file %1").arg(outputFile.fileName());
            return false;
        }
        uint totalBytesRead = 0;
        while (totalBytesRead < tarHeaderData.size) {
            const uint bytesToRead = (totalBytesRead + bufferSize) > tarHeaderData.size
                                         ? (tarHeaderData.size - totalBytesRead)
                                         : bufferSize;
            result = mtar_read_data(&tarData, inputBuffer, bytesToRead);
            if (result != MTAR_ESUCCESS) {
                qDebug().noquote()
                    << QString("Unable to read data from compressed file %1 inside %2 (error %3)")
                           .arg(file, inputFile)
                           .arg(result);
            }
            totalBytesRead += bufferSize;
            if (outputFile.write(reinterpret_cast<const char*>(inputBuffer), bytesToRead) == -1) {
                qDebug().noquote()
                    << QString("Unable to write data to output file %1").arg(outputFile.fileName());
            }
        }
        outputFile.close();
    }
    return true;
}

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
    QtAndroid::requestPermissionsSync(
        {"android.permission.WRITE_EXTERNAL_STORAGE", "android.permission.READ_EXTERNAL_STORAGE"});
#endif

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
    //    initVideo();

    QElapsedTimer timer;
    timer.start();
//    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
//    extract(QDir::cleanPath(documents + "/Auterion Mission Control/Plugins/ccast.amcplugin"),
//            QDir::cleanPath(documents + "/Auterion Mission Control/Plugins/ccast"));
        QString documents = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        extract(QDir::cleanPath(documents + "/ccast.amcplugin"), QDir::cleanPath(documents +
        "/ccast"));
    qDebug() << "Extraction took" << timer.elapsed();

    engine.load(url);

    return app.exec();
}
