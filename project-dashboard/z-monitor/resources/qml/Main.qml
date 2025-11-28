import QtQuick 2.15
import QtQuick.Controls 2.15

/*
 * Placeholder QML entry point for the Z Monitor UI.
 *
 * This view will be replaced by the full dashboard and navigation
 * shell described in 02_ARCHITECTURE.md. For now it is only used
 * to verify that the executable and QML engine are wired correctly.
 */
ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: qsTr("Z Monitor (Bootstrap)")

    Rectangle {
        anchors.fill: parent
        color: "#101820"

        Column {
            anchors.centerIn: parent
            spacing: 16

            Label {
                text: qsTr("Z Monitor – Bootstrap Skeleton")
                font.pixelSize: 28
                color: "#ffffff"
            }

            Label {
                text: qsTr("DDD structure is in place. Controllers and services will be wired next.")
                font.pixelSize: 18
                color: "#a0b0c0"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}


