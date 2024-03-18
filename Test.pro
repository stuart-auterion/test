QT += quick
CONFIG += c++11


QML_IMPORT_PATH +=  \
    $$PWD/src \

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/videoManager.cpp \
    external/libklv/src/Klv.cpp \
    external/libklv/src/KlvParser.cpp \

INCLUDEPATH += \
    inc \
    external \
    external/libklv/include \

HEADERS += \
    inc/videoManager.h \
    inc/QObjectList.h \
    external/libklv/include/Klv.h \
    external/libklv/include/KlvFormatException.hpp \
    external/libklv/include/KlvParser.hpp \

RESOURCES += \
    resources.qrc


# Additional import path used to resolve QML modules in Qt Creator's code model

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

linux {
    linux-g++ | linux-g++-64 | linux-g++-32 | linux-clang {
        CONFIG += LinuxBuild
    } else : android-clang {
        CONFIG += AndroidBuild
    }
}

# VIDEO
LinuxBuild {
    QT += x11extras waylandclient gui-private
    CONFIG += link_pkgconfig
    packagesExist(gstreamer-1.0) {
        PKGCONFIG += gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0 gstreamer-gl-1.0 gstreamer-codecparsers-1.0 gstreamer-audio-1.0 gstreamer-tag-1.0 gstreamer-bad-audio-1.0 gstreamer-plugins-base-1.0
        CONFIG += VideoEnabled
    }
    DEFINES += HAVE_QT_X11 HAVE_QT_EGLFS HAVE_QT_WAYLAND
}
else:AndroidBuild {
    QMAKE_CXX_FLAGS += -lstdc++fs -std=c++17
    DEFINES += HAVE_QT_ANDROID
    contains(QT_ARCH, .*arm64.*) {
        GST_ROOT = $$PWD/external/gstreamer-1.0-android-universal-1.14.4/arm64
    } else:contains(QT_ARCH, .*arm.*) {
        GST_ROOT = $$PWD/external/gstreamer-1.0-android-universal-1.14.4/armv7
    } else {
        GST_ROOT = $$PWD/external/gstreamer-1.0-android-universal-1.14.4/x86
    }
    exists($$GST_ROOT) {
        QMAKE_CXXFLAGS += -pthread
        # We want to link these plugins statically
        LIBS += -L$$GST_ROOT/lib/gstreamer-1.0 \
            -lgstvideo-1.0 \
            -lgstcoreelements \
            -lgstplayback \
            -lgstudp \
            -lgstrtp \
            -lgstrtsp \
            -lgstx264 \
            -lgstlibav \
            -lgstsdpelem \
            -lgstvideoparsersbad \
            -lgstrtpmanager \
            -lgstisomp4 \
            -lgstmatroska \
            -lgstmpegtsdemux \
            -lgstandroidmedia \
            -lgstvideotestsrc \
            -lgstopengl

        # Rest of GStreamer dependencies
        LIBS += -L$$GST_ROOT/lib \
            -lgraphene-1.0 -ljpeg -lpng16 \
            -lgstmpegts-1.0 \
            -lgstfft-1.0 -lm  \
            -lgstnet-1.0 -lgio-2.0 \
            -lgstphotography-1.0 -lgstgl-1.0 -lEGL \
            -lgstaudio-1.0 -lgstcodecparsers-1.0 -lgstbase-1.0 \
            -lgstreamer-1.0 -lgstrtp-1.0 -lgstpbutils-1.0 -lgstrtsp-1.0 -lgsttag-1.0 \
            -lgstvideo-1.0 -lavformat -lavcodec -lavutil -lx264 -lavfilter -lswresample \
            -lgstriff-1.0 -lgstcontroller-1.0 -lgstapp-1.0 \
            -lgstsdp-1.0 -lbz2 -lgobject-2.0 \
            -Wl,--export-dynamic -lgmodule-2.0 -pthread -lglib-2.0 -lorc-0.4 -liconv -lffi -lintl \

        INCLUDEPATH += \
            $$GST_ROOT/include/gstreamer-1.0 \
            $$GST_ROOT/lib/gstreamer-1.0/include \
            $$GST_ROOT/include/glib-2.0 \
            $$GST_ROOT/lib/glib-2.0/include
    }
}
SOURCES += \
    external/gst-plugins-good/ext/qt/gstplugin.cc \
    external/gst-plugins-good/ext/qt/gstqtglutility.cc \
    external/gst-plugins-good/ext/qt/gstqsgtexture.cc \
    external/gst-plugins-good/ext/qt/gstqtsink.cc \
    external/gst-plugins-good/ext/qt/gstqtsrc.cc \
    external/gst-plugins-good/ext/qt/qtwindow.cc \
    external/gst-plugins-good/ext/qt/qtitem.cc

HEADERS += \
    external/gst-plugins-good/ext/qt/gstqsgtexture.h \
    external/gst-plugins-good/ext/qt/gstqtgl.h \
    external/gst-plugins-good/ext/qt/gstqtglutility.h \
    external/gst-plugins-good/ext/qt/gstqtsink.h \
    external/gst-plugins-good/ext/qt/gstqtsrc.h \
    external/gst-plugins-good/ext/qt/qtwindow.h \
    external/gst-plugins-good/ext/qt/qtitem.h

INCLUDEPATH += \
    /usr/include/gstreamer-1.0/ \
    /usr/include/glib-2.0/ \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include/ \

SOURCES += \
    external/gsttsdemux/gst/mpegtsdemux/gsttsdemux.c \
    external/gsttsdemux/gst/mpegtsdemux/mpegtsbase.c \
    external/gsttsdemux/gst/mpegtsdemux/mpegtspacketizer.c \
    external/gsttsdemux/gst/mpegtsdemux/mpegtsparse.c \
    external/gsttsdemux/gst/mpegtsdemux/pesparse.c \
    external/gsttsdemux/gst/mpegtsdemux/tsdemux.c \
    external/gsttsdemux/gst-libs/gst/gst-atsc-section.c \
    external/gsttsdemux/gst-libs/gst/gst-dvb-descriptor.c \
    external/gsttsdemux/gst-libs/gst/gst-dvb-section.c \
    external/gsttsdemux/gst-libs/gst/gstmpegtsdescriptor.c \
    external/gsttsdemux/gst-libs/gst/gstmpegtssection.c \

DEFINES += GETTEXT_PACKAGE

INCLUDEPATH += external/gsttsdemux

HEADERS += \
    external/gsttsdemux/gst/gst-i18n-plugin.h \
    external/gsttsdemux/gst/mpegtsdemux/gstmpegdefs.h \
    external/gsttsdemux/gst/mpegtsdemux/gstmpegdesc.h \
    external/gsttsdemux/gst/mpegtsdemux/mpegtsbase.h \
    external/gsttsdemux/gst/mpegtsdemux/mpegtspacketizer.h \
    external/gsttsdemux/gst/mpegtsdemux/mpegtsparse.h \
    external/gsttsdemux/gst/mpegtsdemux/pesparse.h \
    external/gsttsdemux/gst/mpegtsdemux/tsdemux.h \
    external/gsttsdemux/gst-libs/gst/gst-atsc-section.h \
    external/gsttsdemux/gst-libs/gst/gst-dvb-descriptor.h \
    external/gsttsdemux/gst-libs/gst/gst-dvb-section.h \
    external/gsttsdemux/gst-libs/gst/gstmpegtsdescriptor.h \
    external/gsttsdemux/gst-libs/gst/mpegts-private.h \
    external/gsttsdemux/gst-libs/gst/gstmpegtssection.h \
    external/gsttsdemux/gst-libs/gst/mpegts.h \
