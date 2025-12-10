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
import Theme 1.0

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

    // Main Layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header (Top Bar) - 48px
        TopBar {
            id: header
            Layout.fillWidth: true
            Layout.preferredHeight: TopBarConstants.height

            // Bind properties
            patientName: patientController.patientName
            bedLocation: patientController.bedLocation
            isMonitoring: dashboardController.isMonitoring
            isConnected: root.isConnected
            batteryLevel: root.batteryLevel
            isCharging: root.isCharging
            hasActiveAlarms: root.hasActiveAlarms
            showNotifications: root.showNotifications

            // Theme colors
            colorPanel: root.colorPanel
            colorBorder: root.colorBorder
            colorText: root.colorText
            colorTextMuted: root.colorTextMuted
            colorECG: root.colorECG
            colorCritical: root.colorCritical
            bannerColor: Theme.colors.banner
            bannerBorderColor: Theme.colors.bannerBorder

            // Handle signals
            onPatientBannerClicked: patientProfileDialog.open()
            onNotificationClicked: root.showNotifications = !root.showNotifications
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
                anchors.margins: 8
                anchors.topMargin: 4
                anchors.bottomMargin: 4
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
        BottomNav {
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            currentView: root.currentView
            viewState: root.viewState

            // Theme colors
            colorPanel: root.colorPanel
            colorBorder: root.colorBorder
            colorECG: root.colorECG
            colorSPO2: root.colorSPO2
            colorCritical: root.colorCritical

            onViewChanged: function (newView) {
                root.currentView = newView;
            }

            onCodeBlueClicked: {
                console.log("Code Blue activated!");
            }

            onLogoutClicked: {
                console.log("Logout clicked");
            }
        }
    }

    // Patient Profile Dialog
    PatientProfileDialog {
        id: patientProfileDialog
    }
}
