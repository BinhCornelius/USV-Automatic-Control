import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15
import com.example 1.0

Window {
    visible: true
    width: 800
    height: 600
    title: "Map Viewer"

    // Đặt khai báo thuộc tính ở đây
    property int pwmValue1: 0
    property int pwmValue2: 0

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(21.004107983959077,105.84333981255992) // Hồ Tiền ĐHBK Hà Nội

        Component {
            id: circleComponent
            MapQuickItem {
                sourceItem: Rectangle {
                    width: 10
                    height: 10
                    color: "black"
                    radius: 5
                }
                coordinate: QtPositioning.coordinate(21.00618786830872,105.84315652615584) // Hồ Tiền ĐHBK Hà Nội
                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height / 2
            }
        }

        MapQuickItem {
            id: gpsMarker
            sourceItem: Rectangle {
                width: 10
                height: 10
                color: "red"
                radius: 5
            }
            coordinate: QtPositioning.coordinate(0, 0)
            anchorPoint.x: sourceItem.width / 2
            anchorPoint.y: sourceItem.height / 2
            visible: false
        }
        Connections {
            target: mapHandler
            onGpsCoordinateChanged: {
                gpsMarker.center = mapHandler.gpsCoordinate
                gpsMarker.coordinate = QtPositioning.coordinate(mapHandler.gpsCoordinate.lat, mapHandler.gpsCoordinate.lng);
                gpsMarker.visible = true;
            }
        }

        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
            }
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            onRotationChanged: (delta) => {
                map.bearing -= delta
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }
        WheelHandler {
            id: wheel
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            rotationScale: 1/120
            property: "zoomLevel"
        }
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }
        Shortcut {
            enabled: map.zoomLevel < map.maximumZoomLevel
            sequence: StandardKey.ZoomIn
            onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
        }
        Shortcut {
            enabled: map.zoomLevel > map.minimumZoomLevel
            sequence: StandardKey.ZoomOut
            onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                console.log("Latitude: " + coordinate.latitude + ", Longitude: " + coordinate.longitude);
                mapHandler.addCoordinate(coordinate.latitude, coordinate.longitude);

                var circle = circleComponent.createObject(map);
                circle.coordinate = coordinate;
                map.addMapItem(circle);
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        spacing: 10

        TextField {
            id: pathTypeInput
            placeholderText: "Path Type"
            validator: DoubleValidator {}

        }
        TextField {
            id: rXInput
            placeholderText: "r_x"
            validator: DoubleValidator {}
        }
        TextField {
            id: rYInput
            placeholderText: "r_y"
            validator: DoubleValidator {}
        }
        TextField {
            id:omgXInput
            placeholderText: "omg_x"
            validator: DoubleValidator {}
        }
        TextField {
            id: omgYInput
            placeholderText: "omg_y"
            validator: DoubleValidator {}
        }
    }

    Button {
        id: sendCoordinate
        text: "Send Coordinates"
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: 20
        }
        onClicked: {
            mapHandler.sendCoordinates(
                parseFloat(pathTypeInput.text),
                parseFloat(rXInput.text),
                parseFloat(rYInput.text),
                parseFloat(omgXInput.text),
                parseFloat(omgYInput.text)
            )

            // Clear text fields
            pathTypeInput.text = ""
            rXInput.text = ""
            rYInput.text = ""
            omgXInput.text = ""
            omgYInput.text = ""
            // Xóa tất cả các chấm tròn ngoại trừ gpsMarker
            for (var i = map.mapItems.length - 1; i >= 0; i--) {
                var item = map.mapItems[i];
                if (item !== gpsMarker) {
                    map.removeMapItem(item);
                    item.destroy();
                }
            }
            // Duy trì chấm tròn GPS
            gpsMarker.visible = true;
        }
    }

    Button {
        text: "Enable Key Control"
        onClicked: {
            keyHandler.focus = true;
        }
    }

    Item {
        id: keyHandler
        //focus: true
        anchors.fill: parent
        Keys.onPressed: {
            if (event.key === Qt.Key_W) {
                if (pwmValue1 < 194) {
                    pwmValue1 += 1;
                    pwmValue2 += 1;
                    if (pwmValue1 > 194) pwmValue1 = 194;
                    if (pwmValue2 > 194) pwmValue2 = 194;
                    mapHandler.sendPwm(pwmValue1, pwmValue2);
                }
            } else if (event.key === Qt.Key_S) {
                if (pwmValue1 > 0) pwmValue1 -= 1;
                if (pwmValue2 > 0) pwmValue2 -= 1;
                if (pwmValue1 < 0) pwmValue1 = 0;
                if (pwmValue2 < 0) pwmValue2 = 0;
                    mapHandler.sendPwm(pwmValue1, pwmValue2);

            } else if (event.key === Qt.Key_A) {
                if (pwmValue1 < 194) {
                    pwmValue1 += 1;
                    if (pwmValue1 > 194) pwmValue1 = 194;
                }
                if (pwmValue2 > 0) {
                    pwmValue2 -= 1;
                    if (pwmValue2 < 0) pwmValue2 = 0;
                }
                mapHandler.sendPwm(pwmValue1, pwmValue2);
            } else if (event.key === Qt.Key_D) {
                if (pwmValue1 > 0) {
                    pwmValue1 -= 1;
                    if (pwmValue1 < 0) pwmValue1 = 0;
                }
                if (pwmValue2 < 194) {
                    pwmValue2 += 1;
                    if (pwmValue2 > 194) pwmValue2 = 194;
                }
                mapHandler.sendPwm(pwmValue1, pwmValue2);
            } else if (event.key === Qt.Key_Q) {
                    pwmValue1 = 0;
                    pwmValue2 = 0;

                mapHandler.sendPwm(pwmValue1, pwmValue2);
            }
        }
    }
}
