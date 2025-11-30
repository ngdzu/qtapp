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
    property color accentColor: "#10b981"
    property string subValue: ""

    // Match sample_app VitalCard layout (Column with proper spacing, no justify-between)
    Column {
        anchors.fill: parent
        anchors.margins: 12 // Theme.spacingLg equivalent
        spacing: 8 // Theme.spacingSm equivalent

        // Label row (top)
        Text {
            text: root.label
            font.pixelSize: 11 // text-xs
            font.bold: true
            font.letterSpacing: 1.5
            color: root.accentColor
        }

        // Value + Unit row (below label with spacing) - using reusable VitalValue component
        VitalValue {
            id: vitalValue
            value: root.value
            unit: root.unit
            accentColor: root.accentColor
        }

        // SubValue (if provided)
        Text {
            text: root.subValue
            font.pixelSize: 14 // text-sm
            color: "#a1a1aa" // Zinc-400
            visible: root.subValue !== ""
        }
    }
}
