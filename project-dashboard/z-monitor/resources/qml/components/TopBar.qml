/**
 * @file TopBar.qml
 * @brief Top header bar component for Z Monitor
 *
 * Displays patient banner, clock, connection status, battery, and notifications
 *
 * @author Z Monitor Team
 * @date 2025-12-09
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme 1.0

Rectangle {
    id: topBar

    // Public properties
    property string patientName: ""
    property string bedLocation: ""
    property bool isMonitoring: false
    property bool isConnected: true
    property int batteryLevel: 98
    property bool isCharging: false
    property bool hasActiveAlarms: false
    property bool showNotifications: false

    // Theme colors (passed from parent)
    property color colorPanel: "#18181b"
    property color colorBorder: "#27272a"
    property color colorText: "#e5e7eb"
    property color colorTextMuted: "#9ca3af"
    property color colorECG: "#10b981"
    property color colorCritical: "#ef4444"
    property color bannerColor: "#18181b"
    property color bannerBorderColor: "#27272a"

    // Signals
    signal patientBannerClicked
    signal notificationClicked

    // Styling
    color: colorPanel
    border.color: colorBorder
    border.width: 1
    clip: true  // Prevent children from drawing outside bounds

    // Compute battery icon source based on level and charging state
    function getBatteryIconSource() {
        if (isCharging) {
            return "qrc:/qml/icons/battery-charging.svg";
        } else if (batteryLevel <= 10) {
            return "qrc:/qml/icons/battery-10.svg";
        } else if (batteryLevel <= 20) {
            return "qrc:/qml/icons/battery-20.svg";
        } else if (batteryLevel <= 50) {
            return "qrc:/qml/icons/battery-50.svg";
        } else if (batteryLevel <= 75) {
            return "qrc:/qml/icons/battery-75.svg";
        } else {
            return "qrc:/qml/icons/battery-100.svg";
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        Row {
            Layout.preferredHeight: 64

            // Patient Banner (Left)
            PatientBanner {

                // Layout.preferredWidth: 220
                // Layout.fillHeight: true
                name: topBar.patientName
                bedLabel: topBar.bedLocation
                bannerColor: topBar.bannerColor
                bannerBorderColor: topBar.bannerBorderColor
                colorText: topBar.colorText
                accentColor: topBar.colorECG
                onBannerClicked: topBar.patientBannerClicked()
            }
        }

        // Spacer
        Item {
            Layout.fillWidth: true
        }

        // Clinician Info (Clock & Date)
        Column {
            spacing: 6
            Text {
                id: clockText
                text: Qt.formatTime(new Date(), "HH:mm:ss")
                font.pixelSize: 12
                font.family: "Courier New"
                color: topBar.colorTextMuted
            }
            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: clockText.text = Qt.formatTime(new Date(), "HH:mm:ss")
            }
            Rectangle {
                width: 1
                height: 14
                color: "#3f3f46" // Zinc-700
            }
            Text {
                text: Qt.formatDate(new Date(), "dd MMM yyyy").toUpperCase()
                font.pixelSize: 12
                font.family: "Courier New"
                color: "#52525b" // Zinc-600
            }
        }

        // Connection Status
        ConnectionStatus {
            Layout.preferredWidth: 220
            Layout.preferredHeight: 36
            connected: topBar.isMonitoring
            sensorType: "Shared Memory"
        }

        // Divider
        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: topBar.colorBorder
        }

        // Network Status
        Row {
            spacing: 8

            Image {
                source: topBar.isConnected ? "qrc:/qml/icons/wifi-emerald.svg" : "qrc:/qml/icons/wifi-off-red.svg"
                width: 18
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }

            Column {
                spacing: 2

                Text {
                    text: "CMS ONLINE"
                    font.pixelSize: 10
                    font.bold: true
                    color: topBar.colorTextMuted
                }

                Text {
                    text: "mTLS"
                    font.pixelSize: 8
                    color: "#52525b" // Zinc-600
                }
            }
        }

        // Battery
        Row {
            spacing: 6

            Image {
                source: topBar.getBatteryIconSource()
                width: 18
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: topBar.batteryLevel + "%"
                font.pixelSize: 11
                font.family: "Courier New"
                font.bold: true
                color: topBar.batteryLevel <= 20 ? topBar.colorCritical : topBar.colorTextMuted
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Notification Bell
        Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            color: topBar.showNotifications ? topBar.colorBorder : "transparent"
            radius: 20

            Image {
                anchors.centerIn: parent
                source: topBar.hasActiveAlarms ? "qrc:/qml/icons/bell-red.svg" : "qrc:/qml/icons/bell-muted.svg"
                width: 20
                height: 20
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: topBar.notificationClicked()
            }
        }
    }
}
