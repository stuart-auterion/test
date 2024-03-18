#pragma once

#include <gst/gst.h>

#include <QObject>
#include <QTimer>
#include <QUuid>

class VideoStream : public QObject {
    Q_OBJECT
    /* clang-format off */
    Q_PROPERTY(bool autoplay READ autoplay WRITE setAutoplay NOTIFY autoplayChanged)
    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QObject* gstVideoItem READ gstVideoItem WRITE setGstVideoItem NOTIFY gstVideoItemChanged)
    /* clang-format on */
  public:
    VideoStream(QObject* parent = nullptr);
    void createPipeline();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    // Enumerations
    enum Type { RTPUDP, RTSP, TEST, KLV_ENCODE, KLV_DECODE, INVALID };
    Q_ENUM(Type)
    // Set functions
    void setGstVideoItem(QObject* newGstVideoItem);
    void setType(Type newType);
    void setUri(const QString& newUri);
    // Get functions
    QObject* gstVideoItem() const;
    Type type() const;
    QString uri() const;

    bool autoplay() const;
    void setAutoplay(bool newAutoplay);

  signals:
    void autoplayChanged();
    void gstVideoItemChanged();
    void uriChanged();
    void typeChanged();

  private:
    void _insertKlv();
    static GstFlowReturn _decodeKlvCallback(GstElement *appsink, gpointer data);
    void _decodeKlv(GstSample* sample);

    bool _autoplay;
    QObject* _gstVideoItem = nullptr;
    GstElement* _pipeline = nullptr;
    QTimer _timer;
    Type _type = INVALID;
    QString _uri;
    QTimer _klvTimer;
};

Q_DECLARE_METATYPE(VideoStream*)

void initVideo();
