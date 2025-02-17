import QtQuick
import QtLocation
import QtPositioning

Rectangle {
    width: 800
    height: 600

    property var uavCoordinate: QtPositioning.coordinate(39.925025, 32.836943) // UAV başlangıç konumu
    property real uavHeading: 0 // UAV'ın yön açısı
    property var trailPath: [] // UAV'ın geçtiği izler
    property var clickedCoordinate: QtPositioning.coordinate(0, 0) // Son tıklanan konum

    Map {
        id: map
        anchors.fill: parent
        opacity: 0.99
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.host"
                value: "https://api.maptiler.com/maps/satellite-v2/tiles.json?key=ClRXPuGZzvarZejWpw6q"
            }
        }

        center: uavCoordinate
        zoomLevel: 18

        // UAV'ın izini gösteren poligon
        MapPolyline {
            id: trail
            line.width: 3
            line.color: "#aaFF0000" // Hafif saydam kırmızı
            smooth: true
            path: trailPath
        }

        // UAV'ı temsil eden SVG
        MapQuickItem {
            id: uavMarker
            coordinate: uavCoordinate
            sourceItem: Image {
                source: "qrc:/images/images/uav.svg"
                width: 30
                height: 30
                transform: Rotation {angle: uavHeading}
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
    function updateUAVCoordinate(latitude, longitude, heading) {
        var newCoordinate = QtPositioning.coordinate(latitude, longitude);
        uavCoordinate = newCoordinate;
        uavHeading = heading;
        //console.log("uavHeading : ",uavHeading);

        // İz bırakmak için eski konumları trail'e ekleyelim
        if (trailPath.length === 0 || trailPath[trailPath.length - 1] !== newCoordinate) {
            trailPath.push(newCoordinate);
        }

        map.center = uavCoordinate;
    }
}
