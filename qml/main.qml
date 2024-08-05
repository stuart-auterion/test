import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtLocation 5.15
import QtPositioning 5.15

import Map 1.0

Window {
    id: root
    width: 810
    height: 480
    visible: true
    title: qsTr("Hello World")
    Component.onCompleted: {
        baseMap.zoomLevel = 1
    }

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
                width: parent.width
                model: baseMap.supportedMapTypes
                textRole: "description"
                onCurrentIndexChanged: {
                    baseMap.activeMapType = baseMap.supportedMapTypes[currentIndex]
                }
            }
            Button {
                text: "Clear cache"
                onClicked: {
                    MapTileProvider.clearCache()
                }
            }
        }
    }
    Map {
        id: baseMap
        anchors.fill: parent
        activeMapType: supportedMapTypes[0]
        gesture.acceptedGestures: MapGestureArea.PinchGesture | MapGestureArea.PanGesture | MapGestureArea.PanGesture
        plugin: Plugin {
            id: mapboxglPlugin
            name: "mapboxgl"
            PluginParameter {
                name: "mapboxgl.mapping.cache.size"
                value: 1073741824
            }
            PluginParameter {
                name: "mapboxgl.mapping.use_fbo"
                value: false
            }
            // PluginParameter {
            //     name: "mapboxgl.mapping.cache.directory"
            //     value: MapTileProvider.cacheDirectory
            // }
        }
    }
    Map {
        id: customTilesMap
        anchors.fill: parent
        zoomLevel: baseMap.zoomLevel
        center: baseMap.center
        color: "transparent"
        gesture.acceptedGestures: MapGestureArea.NoGesture
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]
        plugin: Plugin {
            id: customTilesPlugin
            name: "osm"
            PluginParameter {
                name: "osm.mapping.cache.directory"
                value: MapTileProvider.cacheDirectory
            }
            PluginParameter {
                name: "osm.mapping.custom.host"
                value: MapTileProvider.address
            }
            // Turn off all other services
            PluginParameter {
                name: "osm.mapping.providersrepository.disabled"
                value: true
            }
            PluginParameter {
                name: "osm.places.host"
                value: "undefined"
            }
            PluginParameter {
                name: "osm.routing.host"
                value: "undefined"
            }
            PluginParameter {
                name: "osm.geocoding.host"
                value: "undefined"
            }
        }
    }
}
