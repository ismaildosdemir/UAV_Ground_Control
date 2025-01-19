import QtQuick
import QtLocation
import QtPositioning


Rectangle {
    width: 800
    height: 600

    property var uavCoordinate: QtPositioning.coordinate(39.925025, 32.836943) // UAV başlangıç konumu



    // Harita öğesini tanımlayın
    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.host"
                value: "https://api.maptiler.com/maps/outdoor-v2/tiles.json?key=ClRXPuGZzvarZejWpw6q"
            }
        }


        center: uavCoordinate
        zoomLevel: 18

        // UAV'ın konumunu göstermek için bir işaretçi ekleyin
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

        // WheelHandler ile fare tekerleği hareketini yakalıyoruz
        WheelHandler {
            id: wheelHandler
            target: map
            onWheel: function(event) {
                // event.angleDelta.y tekerleğin yukarı/aşağı hareketini belirtir
                if (event.angleDelta.y > 0) {
                    map.zoomLevel += 1; // Zoom in
                } else if (event.angleDelta.y < 0) {
                    map.zoomLevel -= 1; // Zoom out
                }
                // zoomLevel'in minimum ve maksimum değerlerini ayarlayabilirsiniz
                map.zoomLevel = Math.max(5, Math.min(map.zoomLevel, 30));
            }
        }

        MouseArea {
            id: mapMouseArea
            anchors.fill: parent
            onClicked: {
                var coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                console.log("Clicked coordinate - Latitude:", coordinate.latitude, "Longitude:", coordinate.longitude);
                mapFunction.updateCoordinates(coordinate.latitude, coordinate.longitude); // C++ tarafındaki işlevi çağır
            }
        }


    }

    // UAV konumunu güncellemek için fonksiyon
    function updateUAVCoordinate(latitude, longitude) {
        uavCoordinate = QtPositioning.coordinate(latitude, longitude);
        map.center = uavCoordinate;
    }
}
