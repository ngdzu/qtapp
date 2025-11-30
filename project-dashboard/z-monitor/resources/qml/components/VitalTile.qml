/**
 * @file VitalTile.qml
 * @brief Reusable vital signs display tile component
 *
 * Displays a single vital sign with large numeric value, unit, and optional subvalue
 * Converted from Node.js VitalTile.tsx - exact pixel match
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#18181b" // Zinc-900 (medical-panel)
    border.color: "#27272a" // Zinc-800
    border.width: 1
    radius: 4

    property string label: "VITAL"
    property string value: "0"
    property string unit: ""
    property color color: "#10b981"
    property string subValue: ""

    // Match React flex-col justify-between p-2 h-full structure
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8 // p-2 in Tailwind
        spacing: 0

        // Label (text-xs uppercase font-bold tracking-wider)
        Text {
            text: root.label
            font.pixelSize: 11 // text-xs
            font.bold: true
            font.letterSpacing: 1.5
            color: root.color
            Layout.alignment: Qt.AlignTop
        }

        // Spacer to push content to bottom (justify-between)
        Item {
            Layout.fillHeight: true
        }

        // Value + Unit (flex items-baseline space-x-1)
        Row {
            spacing: 4 // space-x-1
            Layout.alignment: Qt.AlignLeft

            Text {
                text: root.value
                font.pixelSize: 48 // text-5xl
                font.family: "monospace"
                font.bold: true
                color: root.color
                anchors.baseline: parent.baseline
            }

            Text {
                text: root.unit
                font.pixelSize: 14 // text-sm
                font.weight: Font.Medium
                color: "#71717a" // Zinc-500
                anchors.baseline: parent.baseline
            }
        }

        // SubValue (text-sm text-zinc-400)
        Text {
            text: root.subValue
            font.pixelSize: 14 // text-sm
            color: "#a1a1aa" // Zinc-400
            visible: root.subValue !== ""
            Layout.alignment: Qt.AlignLeft
        }
    }
}
