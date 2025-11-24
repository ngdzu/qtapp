import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import qml 1.0

Dialog {
    id: root
    
    signal exitConfirmed()
    
    modal: true
    z: 999
    width: Theme.dialogWidth
    height: Theme.dialogHeight
    
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    
    background: Rectangle {
        anchors.fill: parent
        color: Theme.cardBackground
        radius: Theme.radiusXl
        border.color: Theme.border
        border.width: 1
    }

    contentItem: Rectangle {
        anchors.fill: parent
        color: "transparent"
        radius: Theme.radiusXl
        clip: true
        
        Column {
            width: parent.width
            spacing: 0

            // Header section
            Rectangle {
                id: dlgHeader
                width: parent.width
                height: Theme.dialogHeaderHeight
                color: Theme.colorWithOpacity(Theme.background, 0.2)
                
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.spacingXl
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Exit Simulator"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeXl
                    font.bold: true
                }
                
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: Theme.dividerHeight
                    color: Theme.border
                }
            }

            // Content section
            Item {
                width: parent.width
                height: parent.parent.height - dlgHeader.height
                
                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingXl
                    spacing: Theme.spacingXl
                    
                    // Message body
                    Text {
                        width: parent.width
                        text: "Are you sure you want to exit the simulator?"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeMd
                        wrapMode: Text.Wrap
                    }

                    // Buttons
                    RowLayout {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: Theme.spacingMd
                        
                        Button {
                            id: yesButton
                            text: "Yes"
                            Layout.preferredWidth: Theme.dialogButtonWidth
                            Layout.preferredHeight: Theme.buttonHeight
                            onClicked: {
                                yesButton.enabled = false
                                root.close()
                                root.exitConfirmed()
                            }
                            
                            background: Rectangle {
                                anchors.fill: parent
                                color: Theme.primary
                                border.color: Theme.primary
                                border.width: Theme.dividerHeight
                                radius: Theme.radiusMd
                            }
                            
                            contentItem: Text {
                                text: yesButton.text
                                color: Theme.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: Theme.fontSizeMd
                                font.bold: true
                            }
                        }
                        
                        Button {
                            id: noButton
                            text: "No"
                            Layout.preferredWidth: Theme.dialogButtonWidth
                            Layout.preferredHeight: Theme.buttonHeight
                            onClicked: root.close()
                            
                            background: Rectangle {
                                anchors.fill: parent
                                color: Theme.colorWithOpacity(Theme.textPrimary, 0.08)
                                border.color: Theme.border
                                border.width: Theme.dividerHeight
                                radius: Theme.radiusMd
                            }
                            
                            contentItem: Text {
                                text: noButton.text
                                color: Theme.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: Theme.fontSizeMd
                            }
                        }
                    }
                }
            }
        }
    }
}

