import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    property string label: ""
    property int value: 0
    property string unit: ""
    property color accentColor: Theme.accentEmerald
    property string icon: ""
    
    color: Theme.cardBackground
    radius: Theme.radiusXl
    border.color: Theme.border
    
    Column {
        anchors.fill: parent
        anchors.margins: Theme.cardPadding
        spacing: Theme.spacingSm
        
        Row {
            spacing: Theme.spacingSm
            width: parent.width
            Text {
                text: root.label
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSm
                font.bold: true
            }
            Item { width: parent.width - parent.children[0].width - parent.spacing }
        }
        
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
                font.pixelSize: Theme.fontSizeMd
                verticalAlignment: Text.AlignBottom
                anchors.baseline: parent.children[0].baseline
                anchors.baselineOffset: -8
            }
        }
    }
}

