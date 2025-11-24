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
    
    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingSm
        spacing: Theme.spacingXs
        
        // Header row with type and timestamp
        RowLayout {
            width: parent.width
            spacing: Theme.spacingSm
            
            Text {
                text: root.logLevel.toUpperCase()
                color: getLevelTextColor(root.logLevel)
                font.pixelSize: Theme.fontSize10px
                font.bold: true
                font.letterSpacing: 1
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: root.logTime
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                font.family: Theme.fontFamilyMono
                opacity: 0.7
            }
        }
        
        // Message
        Text {
            width: parent.width
            text: root.logMessage
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeXs
            font.family: Theme.fontFamilyMono
            wrapMode: Text.Wrap
            opacity: 0.9
        }
    }
    
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
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

