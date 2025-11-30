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
    radius: 4

    property string text: ""
    property string iconBase: ""  // Base icon name without path or suffix (e.g., "layout-dashboard", "brain-circuit")
    property string iconColorSuffix: ""  // Color suffix for active state (e.g., "emerald", "purple", "blue", "white")
    property bool active: false
    property color activeColor: "#10b981"
    property bool hovered: false

    signal clicked

    // Compute background color based on active and hover states
    // Active state takes precedence over hover
    color: {
        if (root.active) {
            return "#27272a"
        } else if (root.hovered) {
            return "#18181b"
        } else {
            return "transparent"
        }
    }

    // Compute icon source based on active state
    // Active: use colored variant (e.g., "layout-dashboard-emerald.svg")
    // Inactive: use muted variant (e.g., "layout-dashboard-muted.svg")
    function getIconSource() {
        if (root.iconBase === "") return ""
        if (root.active && root.iconColorSuffix !== "") {
            return "qrc:/qml/icons/" + root.iconBase + "-" + root.iconColorSuffix + ".svg"
        } else {
            return "qrc:/qml/icons/" + root.iconBase + "-muted.svg"
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 4

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: root.getIconSource()
            width: 20
            height: 20
            sourceSize.width: 20
            sourceSize.height: 20
            fillMode: Image.PreserveAspectFit
            visible: root.iconBase !== ""
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
        onEntered: root.hovered = true
        onExited: root.hovered = false
    }
}
