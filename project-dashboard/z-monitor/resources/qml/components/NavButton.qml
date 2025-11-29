/**
 * @file NavButton.qml
 * @brief Reusable navigation button component for bottom nav bar
 */

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 100
    height: 48
    color: active ? "#27272a" : "transparent"
    radius: 4

    property string text: ""
    property string icon: ""
    property bool active: false
    property color activeColor: "#10b981"

    signal clicked

    Column {
        anchors.centerIn: parent
        spacing: 4

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.icon
            font.pixelSize: 18
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.text
            font.pixelSize: 9
            font.bold: true
            font.letterSpacing: 1
            color: root.active ? root.activeColor : "#71717a"
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
        hoverEnabled: true
        onEntered: if (!root.active)
            root.color = "#18181b"
        onExited: if (!root.active)
            root.color = "transparent"
    }
}
