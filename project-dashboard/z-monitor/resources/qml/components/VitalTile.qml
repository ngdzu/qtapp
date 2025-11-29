/**
 * @file VitalTile.qml
 * @brief Reusable vital signs display tile component
 *
 * Displays a single vital sign with large numeric value, unit, and optional subvalue
 * Converted from Node.js VitalTile.tsx
 */

import QtQuick
import QtQuick.Controls

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

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Label
        Text {
            text: root.label
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 1.2
            color: root.color
        }

        // Spacer
        Item {
            height: 1
            width: 1
        }

        // Value + Unit
        Row {
            spacing: 6
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                text: root.value
                font.pixelSize: 48
                font.family: "monospace"
                font.bold: true
                color: root.color
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.unit
                font.pixelSize: 13
                font.weight: Font.Medium
                color: "#71717a" // Zinc-500
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
            }
        }

        // Spacer
        Item {
            Layout.fillHeight: true
        }

        // SubValue
        Text {
            text: root.subValue
            font.pixelSize: 12
            color: "#a1a1aa" // Zinc-400
            visible: root.subValue !== ""
        }
    }
}
