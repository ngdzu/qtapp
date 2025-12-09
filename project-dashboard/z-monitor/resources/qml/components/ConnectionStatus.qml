/**
 * @file ConnectionStatus.qml
 * @brief Connection status indicator component
 *
 * Displays sensor connection state with pulsing animation
 */

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property bool connected: false
    property string sensorType: "Shared Memory"

    width: 220
    height: 36
    radius: 18
    color: connected ? "#10b98120" : "#ef444420"
    border.color: connected ? "#10b981" : "#ef4444"
    border.width: 2

    Row {
        anchors.centerIn: parent
        spacing: 8

        // Status indicator dot
        Rectangle {
            width: 10
            height: 10
            radius: 5
            color: root.connected ? "#10b981" : "#ef4444"
            anchors.verticalCenter: parent.verticalCenter
            opacity: 1.0
        }

        Text {
            text: root.connected ? "Connected" : "Disconnected"
            color: root.connected ? "#10b981" : "#ef4444"
            font.pixelSize: 13
            font.weight: Font.Medium
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: "(" + root.sensorType + ")"
            color: "#9ca3af"
            font.pixelSize: 11
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
