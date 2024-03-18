#include "videoManager.h"

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <unistd.h>

#include <QDebug>
#include <QObject>
#include <QQmlEngine>
#include <QUuid>
#include <QtEndian>
#include <QtGlobal>

#include "libklv/include/Klv.h"
#include "libklv/include/KlvParser.hpp"

/******************************************************************************
 * GStreamer initialization
 *****************************************************************************/
static bool initialized = false;

G_BEGIN_DECLS
// The static plugins we use
#ifdef Q_OS_ANDROID
GST_PLUGIN_STATIC_DECLARE(coreelements);
GST_PLUGIN_STATIC_DECLARE(playback);
GST_PLUGIN_STATIC_DECLARE(libav);
GST_PLUGIN_STATIC_DECLARE(rtp);
GST_PLUGIN_STATIC_DECLARE(rtsp);
GST_PLUGIN_STATIC_DECLARE(udp);
GST_PLUGIN_STATIC_DECLARE(videoparsersbad);
GST_PLUGIN_STATIC_DECLARE(x264);
GST_PLUGIN_STATIC_DECLARE(rtpmanager);
GST_PLUGIN_STATIC_DECLARE(isomp4);
GST_PLUGIN_STATIC_DECLARE(matroska);
GST_PLUGIN_STATIC_DECLARE(mpegtsdemux);
GST_PLUGIN_STATIC_DECLARE(opengl);
GST_PLUGIN_STATIC_DECLARE(androidmedia);
GST_PLUGIN_STATIC_DECLARE(videotestsrc);
#endif
GST_PLUGIN_STATIC_DECLARE(qmlgl);
GST_PLUGIN_STATIC_DECLARE(mpegtsdemux);
G_END_DECLS

#ifdef Q_OS_ANDROID
#include <android/log.h>
static void gst_android_log(GstDebugCategory* category, GstDebugLevel level, const gchar* file,
                            const gchar* function, gint line, GObject* object,
                            GstDebugMessage* message, gpointer data) {
    if (level <= gst_debug_category_get_threshold(category)) {
        __android_log_print(ANDROID_LOG_ERROR, "GST", "%s, %s: %s", file, function,
                            gst_debug_message_get(message));
    }
    Q_UNUSED(line)
    Q_UNUSED(object)
    Q_UNUSED(data)
}
#endif

void initVideo() {
#ifdef Q_OS_ANDROID
    gst_debug_add_log_function(gst_android_log, nullptr, nullptr);
#endif
    gst_init(0, 0);
#ifdef Q_OS_ANDROID
    GST_PLUGIN_STATIC_REGISTER(coreelements);
    GST_PLUGIN_STATIC_REGISTER(playback);
    GST_PLUGIN_STATIC_REGISTER(libav);
    GST_PLUGIN_STATIC_REGISTER(rtp);
    GST_PLUGIN_STATIC_REGISTER(rtsp);
    GST_PLUGIN_STATIC_REGISTER(udp);
    GST_PLUGIN_STATIC_REGISTER(videoparsersbad);
    GST_PLUGIN_STATIC_REGISTER(x264);
    GST_PLUGIN_STATIC_REGISTER(rtpmanager);
    GST_PLUGIN_STATIC_REGISTER(isomp4);
    GST_PLUGIN_STATIC_REGISTER(matroska);
    GST_PLUGIN_STATIC_REGISTER(mpegtsdemux);
    GST_PLUGIN_STATIC_REGISTER(opengl);
    GST_PLUGIN_STATIC_REGISTER(androidmedia);
    GST_PLUGIN_STATIC_REGISTER(videotestsrc);
#endif
    GstElement* sink = gst_element_factory_make("qmlglsink", nullptr);
    if (sink == nullptr) {
        GST_PLUGIN_STATIC_REGISTER(qmlgl);
        sink = gst_element_factory_make("qmlglsink", nullptr);
        Q_UNUSED(sink)
    }
//    sink = gst_element_factory_make("mpegtscustom", nullptr);
//    if (sink == nullptr) {
//        GST_PLUGIN_STATIC_REGISTER(mpegtsdemux);
//        sink = gst_element_factory_make("mpegts", nullptr);
//        Q_UNUSED(sink)
//    }
    qmlRegisterType<VideoStream>("Video", 1, 0, "VideoStream");
    qRegisterMetaType<VideoStream*>();
    initialized = true;
}

/******************************************************************************
 * VideoStream class
 *****************************************************************************/
