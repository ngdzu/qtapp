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
        // Matching React: bg-red-950/30 border border-red-900/50
        color: getBackgroundColor()
        border.color: getBorderColor()
        border.width: Theme.dividerHeight
        radius: Theme.radiusLg
    }
    
    function getBackgroundColor() {
        if (root.accentColor === Theme.accentRedLight) return Theme.colorWithOpacity(Theme.accentRed, 0.12) // red-950/30
        if (root.accentColor === Theme.accentOrange) return Theme.colorWithOpacity(Theme.accentOrange, 0.12) // orange-950/30
        return Theme.colorWithOpacity(Theme.accentSky, 0.12) // sky-950/30
    }
    
    function getBorderColor() {
        if (root.accentColor === Theme.accentRedLight) return Theme.colorWithOpacity(Theme.accentRed, 0.2) // red-900/50
        if (root.accentColor === Theme.accentOrange) return Theme.colorWithOpacity(Theme.accentOrange, 0.2) // orange-900/50
        return Theme.colorWithOpacity(Theme.accentSky, 0.2) // sky-900/50
    }
    
    contentItem: Row {
        spacing: Theme.spacingMd
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingLg
        anchors.verticalCenter: parent.verticalCenter
        
        // Icon in rounded circle - matching React: p-2 bg-red-500/10 rounded-full
        Rectangle {
            width: 36
            height: 36
            radius: 18
            color: Theme.colorWithOpacity(root.accentColor, 0.1)
            anchors.verticalCenter: parent.verticalCenter
            
            Image {
                id: eventIcon
                anchors.centerIn: parent
                source: getIconSource()
                width: 18
                height: 18
                sourceSize.width: 18
                sourceSize.height: 18
                fillMode: Image.PreserveAspectFit
                visible: getIconSource() !== "" && status === Image.Ready
            }
            
            Text {
                anchors.centerIn: parent
                text: getIconText()
                color: root.accentColor
                font.pixelSize: Theme.iconSizeSm
                visible: getIconSource() === ""
            }
        }
        
        Column {
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: root.label
                color: root.accentColor
                font.pixelSize: Theme.fontSizeSm
                font.bold: true
            }
            Text {
                text: root.subtitle
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                opacity: 0.6
            }
        }
    }
    
    function getIconSource() {
        if (root.accentColor === Theme.accentRedLight) return "qrc:/qml/icons/triangle-alert.svg"
        if (root.accentColor === Theme.accentOrange) return "qrc:/qml/icons/shield-alert.svg"
        return "qrc:/qml/icons/bell.svg"
    }
    
    function getIconText() {
        // Fallback emoji if SVG not available
        if (root.accentColor === Theme.accentRedLight) return "⚠"
        if (root.accentColor === Theme.accentOrange) return ""
        return ""
    }
}


