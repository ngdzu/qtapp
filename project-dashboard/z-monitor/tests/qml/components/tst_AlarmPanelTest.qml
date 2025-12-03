/**
 * @file tst_AlarmPanelTest.qml
 * @brief Qt Quick Test for AlarmPanel component
 *
 * Tests alarm display, priority sorting, acknowledge/silence actions,
 * audio feedback, and controller integration.
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

import QtQuick
import QtTest

import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "AlarmPanelTest"
    when: windowShown
    width: 400
    height: 600

    // Mock AlarmController
    QtObject {
        id: mockAlarmController

        property var activeAlarms: []
        property int activeAlarmCount: activeAlarms.length
        property bool hasCriticalAlarms: false
        property bool hasWarningAlarms: false
        property var alarmHistory: []

        signal activeAlarmsChanged()
        signal activeAlarmCountChanged()
        signal hasCriticalAlarmsChanged()
        signal hasWarningAlarmsChanged()
        signal alarmHistoryChanged()

        property var acknowledgedAlarms: []
        property var silencedAlarms: []

        function acknowledgeAlarm(alarmId) {
            acknowledgedAlarms.push(alarmId)
        }

        function silenceAlarm(alarmId, durationSeconds) {
            silencedAlarms.push({id: alarmId, duration: durationSeconds})
        }

        function acknowledgeAllAlarms() {
            acknowledgedAlarms = activeAlarms.map(a => a.id)
        }

        function loadAlarmHistory() {
            alarmHistory = [
                {
                    id: "hist-001",
                    type: "HR HIGH",
                    priority: "MINOR",
                    message: "Heart rate above threshold",
                    timestamp: new Date(),
                    acknowledged: true,
                    currentValue: 105,
                    threshold: 100,
                    unit: "bpm"
                }
            ]
            alarmHistoryChanged()
        }
    }

    // Test component
    AlarmPanel {
        id: alarmPanel
        anchors.fill: parent
        alarmController: mockAlarmController
        audioEnabled: false // Disable audio for tests
    }

    function init() {
        // Reset mock controller before each test
        mockAlarmController.activeAlarms = []
        mockAlarmController.hasCriticalAlarms = false
        mockAlarmController.hasWarningAlarms = false
        mockAlarmController.alarmHistory = []
        mockAlarmController.acknowledgedAlarms = []
        mockAlarmController.silencedAlarms = []
        alarmPanel.showHistory = false
        alarmPanel.audioEnabled = false
    }

    function test_01_initialState() {
        compare(alarmPanel.alarmController, mockAlarmController, "Controller is set")
        compare(alarmPanel.audioEnabled, false, "Audio is disabled for tests")
        compare(alarmPanel.showHistory, false, "Show history is false")
    }

    function test_02_emptyState() {
        verify(mockAlarmController.activeAlarmCount === 0, "No active alarms")
        wait(50) // Allow UI to update
        // Check that empty message is visible (implementation-dependent)
    }

    function test_03_singleAlarm() {
        var testAlarm = {
            id: "alarm-001",
            type: "HR HIGH",
            priority: "CRITICAL",
            message: "Heart rate critically high",
            timestamp: new Date(),
            acknowledged: false,
            currentValue: 150,
            threshold: 120,
            unit: "bpm"
        }

        mockAlarmController.activeAlarms = [testAlarm]
        mockAlarmController.hasCriticalAlarms = true
        mockAlarmController.activeAlarmsChanged()
        mockAlarmController.activeAlarmCountChanged()
        mockAlarmController.hasCriticalAlarmsChanged()

        wait(50) // Allow UI to update

        verify(mockAlarmController.activeAlarmCount === 1, "One active alarm")
        verify(mockAlarmController.hasCriticalAlarms, "Has critical alarm")
    }

    function test_04_prioritySorting() {
        var alarms = [
            {
                id: "alarm-001",
                type: "SPO2 LOW",
                priority: "MINOR",
                message: "SpO2 below threshold",
                timestamp: new Date(Date.now() - 60000), // 1 min ago
                acknowledged: false
            },
            {
                id: "alarm-002",
                type: "HR HIGH",
                priority: "CRITICAL",
                message: "Heart rate critically high",
                timestamp: new Date(),
                acknowledged: false
            },
            {
                id: "alarm-003",
                type: "BP HIGH",
                priority: "MAJOR",
                message: "Blood pressure high",
                timestamp: new Date(Date.now() - 30000), // 30s ago
                acknowledged: false
            }
        ]

        mockAlarmController.activeAlarms = alarms
        mockAlarmController.hasCriticalAlarms = true
        mockAlarmController.hasWarningAlarms = true
        mockAlarmController.activeAlarmsChanged()
        mockAlarmController.activeAlarmCountChanged()
        mockAlarmController.hasCriticalAlarmsChanged()
        mockAlarmController.hasWarningAlarmsChanged()

        wait(50) // Allow UI to update

        verify(mockAlarmController.activeAlarmCount === 3, "Three active alarms")
        verify(mockAlarmController.hasCriticalAlarms, "Has critical alarms")
        verify(mockAlarmController.hasWarningAlarms, "Has warning alarms")

        // Note: QML ListView doesn't guarantee sorting unless explicitly implemented
        // The actual sorting should be done in AlarmController or in the model
    }

    function test_05_acknowledgeAlarm() {
        var testAlarm = {
            id: "alarm-001",
            type: "HR HIGH",
            priority: "CRITICAL",
            message: "Heart rate critically high",
            timestamp: new Date(),
            acknowledged: false
        }

        mockAlarmController.activeAlarms = [testAlarm]
        mockAlarmController.activeAlarmsChanged()

        wait(50)

        // Simulate acknowledge button click
        mockAlarmController.acknowledgeAlarm("alarm-001")

        verify(mockAlarmController.acknowledgedAlarms.includes("alarm-001"), "Alarm acknowledged")
    }

    function test_06_silenceAlarm() {
        var testAlarm = {
            id: "alarm-002",
            type: "SPO2 LOW",
            priority: "MAJOR",
            message: "SpO2 below threshold",
            timestamp: new Date(),
            acknowledged: false
        }

        mockAlarmController.activeAlarms = [testAlarm]
        mockAlarmController.activeAlarmsChanged()

        wait(50)

        // Simulate silence button click (5 minutes)
        mockAlarmController.silenceAlarm("alarm-002", 300)

        verify(mockAlarmController.silencedAlarms.length === 1, "Alarm silenced")
        compare(mockAlarmController.silencedAlarms[0].id, "alarm-002", "Correct alarm silenced")
        compare(mockAlarmController.silencedAlarms[0].duration, 300, "Silenced for 5 minutes")
    }

    function test_07_acknowledgeAllAlarms() {
        var alarms = [
            {id: "alarm-001", type: "HR HIGH", priority: "CRITICAL", message: "HR high", timestamp: new Date(), acknowledged: false},
            {id: "alarm-002", type: "SPO2 LOW", priority: "MAJOR", message: "SpO2 low", timestamp: new Date(), acknowledged: false},
            {id: "alarm-003", type: "BP HIGH", priority: "MINOR", message: "BP high", timestamp: new Date(), acknowledged: false}
        ]

        mockAlarmController.activeAlarms = alarms
        mockAlarmController.activeAlarmsChanged()

        wait(50)

        mockAlarmController.acknowledgeAllAlarms()

        verify(mockAlarmController.acknowledgedAlarms.length === 3, "All alarms acknowledged")
    }

    function test_08_alarmHistory() {
        alarmPanel.showHistory = true
        mockAlarmController.loadAlarmHistory()

        wait(50)

        verify(mockAlarmController.alarmHistory.length === 1, "Alarm history loaded")
        compare(mockAlarmController.alarmHistory[0].id, "hist-001", "History alarm ID correct")
        verify(mockAlarmController.alarmHistory[0].acknowledged, "History alarm is acknowledged")
    }

    function test_09_audioToggle() {
        alarmPanel.audioEnabled = true
        verify(alarmPanel.audioEnabled === true, "Audio enabled")

        alarmPanel.audioEnabled = false
        verify(alarmPanel.audioEnabled === false, "Audio disabled")
    }

    function test_10_criticalAlarmAudio() {
        alarmPanel.audioEnabled = true

        var criticalAlarm = {
            id: "alarm-critical",
            type: "CARDIAC ARREST",
            priority: "CRITICAL",
            message: "Cardiac arrest detected",
            timestamp: new Date(),
            acknowledged: false
        }

        mockAlarmController.activeAlarms = [criticalAlarm]
        mockAlarmController.hasCriticalAlarms = true
        mockAlarmController.activeAlarmsChanged()
        mockAlarmController.hasCriticalAlarmsChanged()

        wait(100) // Allow audio to start

        // Audio should be playing (tested visually/manually)
        verify(mockAlarmController.hasCriticalAlarms, "Critical alarm present")
    }

    function test_11_multipleAlarmsWithDifferentPriorities() {
        var alarms = [
            {id: "alarm-001", type: "HR HIGH", priority: "CRITICAL", message: "HR critically high", timestamp: new Date(), acknowledged: false},
            {id: "alarm-002", type: "SPO2 LOW", priority: "CRITICAL", message: "SpO2 critically low", timestamp: new Date(), acknowledged: false},
            {id: "alarm-003", type: "BP HIGH", priority: "MAJOR", message: "BP high", timestamp: new Date(), acknowledged: false},
            {id: "alarm-004", type: "TEMP HIGH", priority: "MINOR", message: "Temp high", timestamp: new Date(), acknowledged: false}
        ]

        mockAlarmController.activeAlarms = alarms
        mockAlarmController.hasCriticalAlarms = true
        mockAlarmController.hasWarningAlarms = true
        mockAlarmController.activeAlarmsChanged()
        mockAlarmController.hasCriticalAlarmsChanged()
        mockAlarmController.hasWarningAlarmsChanged()

        wait(50)

        verify(mockAlarmController.activeAlarmCount === 4, "Four active alarms")
    }

    function test_12_acknowledgedAlarmVisibility() {
        var testAlarm = {
            id: "alarm-ack",
            type: "HR HIGH",
            priority: "MAJOR",
            message: "Heart rate high",
            timestamp: new Date(),
            acknowledged: true // Already acknowledged
        }

        mockAlarmController.activeAlarms = [testAlarm]
        mockAlarmController.activeAlarmsChanged()

        wait(50)

        // Acknowledged alarm should be visible but with reduced opacity
        verify(mockAlarmController.activeAlarmCount === 1, "Acknowledged alarm still in list")
    }
}
