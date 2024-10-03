import QtQuick 2.15
import QtQuick.Window 2.15
import QtLocation 5.15

Window {
    id: root
    width: 810
    height: 480
    visible: true
    title: qsTr("Hello World")

    Map {
        anchors.fill: parent
        center: topMap.center
        zoomLevel: topMap.zoomLevel
        color: "transparent"
        gesture.enabled: false
        plugin: Plugin {
            name: "mapboxgl"
        }
    }

    Map {
        anchors.fill: parent
        center: topMap.center
        zoomLevel: topMap.zoomLevel
        color: "transparent"
        gesture.enabled: false
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.custom.host"
                value: "https://tiles.openseamap.org/seamark/"
            }
        }
    }
    Map {
        id: topMap
        color: "transparent"
        anchors.fill: parent
        plugin: Plugin {
            name: "itemsoverlay"
        }
    }
}
