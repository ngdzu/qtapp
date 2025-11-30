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
import qml 1.0

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
    color: Theme.colors.background // Zinc-950

    // Theme shortcut aliases
    readonly property color colorBackground: Theme.colors.background
    readonly property color colorPanel: Theme.colors.panel
    readonly property color colorBorder: Theme.colors.border
    readonly property color colorText: Theme.colors.text
    readonly property color colorTextMuted: Theme.colors.textMuted
    readonly property color colorECG: Theme.colors.ECG
    readonly property color colorSPO2: Theme.colors.SPO2
    readonly property color colorResp: Theme.colors.RESP
    readonly property color colorNIBP: Theme.colors.NIBP
    readonly property color colorTemp: Theme.colors.TEMP
    readonly property color colorCritical: Theme.colors.critical
    readonly property color colorWarning: Theme.colors.warning
    readonly property color colorSuccess: Theme.colors.success

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
                    color: Theme.colors.banner
                    border.color: Theme.colors.bannerBorder
                    border.width: 1
                    radius: 6

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        // Avatar
                        Image {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            source: "qrc:/qml/icons/avatar.svg"
                            fillMode: Image.PreserveAspectFit
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
                                font.family: Theme.fonts.mono
                                color: root.colorECG
                            }
                        }
                    }
                }

                // Spacer
                Item {
                    Layout.fillWidth: true
                }

                // Clinician Info
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
                        color: Theme.colors.clinicianSubtle
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
                            font.family: Theme.fonts.mono
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
                            font.family: Theme.fonts.mono
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

                    Image {
                        source: "qrc:/qml/icons/antenna.svg"
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

                    Image {
                        source: "qrc:/qml/icons/battery.svg"
                        width: 18
                        height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        text: "98%"
                        font.pixelSize: 11
                        font.family: Theme.fonts.mono
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

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/qml/icons/bell.svg"
                        width: 20
                        height: 20
                        fillMode: Image.PreserveAspectFit
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
