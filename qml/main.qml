import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtLocation 5.15
import QtPositioning 5.15

Window {
    id: root
    width: 810
    height: 480
    visible: true
    title: qsTr("Hello World")
    Rectangle {
        z: 100
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        width: 200 + anchors.margins * 2
        height: 400
        color: "white"
        border.width: 1
        border.color: "black"
        Column {
            anchors.fill: parent
            anchors.margins: parent.anchors.margins
            spacing: parent.anchors.margins
            ComboBox {
                model: ["Custom Tiles", "MapBox"]
                currentIndex: useCustomTiles ? 0 : 1
                onCurrentIndexChanged: {
                    useCustomTiles = (currentIndex === 0)
                }
            }
            ComboBox {
                enabled: !useCustomTiles
                visible: enabled
                width: parent.width
                model: !useCustomTiles ? map.supportedMapTypes : []
                textRole: "description"
                onCurrentIndexChanged: {
                    if (!useCustomTiles) {
                        map.activeMapType = map.supportedMapTypes[currentIndex]
                    }
                }
            }
        }
    }
    property var map: loader.item
    property bool useCustomTiles: true
    onUseCustomTilesChanged: loader.reload()
    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: mapComponent
        function reload() {
            var center = map.center
            var zoomLevel = map.zoomLevel
            sourceComponent = undefined
            sourceComponent = mapComponent
            map.center = center
            map.zoomLevel = zoomLevel
        }
    }
    Component {
        id: mapComponent
        Map {
            id: map
            plugin: useCustomTiles ? customTilesPlugin : mapboxglPlugin
            zoomLevel: 1
            gesture.acceptedGestures: MapGestureArea.PinchGesture | MapGestureArea.PanGesture | MapGestureArea.PanGesture
            activeMapType: supportedMapTypes[useCustomTiles ? supportedMapTypes.length - 1 : 0]
        }
    }
    Plugin {
        id: mapboxglPlugin
        name: "mapboxgl"
    }

    Plugin {
        id: customTilesPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.cache.directory"
            value: "/home/stuart/Desktop/cache"
        }
        PluginParameter {
            name: "osm.mapping.offline.directory"
            value: "/home/stuart/Desktop/tiles"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: true
        }
        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "undefined"
        }
    }
}
