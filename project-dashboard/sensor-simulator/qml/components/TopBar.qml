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
    
    color: "transparent"
    border.color: Theme.border
    border.width: 0
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: Theme.spacingLg
        anchors.leftMargin: Theme.spacingLg
        anchors.rightMargin: Theme.spacingLg
        anchors.topMargin: Theme.spacingLg
        anchors.bottomMargin: Theme.spacingLg
        
        // Logo & Title - matching React reference
        Row {
            spacing: Theme.spacingMd
            Rectangle {
                width: Theme.topBarIconSize
                height: Theme.topBarIconSize
                radius: Theme.radiusXl
                // Gradient effect: from-emerald-500 to-teal-600
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Theme.accentEmerald }
                    GradientStop { position: 1.0; color: "#14b8a6" } // teal-500
                }
                // Using simple Image - SVG should be white/light colored
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/qml/icons/activity.svg"
                    width: Theme.iconSize
                    height: Theme.iconSize
                    sourceSize.width: Theme.iconSize
                    sourceSize.height: Theme.iconSize
                }
            }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2
                Text {
                    text: "Telemetry Simulator"
                    color: "white"
                    font.pixelSize: Theme.fontSizeXl
                    font.bold: true
                    font.family: Theme.fontFamily
                }
                Text {
                    text: "SIM-ID: 9000-X • PORT: 9002 • "
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeXs
                    font.family: Theme.fontFamilyMono
                    Text {
                        anchors.left: parent.right
                        anchors.baseline: parent.baseline
                        text: "CONNECTED"
                        color: Theme.accentEmerald
                        font.pixelSize: Theme.fontSizeXs
                        font.family: Theme.fontFamilyMono
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Last Event Display - matching React reference
        Rectangle {
            Layout.fillWidth: true
            Layout.maximumWidth: 512
            Layout.preferredHeight: Theme.topBarLastEventHeight
            color: Theme.cardBackground
            radius: Theme.radiusLg
            border.color: Theme.border
            border.width: Theme.dividerHeight
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMd
                spacing: Theme.spacingLg
                Text {
                    text: "Last Event"
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeXs
                    font.bold: true
                    font.letterSpacing: 1
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: root.lastEventText
                    color: root.lastEventColor
                    font.pixelSize: Theme.fontSizeSm
                    font.family: Theme.fontFamilyMono
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
            }
        }
    }
}

