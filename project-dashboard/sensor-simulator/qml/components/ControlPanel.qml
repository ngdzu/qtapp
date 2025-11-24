import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import qml 1.0

Rectangle {
    id: root
    
    signal triggerCritical()
    signal triggerWarning()
    signal triggerNotification()
    signal playDemo()
    
    property bool isDemoRunning: false
    
    Layout.fillWidth: true
    Layout.preferredHeight: contentCol.implicitHeight + (Theme.spacingLg * 2)
    
    color: Theme.cardBackground
    radius: Theme.radiusXl
    border.color: Theme.border
    
    ColumnLayout {
        id: contentCol
        anchors.fill: parent
        anchors.margins: Theme.spacingLg
        spacing: Theme.spacingLg
        
        // Scenarios Section - matching React reference
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            
            Text {
                text: "SCENARIOS"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeXs
                font.bold: true
                font.letterSpacing: 2
                Layout.bottomMargin: Theme.spacingMd
            }
            
            Button {
                id: demoButton
                text: root.isDemoRunning ? "Running Demo..." : "Play Demo"
                enabled: !root.isDemoRunning
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.buttonHeightXl
                leftPadding: Theme.spacingLg
                rightPadding: Theme.spacingXl
                topPadding: Theme.spacingMd
                bottomPadding: Theme.spacingMd
                onClicked: root.playDemo()
                
                background: Rectangle {
                    color: demoButton.enabled ? Theme.accentIndigo : Theme.cardBackgroundSecondary
                    radius: Theme.radiusLg
                    border.width: 0
                }
                
                contentItem: Item {
                    anchors.fill: parent
                    anchors.leftMargin: demoButton.leftPadding
                    anchors.rightMargin: demoButton.rightPadding
                    anchors.topMargin: demoButton.topPadding
                    anchors.bottomMargin: demoButton.bottomPadding

                    Row {
                        spacing: Theme.spacingSm
                        anchors.centerIn: parent
                        Image {
                            id: playCircleIcon
                            source: "qrc:/qml/icons/circle-play.svg"
                            width: 18
                            height: 18
                            sourceSize.width: 18
                            sourceSize.height: 18
                        }
                        Text {
                            id: demoText
                            text: demoButton.text
                            color: demoButton.enabled ? "white" : Theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: Theme.fontSizeSm
                            font.bold: true
                        }
                    }
                }
            }
            
            Text {
                text: "Sequence: Critical → Notify → Warning"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSize10px
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacingSm
            }
        }
        
        // Manual Events Section - matching React reference
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            
            Text {
                text: "MANUAL EVENTS"
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeXs
                font.bold: true
                font.letterSpacing: 2
                Layout.bottomMargin: Theme.spacingMd
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSm
                
                // Critical Button
                EventButton {
                    label: "Trigger Critical"
                    subtitle: "Code Blue Alarm"
                    accentColor: Theme.accentRedLight
                    onClicked: root.triggerCritical()
                }
                
                // Warning Button
                EventButton {
                    label: "Trigger Warning"
                    subtitle: "Sensor Check"
                    accentColor: Theme.accentOrange
                    onClicked: root.triggerWarning()
                }
                
                // Notification Button
                EventButton {
                    label: "Notify"
                    subtitle: "General Alert"
                    accentColor: Theme.accentSky
                    onClicked: root.triggerNotification()
                }
            }
        }
        
        // System Info Section
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingMd
            height: Theme.dividerHeight
            color: Theme.border
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSm
            
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "DEVICE STATUS"
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeXs
                    font.family: Theme.fontFamilyMono
                }
                Item { Layout.fillWidth: true }
                Row {
                    spacing: 6
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: Theme.accentEmerald
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "ONLINE"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeXs
                        font.family: Theme.fontFamilyMono
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.systemInfoHeight
                color: Theme.background
                radius: Theme.radiusMd
                border.color: Theme.border
                border.width: Theme.dividerHeight
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMd
                    spacing: Theme.spacingXs
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Proto:"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "v2.1 (JSON)"
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Rate:"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "5 Hz"
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Mode:"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "Interactive"
                            color: Theme.accentIndigo
                            font.pixelSize: Theme.fontSize10px
                            font.family: Theme.fontFamilyMono
                            font.bold: false
                        }
                    }
                }
            }
        }
    }
}

