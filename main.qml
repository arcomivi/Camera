import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

Window {
    visible: true
    color: "lightgrey"
    //width: 640
    //height: 480
    title: qsTr("Hello World")
    Camera {
        id: camera
    }
    VideoOutput {
        source: camera
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: parent.height / 2
        focus : visible // to receive focus and capture key events when visible
    }
}
