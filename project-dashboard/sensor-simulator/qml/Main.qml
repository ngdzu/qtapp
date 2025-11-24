import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import "components"
import qml 1.0

Window {
    id: window
    
    width: 1000
    height: 700
    visible: true
    title: "Sensor Simulator"
    color: Theme.background

    // Intercept window close and ask the user to confirm.
    onClosing: function(event) {
        event.accepted = false
        exitDialog.open()
    }

    // Application state
    property string filterLevel: "All"
    property int logRetention: 50
    property int messageCounter: 0

    // Live vitals populated from the Simulator C++ backend
    property int hr: 72
    property int spo2: 98
    property int rr: 16

    function pushLog(level, text, source) {
        var m = logConsole.logModel
        window.messageCounter += 1
        var id = window.messageCounter
        var now = new Date()
        var timeStr = now.toLocaleTimeString()
        m.append({ "id": id, "time": timeStr, "sensor": source ? source : "-", "message": text, "level": level })
        while (m.count > window.logRetention) m.remove(0)
    }

    // Main layout
    Rectangle {
        anchors.fill: parent
        color: Theme.backgroundSecondary

        // Top bar
        TopBar {
            id: topBar
            anchors.top: parent.top
            lastEventText: window.lastEventText
            lastEventColor: window.lastEventColor
        }

        // Modal overlay (dim the app when any dialog is visible)
        Rectangle {
            id: modalOverlay
            anchors.fill: parent
            color: Theme.colorWithOpacity(Theme.background, 0.45)
            visible: false
            z: 998
        }

        RowLayout {
            anchors.top: topBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: Theme.spacingMd
            anchors.margins: Theme.spacingMd

            // Left column (controls) - 1/4 width
            ColumnLayout {
                Layout.preferredWidth: Theme.controlPanelWidth
                Layout.maximumWidth: Theme.controlPanelWidth
                Layout.minimumWidth: Theme.controlPanelMinWidth
                Layout.fillWidth: false
                spacing: Theme.spacingLg
                
                ControlPanel {
                    id: controlPanel
                    onTriggerCritical: simulator.triggerCritical()
                    onTriggerWarning: simulator.triggerWarning()
                    onTriggerNotification: simulator.triggerNotification("Manual notification")
                    onPlayDemo: simulator.playDemo()
                }
                
                DebugOutput {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            // Right Column (Visualization & Logs)
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: Theme.spacingLg

                // Vitals row (three cards)
                VitalsDisplay {
                    id: vitalsDisplay
                    heartRate: window.hr
                    spo2: window.spo2
                    respRate: window.rr
                }

                // Waveform Chart
                WaveformChart {
                    id: waveformChart
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.waveformHeight
                }

                // Log Console
                LogConsole {
                    id: logConsole
                    filterLevel: window.filterLevel
                    onClearRequested: {
                        logConsole.logModel.clear()
                        window.messageCounter = 0
                    }
                }
            }
        }
    }

    // Last event state (for TopBar)
    property string lastEventText: "-- No events --"
    property color lastEventColor: Theme.textMuted

    Connections {
        target: simulator
        function onAlarmTriggered(level) {
            window.lastEventText = "ALARM(" + level + ")"
            window.lastEventColor = Theme.error
            pushLog(level, "ALARM: " + level)
        }
        function onNotification(text) {
            window.lastEventText = "NOTIFY(" + text + ")"
            window.lastEventColor = Theme.info
            pushLog("Info", "NOTIFY: " + text)
        }
        function onVitalsUpdated(newHr, newSpo2, newRr) {
            // Update window-scoped vitals properties so UI cards refresh
            window.hr = newHr
            window.spo2 = newSpo2
            window.rr = newRr
        }
        function onWaveformUpdated(samples) {
            // Feed waveform data to the chart component
            waveformChart.addSamples(samples)
        }
        function onLogEmitted(level, text) {
            pushLog(level, text)
        }
    }

    // Exit confirmation dialog
    ExitDialog {
        id: exitDialog
        parent: window
        onExitConfirmed: simulator.quitApp()
    }

    Binding {
        target: modalOverlay
        property: "visible"
        value: exitDialog.visible
    }
}
