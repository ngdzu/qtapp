/**
 * @file MonitorView.qml
 * @brief Main patient monitoring dashboard view
 *
 * Displays waveforms (left 8 cols) and vital signs (right 4 cols)
 * Binds to dashboardController and waveformController for live data
 * Converted from Node.js MonitorView.tsx - exact grid structure match
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/qml/components"

Item {
    id: root

    // Grid layout matching React grid-cols-12 gap-1
    RowLayout {
        anchors.fill: parent
        spacing: 4 // gap-1 in Tailwind (4px)

        // Left Column: Waveforms (col-span-8 = 8/12 = 66.67%)
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.6667
            spacing: 4 // gap-1

            // ECG Panel (flex-1)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#18181b" // medical-panel
                border.color: "#27272a" // zinc-800
                border.width: 1
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8 // p-2
                    spacing: 4 // mb-1

                    // Header row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "II ECG"
                            font.pixelSize: 14 // text-sm
                            font.bold: true
                            font.letterSpacing: 1.5 // tracking-wider
                            color: "#10b981" // emerald-500
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Text {
                            text: "FILTER: MON"
                            font.pixelSize: 11 // text-xs
                            color: "#52525b" // zinc-600
                        }
                    }

                    // Waveform area
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        WaveformDisplay {
                            anchors.fill: parent
                            label: "II ECG"
                            waveformColor: "#10b981"
                            gridColor: "#27272a40"
                            waveformData: waveformController.ecgData
                            showQuality: true
                        }
                    }
                }
            }

            // SPO2 Panel (Pleth) (flex-1)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#18181b"
                border.color: "#27272a"
                border.width: 1
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Text {
                        text: "PLETH"
                        font.pixelSize: 14
                        font.bold: true
                        font.letterSpacing: 1.5
                        color: "#3b82f6" // blue-500
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        WaveformDisplay {
                            anchors.fill: parent
                            label: "PLETH"
                            waveformColor: "#3b82f6"
                            gridColor: "#27272a40"
                            waveformData: waveformController.plethData
                            showQuality: true
                        }
                    }
                }
            }

            // Resp Panel (flex-1)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#18181b"
                border.color: "#27272a"
                border.width: 1
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Text {
                        text: "RESP"
                        font.pixelSize: 14
                        font.bold: true
                        font.letterSpacing: 1.5
                        color: "#eab308" // yellow-500
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        WaveformDisplay {
                            anchors.fill: parent
                            label: "RESP"
                            waveformColor: "#eab308"
                            gridColor: "#27272a40"
                            waveformData: waveformController.respData
                            showQuality: true
                        }
                    }
                }
            }
        }

        // Right Column: Vital Signs (col-span-4 = 4/12 = 33.33%)
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.3333
            spacing: 4 // gap-1

            // HR Tile (flex-1)
            VitalTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                label: "HEART RATE"
                value: dashboardController.heartRate > 0 ? dashboardController.heartRate.toString() : "--"
                unit: "BPM"
                accentColor: "#10b981" // ECG green
                subValue: "PVC: 0"
            }

            // SpO2 Tile (flex-1)
            VitalTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                label: "SPO2"
                value: dashboardController.spo2 > 0 ? dashboardController.spo2.toString() : "--"
                unit: "%"
                accentColor: "#3b82f6" // SPO2 blue
                subValue: "PI: 2.4"
            }

            // NIBP Tile (flex-1)
            VitalTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                label: "NIBP"
                value: dashboardController.bloodPressure !== "" ? dashboardController.bloodPressure : "--/--"
                unit: "mmHg"
                accentColor: "#71717a" // TEXT_MUTED from constants.ts
                subValue: "MAP: 93"
            }

            // Resp Rate Tile (flex-1)
            VitalTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                label: "RESP RATE"
                value: dashboardController.respiratoryRate > 0 ? dashboardController.respiratoryRate.toString() : "--"
                unit: "rpm"
                accentColor: "#eab308" // RESP yellow
            }

            // Temp Tile (flex-1)
            VitalTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                label: "TEMP"
                value: dashboardController.temperature > 0 ? dashboardController.temperature.toFixed(1) : "--"
                unit: "°C"
                accentColor: "#ffffff" // white from React
                subValue: "T2: 36.5"
            }
        }
    }
}
