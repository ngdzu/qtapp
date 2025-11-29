/**
 * @file MonitorView.qml
 * @brief Main patient monitoring dashboard view
 *
 * Displays waveforms (left 8 cols) and vital signs (right 4 cols)
 * Converted from Node.js MonitorView.tsx
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/qml/components"

Item {
    id: root

    // Grid layout: 8 cols waveforms + 4 cols vitals
    RowLayout {
        anchors.fill: parent
        spacing: 4

        // Left Column: Waveforms (2/3 width)
        Column {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width * 0.67
            spacing: 4

            // ECG Panel
            WaveformPanel {
                width: parent.width
                height: (root.height - 8) / 3
                label: "II ECG"
                labelColor: "#10b981" // Green
                waveformColor: "#10b981"
                showFilter: true
            }

            // Pleth Panel
            WaveformPanel {
                width: parent.width
                height: (root.height - 8) / 3
                label: "PLETH"
                labelColor: "#3b82f6" // Blue
                waveformColor: "#3b82f6"
            }

            // Resp Panel
            WaveformPanel {
                width: parent.width
                height: (root.height - 8) / 3
                label: "RESP"
                labelColor: "#eab308" // Yellow
                waveformColor: "#eab308"
            }
        }

        // Right Column: Vital Signs (1/3 width)
        Column {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width * 0.33
            spacing: 4

            // Heart Rate Tile
            VitalTile {
                width: parent.width
                height: (root.height - 12) / 4
                label: "HEART RATE"
                value: "85"
                unit: "BPM"
                color: "#10b981" // Green
                subValue: "PVC: 0"
            }

            // SpO2 Tile
            VitalTile {
                width: parent.width
                height: (root.height - 12) / 4
                label: "SPO2"
                value: "97"
                unit: "%"
                color: "#3b82f6" // Blue
                subValue: "PI: 2.4"
            }

            // NIBP Tile
            VitalTile {
                width: parent.width
                height: (root.height - 12) / 4
                label: "NIBP"
                value: "123/76"
                unit: "mmHg"
                color: "#a1a1aa" // Zinc-400
                subValue: "MAP: 93"
            }

            // Resp Rate Tile
            VitalTile {
                width: parent.width
                height: (root.height - 12) / 4
                label: "RESP RATE"
                value: "15"
                unit: "rpm"
                color: "#eab308" // Yellow
            }

            // Temp Tile (if space allows)
            VitalTile {
                width: parent.width
                height: (root.height - 12) / 4
                label: "TEMP"
                value: "36.8"
                unit: "°C"
                color: "#d946ef" // Fuchsia
                subValue: "T2: 36.5"
            }
        }
    }
}
