import QtQuick 2.15

import org.freedesktop.gstreamer.GLVideoItem 1.0
import Video 1.0

Item {
    property alias autoplay: videoStream.autoplay
    property alias uri: videoStream.uri
    property alias type: videoStream.type
    VideoStream {
        id: videoStream
        autoplay: true
        gstVideoItem: gstGLVideoItem
    }
    GstGLVideoItem {
        id: gstGLVideoItem
        anchors.fill: parent
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "black"
        color: "transparent"
    }
}
