import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    property string label: ""
    property int value: 0
    property string unit: ""
    property color accentColor: Theme.accentEmerald
    property string icon: "" // Deprecated - use iconSource instead
    property string iconSource: ""
    
    color: Theme.cardBackground
    radius: Theme.radiusXl
    border.color: Theme.border
    border.width: Theme.dividerHeight
    
    function getColoredIconSource() {
        if (!root.iconSource) return ""
        // Use pre-colored SVG files based on iconSource name
        if (root.iconSource === "activity") {
            return "qrc:/qml/icons/activity-green.svg"
        } else if (root.iconSource === "droplets") {
            return "qrc:/qml/icons/droplets-blue.svg"
        } else if (root.iconSource === "wind") {
            return "qrc:/qml/icons/wind-yellow.svg"
        }
        // Fallback to original
        return "qrc:/qml/icons/" + root.iconSource + ".svg"
    }
    
    // Icon in top-right corner (opacity-10) - colored background icon
    Image {
        id: bgIcon
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Theme.spacingLg
        source: getColoredIconSource()
        width: Theme.iconSize
        height: Theme.iconSize
        sourceSize.width: Theme.iconSize
        sourceSize.height: Theme.iconSize
        fillMode: Image.PreserveAspectFit
        visible: root.iconSource !== ""
        opacity: 0.1
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingLg
        spacing: Theme.spacingSm
        
        // Label row with icon
        Row {
            spacing: Theme.spacingSm
            width: parent.width
            // Colored SVG icon - use pre-colored SVG files
            Image {
                id: iconImg
                source: root.getColoredIconSource()
                width: Theme.iconSize
                height: Theme.iconSize
                sourceSize.width: Theme.iconSize
                sourceSize.height: Theme.iconSize
                fillMode: Image.PreserveAspectFit
                visible: root.iconSource !== ""
            }
            Text {
                text: root.label.toUpperCase()
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeXs
                font.bold: true
                font.letterSpacing: 1
            }
        }
        
        // Value row
        Row {
            spacing: Theme.spacingSm
            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: Theme.fontSize4xl
                font.bold: true
                font.family: Theme.fontFamilyMono
            }
            Text {
                text: root.unit
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSm
                font.bold: false
                anchors.baseline: parent.children[0].baseline
                anchors.baselineOffset: 0
            }
        }
    }
}

