import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    property string logTime: ""
    property string logLevel: "Info"
    property string logMessage: ""
    property bool isVisible: true
    
    width: parent ? parent.width : 0
    height: Theme.logEntryHeight
    color: "transparent"
    visible: root.isVisible
    
    RowLayout {
        anchors.fill: parent
        spacing: Theme.spacingMd
        
        // Time
        Text {
            text: root.logTime
            color: Theme.textMuted
            font.pixelSize: Theme.fontSizeSm
            font.family: Theme.fontFamilyMono
            Layout.preferredWidth: Theme.logTimeWidth
        }
        
        // Level Badge
        Rectangle {
            Layout.preferredWidth: Theme.logBadgeWidth
            Layout.preferredHeight: Theme.logBadgeHeight
            radius: Theme.radiusXs
            color: getLevelBgColor(root.logLevel)
            border.color: getLevelBorderColor(root.logLevel)
            border.width: 1
            
            Text {
                anchors.centerIn: parent
                text: root.logLevel
                color: getLevelTextColor(root.logLevel)
                font.pixelSize: Theme.fontSizeXs
                font.bold: true
            }
        }
        
        // Message
        Text {
            text: root.logMessage
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeBase
            font.family: Theme.fontFamilyMono
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
    
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Theme.logIndicatorWidth
        color: root.logLevel ? getLevelTextColor(root.logLevel) : Theme.textMuted
    }
    
    function getLevelBgColor(level) {
        if (level === "Critical") return Theme.colorWithOpacity(Theme.error, 0.1)
        if (level === "Warning") return Theme.colorWithOpacity(Theme.warning, 0.1)
        if (level === "Info") return Theme.colorWithOpacity(Theme.success, 0.1)
        return Theme.colorWithOpacity(Theme.textMuted, 0.1)
    }
    
    function getLevelBorderColor(level) {
        if (level === "Critical") return Theme.colorWithOpacity(Theme.error, 0.3)
        if (level === "Warning") return Theme.colorWithOpacity(Theme.warning, 0.3)
        if (level === "Info") return Theme.colorWithOpacity(Theme.success, 0.3)
        return Theme.colorWithOpacity(Theme.textMuted, 0.3)
    }
    
    function getLevelTextColor(level) {
        if (level === "Critical") return Theme.error
        if (level === "Warning") return Theme.warning
        if (level === "Info") return Theme.success
        return Theme.textMuted
    }
}

