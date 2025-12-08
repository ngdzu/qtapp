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
import "components"

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

    // View states (lowercase property per QML rules)
    readonly property var viewState: ({
            Monitor: 0,
            Trends: 1,
            Analysis: 2,
            Settings: 3,
            PatientProfile: 4
        })

    property int currentView: root.viewState.Monitor
    property bool showNotifications: false
    property int batteryLevel: 98  // Battery percentage (0-100)
    property bool isCharging: false  // Whether battery is charging
    property bool hasActiveAlarms: false  // Whether there are active alarms
    property bool isConnected: true  // Connection status

    // Compute battery icon source based on level and charging state
    function getBatteryIconSource() {
        if (root.isCharging) {
            return "qrc:/qml/icons/battery-charging.svg";
        } else if (root.batteryLevel <= 10) {
            return "qrc:/qml/icons/battery-10.svg";
        } else if (root.batteryLevel <= 20) {
            return "qrc:/qml/icons/battery-20.svg";
        } else if (root.batteryLevel <= 50) {
            return "qrc:/qml/icons/battery-50.svg";
        } else if (root.batteryLevel <= 75) {
            return "qrc:/qml/icons/battery-75.svg";
        } else {
            return "qrc:/qml/icons/battery-100.svg";
        }
    }

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
                PatientBanner {
                    Layout.preferredWidth: 220
                    Layout.fillHeight: true
                    name: patientController.patientName
                    bedLabel: patientController.bedLocation
                    bannerColor: Theme.colors.banner
                    bannerBorderColor: Theme.colors.bannerBorder
                    colorText: root.colorText
                    accentColor: root.colorECG
                    onBannerClicked: root.currentView = root.viewState.PatientProfile
                }

                // Spacer
                Item {
                    Layout.fillWidth: true
                }

                // Clinician Info
                Column {
                    spacing: 6
                    Text {
                        id: clockText
                        text: Qt.formatTime(new Date(), "HH:mm:ss")
                        font.pixelSize: 12
                        font.family: Theme.fonts.mono
                        color: root.colorTextMuted
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
                        font.family: Theme.fonts.mono
                        color: "#52525b" // Zinc-600
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
                        source: root.isConnected ? "qrc:/qml/icons/wifi-emerald.svg" : "qrc:/qml/icons/wifi-off-red.svg"
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
                        source: root.getBatteryIconSource()
                        width: 18
                        height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        text: root.batteryLevel + "%"
                        font.pixelSize: 11
                        font.family: Theme.fonts.mono
                        font.bold: true
                        color: root.batteryLevel <= 20 ? root.colorCritical : root.colorTextMuted
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
                        source: root.hasActiveAlarms ? "qrc:/qml/icons/bell-red.svg" : "qrc:/qml/icons/bell-muted.svg"
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

            Component {
                id: patientProfileComponent
                PatientProfileView {
                    panelColor: root.colorPanel
                    borderColor: root.colorBorder
                    textColor: root.colorText
                    accentColor: root.colorECG
                }
            }

            Loader {
                id: contentLoader
                anchors.fill: parent
                anchors.margins: 4
                source: {
                    switch (root.currentView) {
                    case root.viewState.Monitor:
                        console.info("Main: Loading MonitorView");
                        return "views/MonitorView.qml";
                    case root.viewState.Trends:
                        console.info("Main: Loading TrendsView");
                        return "views/TrendsView.qml";
                    case root.viewState.Analysis:
                        console.info("Main: Loading AnalysisView");
                        return "views/AnalysisView.qml";
                    case root.viewState.Settings:
                        console.info("Main: Loading SettingsView");
                        return "views/SettingsView.qml";
                    case root.viewState.PatientProfile:
                        console.info("Main: Loading PatientProfileView");
                        return "";
                    default:
                        console.info("Main: Loading default MonitorView");
                        return "views/MonitorView.qml";
                    }
                }

                onStatusChanged: {
                    if (status === Loader.Loading) {
                        console.info("Loader: Loading", source);
                    } else if (status === Loader.Ready) {
                        console.info("Loader: Ready", source);
                    } else if (status === Loader.Error) {
                        console.error("Loader: Error loading", source);
                    }
                }
                sourceComponent: currentView === viewState.PatientProfile ? patientProfileComponent : undefined
            }

            // ADT view is loaded via Loader above
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
                        iconBase: "layout-dashboard"
                        iconColorSuffix: "emerald"
                        active: root.currentView === root.viewState.Monitor
                        activeColor: root.colorECG
                        onClicked: root.currentView = root.viewState.Monitor
                    }

                    NavButton {
                        text: "AI ANALYSIS"
                        iconBase: "brain-circuit"
                        iconColorSuffix: "purple"
                        active: root.currentView === root.viewState.Analysis
                        activeColor: "#a855f7" // Purple-500
                        onClicked: root.currentView = root.viewState.Analysis
                    }

                    NavButton {
                        text: "TRENDS"
                        iconBase: "activity"
                        iconColorSuffix: "blue"
                        active: root.currentView === root.viewState.Trends
                        activeColor: root.colorSPO2
                        onClicked: root.currentView = root.viewState.Trends
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

                            Image {
                                anchors.horizontalCenter: parent.horizontalCenter
                                source: "qrc:/qml/icons/siren.svg"
                                width: 20
                                height: 20
                                sourceSize.width: 20
                                sourceSize.height: 20
                                fillMode: Image.PreserveAspectFit
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
                        iconBase: "settings"
                        iconColorSuffix: "white"
                        active: root.currentView === root.viewState.Settings
                        onClicked: root.currentView = root.viewState.Settings
                    }

                    NavButton {
                        text: "LOGOUT"
                        iconBase: "log-out"
                        iconColorSuffix: "gray"
                        active: false
                        onClicked: console.log("Logout clicked")
                    }
                }
            }
        }
    }
}
