/**
 * @file Main.qml
 * @brief Main application entry point for Z Monitor
 *
 * Fixed 1280x800 resolution for 8-inch touch screen
 * Dark cyberpunk/modern aesthetic with medical monitoring theme
 * Converted from Node.js reference UI (sample_app/z-monitor)
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "views"

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    minimumWidth: 1280
    minimumHeight: 800
    maximumWidth: 1280
    maximumHeight: 800
    title: "Z Monitor - Patient Monitoring System"
    color: "#09090b" // Zinc-950

    // Theme Colors (matching Node.js reference)
    readonly property color colorBackground: "#09090b"      // Zinc-950
    readonly property color colorPanel: "#18181b"           // Zinc-900
    readonly property color colorBorder: "#27272a"          // Zinc-800
    readonly property color colorText: "#fafafa"            // Zinc-50
    readonly property color colorTextMuted: "#71717a"       // Zinc-500
    readonly property color colorECG: "#10b981"             // Emerald-500 (green)
    readonly property color colorSPO2: "#3b82f6"            // Blue-500
    readonly property color colorResp: "#eab308"            // Yellow-500
    readonly property color colorNIBP: "#a1a1aa"            // Zinc-400
    readonly property color colorTemp: "#d946ef"            // Fuchsia-500
    readonly property color colorCritical: "#ef4444"        // Red-500
    readonly property color colorWarning: "#f59e0b"         // Amber-500
    readonly property color colorSuccess: "#10b981"         // Emerald-500

    // View States
    enum ViewState {
        Monitor,
        Trends,
        Analysis,
        Settings
    }

    property int currentView: root.ViewState.Monitor
    property bool showNotifications: false

    // Main Layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header (Top Bar) - 60px
        Rectangle {
            id: header
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: root.colorPanel
            border.color: root.colorBorder
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 8
                anchors.bottomMargin: 8
                spacing: 16

                // Patient Banner (Left)
                Rectangle {
                    Layout.preferredWidth: 220
                    Layout.fillHeight: true
                    color: "#27272a" // Zinc-800
                    border.color: "#3f3f46" // Zinc-700
                    border.width: 1
                    radius: 6

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        // Avatar
                        Rectangle {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            color: "#7c3aed" // Violet-600
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: "👤"
                                font.pixelSize: 18
                            }
                        }

                        // Patient Info
                        Column {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: "DOE, JOHN"
                                font.pixelSize: 13
                                font.bold: true
                                color: root.colorText
                            }

                            Text {
                                text: "BED-04"
                                font.pixelSize: 10
                                font.family: "monospace"
                                color: root.colorECG
                            }
                        }
                    }
                }

                // Spacer
                Item {
                    Layout.fillWidth: true
                }

                // User Info
                Column {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2

                    Text {
                        text: "SARAH JOHNSON"
                        font.pixelSize: 11
                        font.bold: true
                        color: root.colorTextMuted
                        horizontalAlignment: Text.AlignRight
                    }

                    Text {
                        text: "CLINICIAN"
                        font.pixelSize: 9
                        font.bold: true
                        font.letterSpacing: 1.5
                        color: "#3f3f46" // Zinc-700
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // Clock & Date
                Rectangle {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true
                    color: "transparent"

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            id: clockText
                            text: Qt.formatTime(new Date(), "HH:mm:ss")
                            font.pixelSize: 12
                            font.family: "monospace"
                            color: root.colorTextMuted

                            Timer {
                                interval: 1000
                                running: true
                                repeat: true
                                onTriggered: clockText.text = Qt.formatTime(new Date(), "HH:mm:ss")
                            }
                        }

                        Rectangle {
                            width: 1
                            height: 14
                            color: "#3f3f46" // Zinc-700
                        }

                        Text {
                            text: Qt.formatDate(new Date(), "dd MMM yyyy").toUpperCase()
                            font.pixelSize: 12
                            font.family: "monospace"
                            color: "#52525b" // Zinc-600
                        }
                    }
                }

                // Connection Status
                ConnectionStatus {
                    Layout.preferredWidth: 220
                    Layout.preferredHeight: 36
                    connected: dashboardController.isMonitoring
                    sensorType: "Shared Memory"
                }

                // Divider
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: root.colorBorder
                }

                // Connection Status
                Row {
                    spacing: 8

                    Text {
                        text: "📡"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        spacing: 2

                        Text {
                            text: "CMS ONLINE"
                            font.pixelSize: 10
                            font.bold: true
                            color: root.colorTextMuted
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

                    Text {
                        text: "🔋"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "98%"
                        font.pixelSize: 11
                        font.family: "monospace"
                        font.bold: true
                        color: root.colorTextMuted
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                // Notification Bell
                Rectangle {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    color: root.showNotifications ? root.colorBorder : "transparent"
                    radius: 20

                    Text {
                        anchors.centerIn: parent
                        text: "🔔"
                        font.pixelSize: 20
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.showNotifications = !root.showNotifications
                    }
                }
            }
        }

        // Main Content Area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Loader {
                id: contentLoader
                anchors.fill: parent
                anchors.margins: 4
                source: {
                    switch (root.currentView) {
                    case root.ViewState.Monitor:
                        return "views/MonitorView.qml";
                    case root.ViewState.Trends:
                        return "views/TrendsView.qml";
                    case root.ViewState.Settings:
                        return "views/SettingsView.qml";
                    default:
                        return "views/MonitorView.qml";
                    }
                }
            }
        }

        // Bottom Navigation Bar - 64px
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: root.colorPanel
            border.color: root.colorBorder
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                // Left Navigation Buttons
                Row {
                    spacing: 4

                    NavButton {
                        text: "MONITOR"
                        icon: "📊"
                        active: root.currentView === root.ViewState.Monitor
                        activeColor: root.colorECG
                        onClicked: root.currentView = root.ViewState.Monitor
                    }

                    NavButton {
                        text: "AI ANALYSIS"
                        icon: "🧠"
                        active: root.currentView === root.ViewState.Analysis
                        activeColor: "#a855f7" // Purple-500
                        onClicked: root.currentView = root.ViewState.Analysis
                    }

                    NavButton {
                        text: "TRENDS"
                        icon: "📈"
                        active: root.currentView === root.ViewState.Trends
                        activeColor: root.colorSPO2
                        onClicked: root.currentView = root.ViewState.Trends
                    }

                    Rectangle {
                        width: 1
                        height: 48
                        color: root.colorBorder
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // Code Blue Button
                    Rectangle {
                        width: 120
                        height: 48
                        color: "#450a0a" // Red-950
                        border.color: "#7f1d1d" // Red-900
                        border.width: 1
                        radius: 4

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "🚨"
                                font.pixelSize: 20
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "CODE BLUE"
                                font.pixelSize: 9
                                font.bold: true
                                font.letterSpacing: 1
                                color: root.colorCritical
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: console.log("Code Blue activated!")
                        }
                    }
                }

                // Spacer
                Item {
                    Layout.fillWidth: true
                }

                // Right Navigation Buttons
                Row {
                    spacing: 4

                    NavButton {
                        text: "MENU"
                        icon: "⚙️"
                        active: root.currentView === root.ViewState.Settings
                        onClicked: root.currentView = root.ViewState.Settings
                    }

                    NavButton {
                        text: "LOGOUT"
                        icon: "🚪"
                        onClicked: console.log("Logout clicked")
                    }
                }
            }
        }
    }
}
