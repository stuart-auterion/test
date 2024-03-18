import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt.labs.animation 1.0
import QtQml.Models 2.15
import QtGraphicalEffects 1.15

import Video 1.0

Window {
    id: root
    width: 810
    height: 480
    visible: true
    title: qsTr("Hello World")
    Flow {
        spacing: 1
        anchors.fill: parent
        anchors.margins: 1
        VideoPlayer {
            type: VideoStream.TEST
            width: 400
            height: 480/640 * width
        }
        VideoPlayer {
            type: VideoStream.KLV_DECODE
            width: 400
            height: 9/16 * width
        }
    }
}
