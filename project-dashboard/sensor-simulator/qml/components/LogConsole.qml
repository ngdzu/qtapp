import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import qml 1.0

Rectangle {
    id: root
    
    property alias logModel: logView.model
    property string filterLevel: "All"
    property bool isPaused: false
    
    signal clearRequested()
    signal togglePause()
    
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    color: Theme.cardBackground
    radius: Theme.radiusXl
    border.color: Theme.border
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header - matching React reference
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.logHeaderHeight
            color: Theme.cardBackground
            border.width: 0
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingLg
                spacing: Theme.spacingSm
                
                // Terminal icon + Title
                Row {
                    spacing: Theme.spacingSm
                    Image {
                        id: terminalIcon
                        source: "qrc:/qml/icons/terminal.svg"
                        width: 18
                        height: 18
                        sourceSize.width: 18
                        sourceSize.height: 18
                        fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "Telemetry Stream"
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Rectangle {
                        width: 60
                        height: 20
                        radius: 10
                        color: Theme.cardBackgroundSecondary
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            anchors.centerIn: parent
                            text: logView.count + " events"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeXs
                            font.family: Theme.fontFamilyMono
                        }
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Pause/Play button
                Button {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    onClicked: root.togglePause()
                    
                    background: Rectangle {
                        color: root.isPaused ? Theme.colorWithOpacity(Theme.accentAmber, 0.1) : "transparent"
                        radius: Theme.radiusLg
                        border.width: 0
                    }
                    
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: root.isPaused ? "qrc:/qml/icons/play.svg" : "qrc:/qml/icons/pause.svg"
                        width: 16
                        height: 16
                        sourceSize.width: 16
                        sourceSize.height: 16
                        fillMode: Image.PreserveAspectFit
                    }
                }
                
                // Clear button (trash icon)
                Button {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    onClicked: root.clearRequested()
                    
                    background: Rectangle {
                        color: "transparent"
                        radius: Theme.radiusLg
                        border.width: 0
                    }
                    
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/qml/icons/trash-2.svg"
                        width: 16
                        height: 16
                        sourceSize.width: 16
                        sourceSize.height: 16
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.dividerHeight
                color: Theme.border
            }
        }
        
        // Filter Bar - matching React reference
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: Theme.colorWithOpacity(Theme.cardBackground, 0.5)
            border.width: 0
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingSm
                spacing: Theme.spacingSm
                
                Image {
                    id: filterIcon
                    source: "qrc:/qml/icons/funnel-muted.svg"
                    width: 14
                    height: 14
                    sourceSize.width: 14
                    sourceSize.height: 14
                    Layout.leftMargin: Theme.spacingSm
                }
                
                Repeater {
                    model: ["ALL", "CRITICAL", "WARNING", "INTERNAL", "INFO"]
                    Button {
                        text: modelData
                        Layout.preferredHeight: 24
                        Layout.preferredWidth: implicitWidth + Theme.spacingMd
                        
                        background: Rectangle {
                            color: root.filterLevel.toUpperCase() === modelData ? Theme.cardBackgroundSecondary : "transparent"
                            radius: 9999
                            border.width: 0
                        }
                        
                        contentItem: Text {
                            text: modelData
                            color: root.filterLevel.toUpperCase() === modelData ? "white" : Theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: Theme.fontSizeXs
                            font.bold: root.filterLevel.toUpperCase() === modelData
                        }
                        
                        onClicked: {
                            var level = ""
                            if (modelData === "ALL") level = "All"
                            else if (modelData === "CRITICAL") level = "Critical"
                            else if (modelData === "WARNING") level = "Warning"
                            else if (modelData === "INTERNAL") level = "Internal"
                            else if (modelData === "INFO") level = "Info"
                            if (level !== "") root.filterLevel = level
                        }
                    }
                }
            }
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.dividerHeight
                color: Theme.border
            }
        }

        // Log List
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                
                background: Rectangle {
                    color: Theme.cardBackgroundSecondary
                    radius: Theme.radiusSm
                }
                
                contentItem: Rectangle {
                    implicitWidth: Theme.scrollBarSize
                    implicitHeight: Theme.scrollBarSize
                    radius: Theme.radiusSm
                    color: parent.pressed ? Theme.colorWithOpacity(Theme.textMuted, 0.8) : 
                           parent.hovered ? Theme.colorWithOpacity(Theme.textMuted, 0.6) : 
                           Theme.colorWithOpacity(Theme.textMuted, 0.4)
                }
            }
            
            // Console Output - matching React reference
            Item {
                anchors.fill: parent
                
                ListView {
                    id: logView
                    anchors.fill: parent
                    anchors.margins: Theme.spacingSm
                    model: ListModel {}
                    spacing: Theme.spacingXs
                    clip: true
                    
                    delegate: LogEntry {
                        width: logView.width
                        logTime: time
                        logLevel: level
                        logMessage: message
                        isVisible: root.filterLevel === "All" || 
                                  (root.filterLevel === "Critical" && level === "Critical") ||
                                  (root.filterLevel === "Warning" && level === "Warning") ||
                                  (root.filterLevel === "Internal" && level === "Internal") ||
                                  (root.filterLevel === "Info" && (level === "Info" || level === "Debug"))
                    }
                }
                
                // Empty state - matching React reference
                Column {
                    anchors.centerIn: parent
                    visible: logView.count === 0
                    spacing: Theme.spacingSm
                    
                    Image {
                        id: emptyTerminalIcon
                        source: "qrc:/qml/icons/terminal.svg"
                        width: 32
                        height: 32
                        sourceSize.width: 32
                        sourceSize.height: 32
                        opacity: 0.2
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    
                    Text {
                        text: "Waiting for data stream..."
                        color: Theme.textMutedDark
                        font.pixelSize: Theme.fontSizeBase
                        font.family: Theme.fontFamilyMono
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }
}

