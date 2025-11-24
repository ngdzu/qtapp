import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import qml 1.0

Button {
    id: root
    
    property string label: ""
    property string subtitle: ""
    property color accentColor: Theme.accentEmerald
    
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.buttonHeight
    
    background: Rectangle {
        color: Theme.colorWithOpacity(root.accentColor, 0.07)
        border.color: Theme.colorWithOpacity(root.accentColor, 0.3)
        border.width: Theme.dividerHeight
        radius: Theme.radiusLg
    }
    
    contentItem: Row {
        spacing: Theme.spacingSm
        anchors.centerIn: parent
        
        Rectangle {
            width: Theme.eventIconSize
            height: Theme.eventIconSize
            radius: Theme.radiusMd
            color: root.accentColor
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Column {
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: root.label
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeMd
                font.bold: true
            }
            Text {
                text: root.subtitle
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSm
            }
        }
    }
}

