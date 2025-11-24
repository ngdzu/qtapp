import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    property string lastEventText: "-- No events --"
    property color lastEventColor: Theme.textMuted
    
    height: Theme.headerHeight
    anchors.left: parent.left
    anchors.right: parent.right
    
    color: Theme.colorWithOpacity(Theme.background, 0.25)
    border.color: Theme.border
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMd
        spacing: Theme.spacingMd
        
        // Logo & Title
        Row {
            spacing: Theme.spacingMd
            Rectangle {
                width: Theme.topBarIconSize
                height: Theme.topBarIconSize
                radius: Theme.radiusLg
                color: Theme.accentEmerald
                    Text {
                        anchors.centerIn: parent
                        text: "⚡"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.iconSizeSm
                    }
            }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    text: "Telemetry Simulator"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeXl
                    font.bold: true
                }
                Text {
                    text: "SIM-ID: 9000-X • PORT: 9002 • CONNECTED"
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeXs
                    font.family: Theme.fontFamilyMono
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Last Event Display
        Rectangle {
            Layout.preferredWidth: Theme.topBarLastEventWidth
            Layout.preferredHeight: Theme.topBarLastEventHeight
            color: Theme.cardBackground
            radius: Theme.radiusMd
            border.color: Theme.border
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMd
                Text {
                    text: "LAST EVENT"
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeXs
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: root.lastEventText
                    color: root.lastEventColor
                    font.pixelSize: Theme.fontSizeBase
                    font.family: Theme.fontFamilyMono
                    font.bold: true
                }
            }
        }
    }
}

