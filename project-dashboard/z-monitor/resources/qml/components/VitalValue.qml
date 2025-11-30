/**
 * @file VitalValue.qml
 * @brief Reusable component for displaying vital sign value and unit
 *
 * Displays a large numeric value with unit label, styled with accent color.
 * Can be used independently or as part of VitalTile.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

import QtQuick
import QtQuick.Controls

/**
 * @brief Reusable vital value display component
 *
 * Displays a large numeric value with unit, styled with accent color.
 * Matches sample_app VitalCard value display style.
 */
Row {
    id: root
    
    property string value: "0"           ///< Numeric value to display
    property string unit: ""            ///< Unit label (e.g., "BPM", "%", "mmHg")
    property color accentColor: "#10b981"  ///< Accent color for value text
    property int valueFontSize: 36      ///< Font size for value (default: 36px)
    property int unitFontSize: 14       ///< Font size for unit (default: 14px)
    property string fontFamily: "monospace"  ///< Font family for value
    
    spacing: 4 // space-x-1 equivalent
    
    Text {
        id: valueText
        text: root.value
        font.pixelSize: root.valueFontSize
        font.family: root.fontFamily
        font.bold: true
        color: root.accentColor
    }
    
    Text {
        id: unitText
        text: root.unit
        font.pixelSize: root.unitFontSize
        font.weight: Font.Medium
        color: "#71717a" // Zinc-500
        anchors.baseline: valueText.baseline
    }
}

