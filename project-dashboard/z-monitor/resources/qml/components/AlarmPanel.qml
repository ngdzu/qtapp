import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 360
    height: 480

    // Controller providing alarm data and actions
    // Expected properties: activeAlarms (array), alarmHistory (array)
    // Expected methods: acknowledgeAlarm(id), silenceAlarm(id, seconds)
    property var alarmController: null
    property bool showHistory: false
    // Enable or disable alarm sounds in the panel
    property bool audioEnabled: true

        // Alarm list
        ListView {
            id: alarmListView
            anchors.fill: parent
            clip: true
            spacing: 4

            model: root.alarmController ? (root.showHistory ? root.alarmController.alarmHistory : root.alarmController.activeAlarms) : []

            // Empty state
            Text {
                anchors.centerIn: parent
                visible: alarmListView.count === 0
                text: root.showHistory ? "No alarm history" : "No active alarms"
                font.pixelSize: 14
                color: "#71717a" // Zinc-500
            }

            delegate: Rectangle {
                id: alarmItem
                width: alarmListView.width
                height: alarmContent.height + 16
                color: {
                    if (modelData.acknowledged) return "#27272a" // Zinc-800
                    switch (modelData.priority) {
                        case "CRITICAL": return "#7f1d1d" // Red-900 with transparency
                        case "MAJOR": return "#7c2d12" // Orange-900
                        case "MINOR": return "#713f12" // Yellow-900
                        default: return "#27272a"
                    }
                }
                border.color: {
                    switch (modelData.priority) {
                        case "CRITICAL": return "#ef4444" // Red-500
                        case "MAJOR": return "#f97316" // Orange-500
                        case "MINOR": return "#eab308" // Yellow-500
                        default: return "#52525b"
                    }
                }
                border.width: modelData.acknowledged ? 1 : 2
                radius: 4
                opacity: modelData.acknowledged ? 0.6 : 1.0

                // Blinking animation for critical unacknowledged alarms
                SequentialAnimation on opacity {
                    running: !modelData.acknowledged && modelData.priority === "CRITICAL"
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.4; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }

                Accessible.role: Accessible.ListItem
                Accessible.name: modelData.type + " alarm"
                Accessible.description: modelData.message

                ColumnLayout {
                    id: alarmContent
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        margins: 8
                    }
                    spacing: 4

                    // Alarm header: type and timestamp
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: modelData.type
                            font.pixelSize: 13
                            font.bold: true
                            color: {
                                switch (modelData.priority) {
                                    case "CRITICAL": return "#fca5a5" // Red-300
                                    case "MAJOR": return "#fdba74" // Orange-300
                                    case "MINOR": return "#fde047" // Yellow-300
                                    default: return "#f4f4f5"
                                }
                            }
                            Layout.fillWidth: true
                        }

                        // Priority badge
                        Rectangle {
                            color: {
                                switch (modelData.priority) {
                                    case "CRITICAL": return "#ef4444"
                                    case "MAJOR": return "#f97316"
                                    case "MINOR": return "#eab308"
                                    default: return "#71717a"
                                }
                            }
                            radius: 3
                            implicitWidth: priorityText.width + 8
                            implicitHeight: 18

                            Text {
                                id: priorityText
                                anchors.centerIn: parent
                                text: modelData.priority
                                font.pixelSize: 10
                                font.bold: true
                                color: "#ffffff"
                            }
                        }

                        Text {
                            text: Qt.formatDateTime(new Date(modelData.timestamp), "hh:mm:ss")
                            font.pixelSize: 11
                            color: "#a1a1aa" // Zinc-400
                        }
                    }

                    // Alarm message
                    Text {
                        text: modelData.message
                        font.pixelSize: 12
                        color: "#e4e4e7" // Zinc-200
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Current value and threshold (if available)
                    RowLayout {
                        visible: modelData.currentValue !== undefined && modelData.threshold !== undefined
                        spacing: 12
                        Layout.fillWidth: true

                        Text {
                            text: "Current: " + modelData.currentValue + (modelData.unit || "")
                            font.pixelSize: 11
                            color: "#fca5a5" // Red-300
                        }

                        Text {
                            text: "Threshold: " + modelData.threshold + (modelData.unit || "")
                            font.pixelSize: 11
                            color: "#a1a1aa" // Zinc-400
                        }
                    }

                    // Action buttons (only for active alarms)
                    RowLayout {
                        visible: !root.showHistory && !modelData.acknowledged
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.topMargin: 4

                        Button {
                            id: acknowledgeButton
                            text: "Acknowledge"
                            font.pixelSize: 11

                            background: Rectangle {
                                color: acknowledgeButton.down ? "#059669" : (acknowledgeButton.hovered ? "#10b981" : "#047857")
                                radius: 4
                            }

                            contentItem: Text {
                                text: acknowledgeButton.text
                                font: acknowledgeButton.font
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                if (root.alarmController) {
                                    root.alarmController.acknowledgeAlarm(modelData.id)
                                }
                            }

                            Accessible.role: Accessible.Button
                            Accessible.name: "Acknowledge alarm"
                        }

                        Button {
                            id: silenceButton
                            text: "Silence 5m"
                            font.pixelSize: 11

                            background: Rectangle {
                                color: silenceButton.down ? "#ca8a04" : (silenceButton.hovered ? "#eab308" : "#a16207")
                                radius: 4
                            }

                            contentItem: Text {
                                text: silenceButton.text
                                font: silenceButton.font
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                if (root.alarmController) {
                                    root.alarmController.silenceAlarm(modelData.id, 300) // 5 minutes
                                }
                            }

                            Accessible.role: Accessible.Button
                            Accessible.name: "Silence alarm for 5 minutes"
                        }
                    }

                    // Acknowledged status
                    Text {
                        visible: modelData.acknowledged
                        text: "✓ Acknowledged"
                        font.pixelSize: 11
                        font.italic: true
                        color: "#10b981" // Green-500
                        Layout.topMargin: 4
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }
    }
