import QtQuick
import QtQuick.Layouts

Item {
    id: root
    
    property string title: "Title"
    property string value: "0%"
    property string iconSource: "" // We might use a font icon or image
    property color accentColor: "#10b981"
    property Component visualization: null
    
    Layout.fillWidth: true
    Layout.preferredHeight: 140
    
    // Background Glow (positioned behind card, same shape as card)
    Rectangle {
        id: glow
        anchors.fill: parent
        anchors.margins: -4
        color: root.accentColor
        opacity: 0.15
        radius: 10
        z: 0
    }
    
    // Main Card Background
    Rectangle {
        id: cardBackground
        anchors.fill: parent
        color: "#18181b" // Zinc-900
        border.color: "#27272a" // Zinc-800
        border.width: 1
        radius: 8
        z: 1
        
        // Hover effect
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: glow.opacity = 0.25
            onExited: glow.opacity = 0.15
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8
        z: 2
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: root.title
                color: "#a1a1aa" // Zinc-400
                font.pixelSize: 14
                font.weight: Font.Medium
                Layout.fillWidth: true
            }
            
            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: 16
                font.bold: true
            }
        }
        
        // Visualization Container
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            Loader {
                anchors.fill: parent
                sourceComponent: root.visualization
            }
        }
    }
}
