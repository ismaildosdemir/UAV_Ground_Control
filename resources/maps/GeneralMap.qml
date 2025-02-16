import QtQuick
import QtLocation
import QtPositioning

Rectangle {
    width: 800
    height: 600

    property var uavCoordinate: QtPositioning.coordinate(39.925025, 32.836943) // UAV başlangıç konumu
    property var clickedCoordinate: QtPositioning.coordinate(0, 0) // Son tıklanan konum

    Map {
        id: map
        anchors.fill: parent
        opacity: 0.99 // https://bugreports.qt.io/browse/QTBUG-82185
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.host"
                value: "https://api.maptiler.com/maps/satellite-v2/tiles.json?key=ClRXPuGZzvarZejWpw6q"
            }
        }

        center: uavCoordinate
        zoomLevel: 18

        // UAV'ın konumunu göstermek için işaretçi
        MapQuickItem {
            id: uavMarker
            coordinate: uavCoordinate
            sourceItem: Rectangle {
                width: 10
                height: 10
                color: "red"
                radius: 5
                border.color: "white"
            }
        }

        // Tıklanan yeri göstermek için işaretçi
        MapQuickItem {
            id: clickMarker
            coordinate: clickedCoordinate
            visible: false // Başlangıçta gizli
            opacity: 0.7 // Yarı saydam yapıyoruz

            sourceItem: Rectangle {
                width: 10
                height: 10
                color: "#ff5252"
                radius: 4
                border.color: "#ff5252"
            }
        }

        // Fare tekerleği ile yakınlaştırma
        WheelHandler {
            id: wheelHandler
            target: map
            onWheel: function(event) {
                if (event.angleDelta.y > 0) {
                    map.zoomLevel += 1; // Yakınlaştır
                } else if (event.angleDelta.y < 0) {
                    map.zoomLevel -= 1; // Uzaklaştır
                }
                map.zoomLevel = Math.max(5, Math.min(map.zoomLevel, 30));
            }
        }

        // Haritada tıklanan yere işaret koyma
        MouseArea {
            id: mapMouseArea
            anchors.fill: parent
            onClicked: {
                var coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                console.log("Clicked coordinate - Latitude:", coordinate.latitude, "Longitude:", coordinate.longitude);
                mapFunction.updateCoordinates(coordinate.latitude, coordinate.longitude); // C++ tarafındaki işlevi çağır

                clickedCoordinate = QtPositioning.coordinate(coordinate.latitude, coordinate.longitude);
                clickMarker.visible = true; // İşareti görünür yap
            }
        }
    }

    // UAV konumunu güncellemek için fonksiyon
    function updateUAVCoordinate(latitude, longitude) {
        uavCoordinate = QtPositioning.coordinate(latitude, longitude);
        map.center = uavCoordinate;
    }
}
