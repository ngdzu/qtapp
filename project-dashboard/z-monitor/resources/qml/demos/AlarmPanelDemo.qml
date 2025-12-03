import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

pragma ComponentBehavior: Bound

ApplicationWindow {
    id: app
    width: 960
    height: 540
    visible: true
    title: "AlarmPanel Demo"

    // Note: run with a non-native style if you want custom Button visuals
    // Example: QT_QUICK_CONTROLS_STYLE=Basic

    // Mock AlarmController implementing expected API surface
    QtObject {
        id: mockAlarmController
        property var activeAlarms: [
            { id: 1, type: "HR", priority: "critical", message: "Heart Rate > 180", timestamp: Date.now(), currentValue: 182, threshold: 160, acknowledged: false },
            { id: 2, type: "SpO2", priority: "major", message: "SpO2 < 88%", timestamp: Date.now() - 20000, currentValue: 86, threshold: 90, acknowledged: false },
            { id: 3, type: "TEMP", priority: "minor", message: "Temp High", timestamp: Date.now() - 40000, currentValue: 38.2, threshold: 37.5, acknowledged: true }
        ]
        property int activeAlarmCount: activeAlarms.length
        property bool hasCriticalAlarms: activeAlarms.some(a => a.priority === "critical" && !a.acknowledged)
        property bool hasWarningAlarms: activeAlarms.some(a => (a.priority === "major" || a.priority === "minor") && !a.acknowledged)
        property var alarmHistory: []

        // QML automatically creates change signals: activeAlarmsChanged, hasCriticalAlarmsChanged, hasWarningAlarmsChanged

        function acknowledgeAlarm(id) {
            for (var i = 0; i < activeAlarms.length; i++) {
                if (activeAlarms[i].id === id) {
                    activeAlarms[i].acknowledged = true
                    activeAlarmsChanged()
                    hasCriticalAlarmsChanged()
                    hasWarningAlarmsChanged()
                    return true
                }
            }
            return false
        }
        function silenceAlarm(id, minutes) {
            // No-op mock: mark as acknowledged for demo
            return acknowledgeAlarm(id)
        }
        function acknowledgeAllAlarms() {
            for (var i = 0; i < activeAlarms.length; i++) {
                activeAlarms[i].acknowledged = true
            }
            activeAlarmsChanged()
            hasCriticalAlarmsChanged()
            hasWarningAlarmsChanged()
        }
        function loadAlarmHistory() {
            alarmHistory = activeAlarms.concat(alarmHistory)
            return alarmHistory
        }

    }

    // Simulate incoming new alarm after 3s
    Timer {
        interval: 3000; running: true; repeat: false
        onTriggered: {
            mockAlarmController.activeAlarms.unshift({ id: 4, type: "RESP", priority: "major", message: "Resp Rate Low", timestamp: Date.now(), currentValue: 6, threshold: 10, acknowledged: false })
            mockAlarmController.activeAlarmsChanged()
            mockAlarmController.hasWarningAlarmsChanged()
        }
    }

    AlarmPanel {
        id: panel
        anchors.fill: parent
        audioEnabled: false // disable audio for demo unless assets added
        alarmController: mockAlarmController
    }
}
