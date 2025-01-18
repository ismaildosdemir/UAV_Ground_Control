import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    width: 800
    height: 600

    property string thunderforestApiKey: "YOUR_API_KEY" // Thunderforest API anahtarını buraya ekleyin

    Plugin {
        id: osmPlugin
        name: "osm"

        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.thunderforest.com/landscape/{z}/{x}/{y}.png?apikey=" + thunderforestApiKey
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: osmPlugin
        center: QtPositioning.coordinate(37.7749, -122.4194) // Başlangıç merkezi
        zoomLevel: 10

        MapQuickItem {
            id: uavMarker
            coordinate: QtPositioning.coordinate(37.7749, -122.4194) // UAV başlangıç koordinatı
            sourceItem: Rectangle {
                width: 10
                height: 10
                color: "red"
                radius: 5
                border.color: "white"
            }
        }
    }

    WheelHandler {
        id: wheelHandler
        target: map
        onWheel: {
            if (event.angleDelta.y > 0) {
                map.zoomLevel += 1; // Yaklaştır
            } else if (event.angleDelta.y < 0) {
                map.zoomLevel -= 1; // Uzaklaştır
            }
            map.zoomLevel = Math.max(1, Math.min(map.zoomLevel, 30));
        }
    }

    MouseArea {
        id: mapMouseArea
        anchors.fill: parent
        onClicked: {
            var lat = map.toCoordinate(Qt.point(mouse.x, mouse.y)).latitude;
            var lon = map.toCoordinate(Qt.point(mouse.x, mouse.y)).longitude;
            console.log("Clicked at: " + lat + ", " + lon);
            updateUAVCoordinate(lat, lon);

        }
    }

    function updateUAVCoordinate(latitude, longitude) {
        uavMarker.coordinate = QtPositioning.coordinate(latitude, longitude);
        map.center = QtPositioning.coordinate(latitude, longitude);
    }
}
