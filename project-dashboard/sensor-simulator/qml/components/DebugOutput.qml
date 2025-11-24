import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    color: Theme.colorWithOpacity(Theme.cardBackground, 0.5)
    radius: Theme.radiusXl
    border.color: Theme.border
    border.width: Theme.dividerHeight
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingLg
        spacing: Theme.spacingSm
        
        Text {
            text: "Debug Output"
            color: Theme.textMutedDark
            font.pixelSize: Theme.fontSize10px
            font.bold: true
            font.letterSpacing: 2
            Layout.bottomMargin: Theme.spacingSm
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: Theme.dividerHeight
            color: Theme.border
            Layout.bottomMargin: Theme.spacingSm
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingXs
            
            Text {
                text: "Initialized graphics context... OK"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                font.family: Theme.fontFamilyMono
            }
            
            Text {
                text: "Loaded waveform profiles... OK"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                font.family: Theme.fontFamilyMono
            }
            
            Text {
                text: "Audio engine... STARTED"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                font.family: Theme.fontFamilyMono
            }
            
            Text {
                text: "Transport layer... LISTENING (5Hz)"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                font.family: Theme.fontFamilyMono
            }
        }
        
        Item { Layout.fillHeight: true }
    }
}

