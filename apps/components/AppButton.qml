import QtQuick 2.12

Rectangle {

    id: root

    width: 180
    height: 50

    radius: 6

    color: "#4A90E2"

    property alias text: label.text

    signal clicked()

    Text {

        id: label

        anchors.centerIn: parent

        color: "white"

        font.pixelSize: 18
    }

    MouseArea {

        anchors.fill: parent

        onClicked: root.clicked()

        onPressed: root.opacity = 0.7

        onReleased: root.opacity = 1
    }
}