VideoStream::VideoStream(QObject* parent) : QObject(parent) {
    if (!initialized) {
        qDebug() << "not initialized!";
    }
    _timer.setInterval(50);
    connect(&_timer, &QTimer::timeout, this, &VideoStream::start);
}

void VideoStream::start() {
    if (_pipeline != nullptr) {
        GstState state;
        gst_element_get_state(_pipeline, &state, nullptr, 10);
        if (state != GST_STATE_PLAYING) {
            gst_element_set_state(_pipeline, GST_STATE_PLAYING);
        }
    }
}

void VideoStream::stop() {
    if (_pipeline != nullptr) {
        GstState state;
        gst_element_get_state(_pipeline, &state, nullptr, 10);
        if (state == GST_STATE_PLAYING) {
            gst_element_set_state(_pipeline, GST_STATE_PAUSED);
        }
    }
}

static void cb_new_pad(GstElement* element, GstPad* pad, gpointer data) {
    qDebug() << "New pad: " << gst_pad_get_name(pad);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    qDebug() << gst_caps_to_string(caps);
}

void VideoStream::createPipeline() {
    // If we're missing information
    if ((_gstVideoItem == nullptr) ||
        (_type == INVALID) /*|| (_uri.isEmpty() && (_type != TEST))*/) {
        return;
    }
    if (_pipeline != nullptr) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        _pipeline = nullptr;
    }
    QString pipelineString;
    switch (_type) {
        case KLV_ENCODE:
            pipelineString =
                QString(
                    /* Video source to tee */
                    "videotestsrc ! tee name=t "
                    /* Mux to network sink */
                    "rtpmux name=mux ! udpsink port=5000 host=%1 "
                    /* Tee'd video source to mux */
                    //                    "t. ! x264enc tune=zerolatency bitrate=1000
                    //                    speed-preset=superfast ! " "rtph264pay ! queue ! mux. "
                    /* KLV to Mux */
                    "appsrc name=klvsrc caps=meta/x-klv,parsed=true,spare=true,is-live=true ! "
                    "rtpklvpay ! queue ! mux. "
                    /* Tee'd video source to QML sink */
                    "t. ! glupload ! glcolorconvert ! qmlglsink name=qmlsink")
                    .arg(_uri);
            break;
        case KLV_DECODE:
            pipelineString = QString(
                "filesrc "
                "location=/home/stuart/Videos/video_h264_raw_hvalstad.ts ! "
                "tsdemux name=demux ! queue ! decodebin ! "
                "glupload ! glcolorconvert ! qmlglsink name=qmlsink "
                "demux. ! meta/x-klv ! queue ! appsink emit-signals=true name=klvsink"
            );
            break;
        case RTSP:
            pipelineString = QString(
                                 "rtspsrc location=rtsp://%1 ! rtpjitterbuffer ! parsebin ! queue  "
                                 "! decodebin ! glupload ! glcolorconvert ! qmlglsink name=qmlsink")
                                 .arg(_uri);
            break;
        case RTPUDP:
            // gst-launch-1.0 -v videotestsrc !
            // "video/x-raw,format=I420,width=1920,height=1080,framerate=60/1" ! x264enc
            // tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink port=5004
            // host=127.0.0.1
            pipelineString = QString(
                                 "udpsrc port=5004 ! application/x-rtp, media=(string)video, "
                                 "clock-rate=(int)90000, encoding-name=(string)H264, "
                                 "payload=(int)96 ! parsebin ! queue ! decodebin ! glupload ! "
                                 "glcolorconvert ! qmlglsink name=qmlsink")
                                 .arg(_uri);
            break;
        case TEST:
            pipelineString =
                QString("videotestsrc ! glupload ! glcolorconvert ! qmlglsink name=qmlsink");
            break;
        case INVALID:
        default:
            return;
    }
    qDebug() << pipelineString.toStdString().c_str();
    _pipeline = gst_pipeline_new(NULL);
    _pipeline = gst_parse_launch(pipelineString.toStdString().c_str(), NULL);
    GstElement* sink = gst_bin_get_by_name(GST_BIN(_pipeline), "qmlsink");
    g_object_set(sink, "widget", _gstVideoItem, NULL);
    if (_type == KLV_ENCODE) {
        _klvTimer.setInterval(1000);
        connect(&_klvTimer, &QTimer::timeout, this, &VideoStream::_insertKlv);
        _klvTimer.start();
    }
    if (_type == KLV_DECODE) {
        GstElement* element;
        element = gst_bin_get_by_name(GST_BIN(_pipeline), "demux");
        g_signal_connect(element, "pad-added", G_CALLBACK(cb_new_pad), NULL);
//        GstElement* klvsink = gst_bin_get_by_name(GST_BIN(_pipeline), "klvsink");
//        g_signal_connect(klvsink, "new-sample", G_CALLBACK(_decodeKlvCallback), this);
    }
}

