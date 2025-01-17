import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    width: 800
    height: 600

    // Harita öğesini tanımlayın
    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm" // OpenStreetMap plugin
        }
        center: QtPositioning.coordinate(37.7749, -122.4194) // Başlangıç koordinatları (San Francisco)
        zoomLevel: 10

        // UAV'ın konumunu göstermek için bir işaretçi ekleyin
        MapQuickItem {
            id: uavMarker
            coordinate: QtPositioning.coordinate(37.7749, -122.4194)
            sourceItem: Rectangle {
                width: 10
                height: 10
                color: "red"
                radius: 5
                border.color: "white"
            }
        }

        MouseArea {
            id: mapMouseArea
            anchors.fill: parent
            onClicked: {
                var lat = map.toCoordinate(Qt.point(mouse.x, mouse.y)).latitude;
                var lon = map.toCoordinate(Qt.point(mouse.x, mouse.y)).longitude;
                console.log("Clicked at: " + lat + ", " + lon);
                Qt.callLater(Qt.copyToClipboard, lat + ", " + lon);
            }
        }
    }
}
