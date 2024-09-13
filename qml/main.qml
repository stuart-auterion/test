import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Layouts 1.15

import Map 1.0

ApplicationWindow {
    id: root
    width: 810
    height: 480
    visible: true
    title: qsTr("Maps!")
    Component.onCompleted: {
        map.zoomLevel = 1
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
            Text {
                width: parent.width
                text: qsTr("Base Map")
                horizontalAlignment: Text.AlignHCenter
                font.underline: true
            }
            ComboBox {
                width: parent.width
                model: baseMap.supportedMapTypes
                textRole: "description"
                onCurrentIndexChanged: {
                    baseMap.activeMapType = baseMap.supportedMapTypes[currentIndex]
                }
            }
            Button {
                width: parent.width
                text: "Clear cache"
                onClicked: {
                    MapTileProvider.clearCache()
                }
            }
            Text {
                width: parent.width
                text: qsTr("Custom Tiles")
                horizontalAlignment: Text.AlignHCenter
                font.underline: true
            }
            Button {
                width: parent.width
                text: "Zoom to custom tiles"
                onClicked: {
                    if (MapTileProvider.tileArea.topLeft.isValid) {
                        map.fitViewportToGeoShape(MapTileProvider.tileArea,
                                                              root.width * 0.10)
                        customTilesOutline.opacity = 1.0
                        customTilesOutlineTimer.start()
                    }
                }
                Timer {
                    id: customTilesOutlineTimer
                    interval: 1000
                    onTriggered: {
                        customTilesOutline.opacity = 0.0
                    }
                }
            }
            Button {
                width: parent.width
                text: "Reload custom tiles"
                onClicked: {
                    MapTileProvider.reloadCustomTiles()
                }
            }
            GridLayout {
                width: parent.width
                columns: 2
                Text {
                    text: qsTr("Custom Tile Info")
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.underline: true
                }
                Text {
                    text: qsTr("Zoom:")
                }
                Text {
                    text: MapTileProvider.maxZoomLevel
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                Text {
                    text: qsTr("Lat:")
                }
                Text {
                    text: MapTileProvider.tileArea.topLeft.latitude.toFixed(5) + ", " + /* */
                          MapTileProvider.tileArea.bottomRight.latitude.toFixed(5)
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                Text {
                    text: qsTr("Long:")
                }
                Text {
                    text: MapTileProvider.tileArea.topLeft.longitude.toFixed(5) + ", " + /* */
                          MapTileProvider.tileArea.bottomRight.longitude.toFixed(5)
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
    // Make a root-level parameter pointing to the loaded item so that baseMap is accessible by the same name
    // outside of the loader
    property var baseMap: baseMapLoader.item
    Loader {
        id: baseMapLoader
        anchors.fill: parent
        sourceComponent: baseMapComponent
        // We have to do this workaround because baseMap.clearData() doesn't actually seem to work for mapboxgl, and
        // manually clearing the cache causes a crash, so the only way to properly clear it is to entirely reload
        // the Map component.
        Connections {
            target: MapTileProvider
            function onCacheCleared() {
                baseMapLoader.sourceComponent = undefined
                baseMapLoader.sourceComponent = baseMapComponent
            }
        }
    }
    Component {
        id: baseMapComponent
        Map {
            id: baseMap
            anchors.fill: parent
            activeMapType: supportedMapTypes[0]
            zoomLevel: itemsOverlayMap.zoomLevel
            center: itemsOverlayMap.center
            minimumZoomLevel: itemsOverlayMap.minimumZoomLevel
            maximumZoomLevel: itemsOverlayMap.maximumZoomLevel
            gesture.enabled: false
            color: "transparent"
            copyrightsVisible: false
            plugin: Plugin {
                name: "mapboxgl"
                PluginParameter {
                    name: "mapboxgl.mapping.cache.size"
                    value: 1073741824
                }
                PluginParameter {
                    name: "mapboxgl.mapping.use_fbo"
                    value: false
                }
                PluginParameter {
                    name: "mapboxgl.mapping.cache.directory"
                    value: MapTileProvider.cacheDirectory
                }
            }
        }
    }
    Map {
        id: customTilesMap
        anchors.fill: parent
        zoomLevel: itemsOverlayMap.zoomLevel
        center: itemsOverlayMap.center
        color: "transparent"
        gesture.enabled: false
        minimumZoomLevel: itemsOverlayMap.minimumZoomLevel
        maximumZoomLevel: itemsOverlayMap.maximumZoomLevel
        activeMapType: supportedMapTypes[0]
        Connections {
            target: MapTileProvider
            function onCustomTilesChanged() {
                customTilesMap.clearData()
            }
        }
        plugin: Plugin {
            id: customTilesPlugin
            name: "osm"
            PluginParameter {
                name: "osm.mapping.cache.directory"
                value: MapTileProvider.cacheDirectory
            }
            // Cache no tiles. Since they are served from localhost, there is no speedup for caching and all
            // we'll be doing is duplicating files already on disk
            PluginParameter {
                name: "osm.mapping.cache.disk.size"
                value: "0"
            }
            PluginParameter {
                name: "osm.mapping.cache.disk.cost_strategy"
                value: "unitary"
            }
            // We're using a custom provdiers repository as this gives us additional control over map metadata,
            // particularly the max zoom level. We could alternatively set the property "osm.mapping.custom.host"
            // and point that to our localhost, but that ONLY serves tiles and doesn't allow control of anything
            // else
            PluginParameter {
                name: "osm.mapping.providersrepository.address"
                value: MapTileProvider.address
            }
            // Set all the other hosts to unused. These shouldn't ever be called, but just make sure we're not
            // making extraneous web calls to things we don't care about
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
    // To reduce confusion about the multiple maps, make a generic pointer to the topmost map which we should be
    // interacting with
    property var map: itemsOverlayMap
    Map {
        id: itemsOverlayMap
        anchors.fill: parent
        minimumZoomLevel: 0
        maximumZoomLevel: MapTileProvider.maxZoomLevel
        gesture.acceptedGestures: MapGestureArea.PinchGesture | MapGestureArea.PanGesture | MapGestureArea.PanGesture
        color: "transparent"
        plugin: Plugin {
            name: "itemsoverlay"
        }
        MapPolyline {
            id: customTilesOutline
            visible: opacity > 0
            opacity: 0
            Behavior on opacity {
                enabled: customTilesOutlineTimer.running
                NumberAnimation {
                    duration: customTilesOutlineTimer.interval
                }
            }
            line.width: 2
            line.color: "blue"
            path: [/* */
                MapTileProvider.tileArea.topLeft, /* */
                MapTileProvider.tileArea.topRight, /* */
                MapTileProvider.tileArea.bottomRight, /* */
                MapTileProvider.tileArea.bottomLeft, /* */
                MapTileProvider.tileArea.topLeft /* */
            ]
        }
    }
}