bool VideoStream::autoplay() const {
    return _autoplay;
}

QObject* VideoStream::gstVideoItem() const {
    return _gstVideoItem;
}

VideoStream::Type VideoStream::type() const {
    return _type;
}

QString VideoStream::uri() const {
    return _uri;
}

void VideoStream::setAutoplay(bool newAutoplay) {
    if (_autoplay != newAutoplay) {
        _autoplay = newAutoplay;
        emit autoplayChanged();
    }
    if (_autoplay) {
        if (!_timer.isActive()) {
            _timer.start();
        }
    } else {
        _timer.stop();
    }
}

static uint8_t data[21] = {
    0x06, 0x0E, 0x2B, 0x34, 0x02, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Universal header */
    4,                                                                /* Total length of KLV */
    1,                                                                /* Key */
    2,                                                                /* Length */
    0x01, 0x02                                                        /* Value */
};
void VideoStream::_insertKlv() {
    GstFlowReturn ret = GST_FLOW_ERROR;
    GstElement* klvsrc = gst_bin_get_by_name(GST_BIN(_pipeline), "klvsrc");
    if (klvsrc) {
        size_t size = sizeof(data) / sizeof(data[0]);
        GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
        gst_buffer_fill(buffer, 0, data, size);
        ret = gst_app_src_push_buffer((GstAppSrc*)klvsrc, buffer);
    }
    if (ret != GST_FLOW_OK) {
        qDebug() << "KLV encode error";
    }
}

GstFlowReturn VideoStream::_decodeKlvCallback(GstElement* appsink, gpointer data) {
    VideoStream* parent = static_cast<VideoStream*>(data);
    if ((appsink == nullptr) || (parent == nullptr)) {
        return GST_FLOW_OK;
    }
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
    if (sample != nullptr) {
        parent->_decodeKlv(sample);
    }
    gst_sample_unref(sample);
    sample = nullptr;
    return GST_FLOW_OK;
}

void VideoStream::_decodeKlv(GstSample* sample) {
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (buffer == nullptr) {
        return;
    }
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        return;
    }
    KlvParser parser({KlvParser::KEY_ENCODING_16_BYTE, KlvParser::KEY_ENCODING_BER_OID});
    KLV* parsed_klv = NULL;
    for (gsize i = 0; i < map.size; i++) {
        parsed_klv = parser.parseByte(map.data[i]);
    }
    if (parsed_klv) {
        std::unordered_map<std::vector<uint8_t>, KLV> map = parsed_klv->indexToMap();
        for (std::pair<const std::vector<unsigned char>, KLV>& pair : map) {
            if (pair.first.empty() || (pair.first.size() > 1)) {
                continue;
            }
            uint8_t key = pair.first[0];
            QByteArray value(reinterpret_cast<const char*>(pair.second.getValue().data()),
                             pair.second.getValue().size());
            switch (key) {
                case 16:  // Horizontal field of view
                    qDebug() << "HFoV: "
                             << (qFromBigEndian<quint16>(value.data()) * 180.0f / 0xFFFF);
                    break;
                case 17:  // Vertical field of view
                    qDebug() << "VFoV: "
                             << (qFromBigEndian<quint16>(value.data()) * 180.0f / 0xFFFF);
                    break;
                case 56:  // Vertical field of view
                    qDebug() << "Ground speed: " << qFromBigEndian<quint8>(value.data());
                    break;
                default:
                    break;
            }
        }
    }
}

void VideoStream::setGstVideoItem(QObject* newGstVideoItem) {
    if (_gstVideoItem != newGstVideoItem) {
        _gstVideoItem = newGstVideoItem;
        emit gstVideoItemChanged();
        createPipeline();
    }
}

void VideoStream::setType(Type newType) {
    if (_type != newType) {
        _type = newType;
        emit typeChanged();
        createPipeline();
    }
}

void VideoStream::setUri(const QString& newUri) {
    if (_uri != newUri) {
        _uri = newUri;
        emit uriChanged();
        createPipeline();
    }
}
