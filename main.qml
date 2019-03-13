import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

Window {
    visible: true
    color: "lightgrey"
    width: Qt.platform.os==="windows" ? 640 : Screen.width
    height: Qt.platform.os==="windows" ? 480 : Screen.height
    title: qsTr("Hello World")
    //    Camera {
    //        id: camera
    //        objectName: "myCamera"
    //    }
    //        VideoOutput {
    //            id: directCamera
    //            source: camera
    //            anchors.top: parent.top
    //            anchors.right: parent.right
    //            anchors.left: parent.left
    //            height: parent.height / 2
    //            focus : visible // to receive focus and capture key events when visible
    //        }
    Component.onCompleted: {
        console.info(Qt.platform.os);
        textField.text += ", "+"Screen.width:"+Screen.width;
        textField.text += ", "+"Screen.height:"+Screen.height;
    }

    VideoOutput {
        id: streamedCamera
        objectName: "display"
        height: parent.height/2
        anchors.top: parent.top
        width: parent.width
        source: $provider
    }
    Row {
        spacing: 1
        Rectangle {
            id: startClient
            width: 50
            height: 50
            Text {
                text: qsTr("Start")
                anchors.centerIn: parent
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    $application.createClient();
                }
            }
        }
        Rectangle {
            id: closeClient
            width: 50
            height: 50
            Text {
                text: qsTr("Close")
                anchors.centerIn: parent
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    $application.closeClient();
                }
            }
        }
    }

    Rectangle {
        anchors.right: parent.right;
        anchors.rightMargin: 10;
        anchors.left: parent.left
        anchors.leftMargin: 10
        color: "grey"
        anchors.bottom: parent.bottom
        height: parent.height/3
        Text {
            id: textField
            text: qsTr("text")
            wrapMode: Text.WrapAnywhere
            width: parent.width
            height: parent.height
        }
    }
    Connections {
        target: $application
        onUpdateText: {
            textField.text += ", "+text;
        }
    }
}
