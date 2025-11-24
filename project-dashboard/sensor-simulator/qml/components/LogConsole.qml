import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import qml 1.0

Rectangle {
    id: root
    
    property alias logModel: logView.model
    property string filterLevel: "All"
    
    signal clearRequested()
    
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    color: Theme.cardBackground
    radius: Theme.radiusXl
    border.color: Theme.border
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.logHeaderHeight
            color: Theme.colorWithOpacity(Theme.background, 0.2)
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMd
                spacing: Theme.spacingMd
                
                Text {
                    text: "Telemetry Stream"
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: Theme.fontSizeBase
                }
                
                Rectangle {
                    width: Theme.eventBadgeWidth
                    height: Theme.eventBadgeHeight
                    radius: Theme.eventBadgeRadius
                    color: Theme.cardBackgroundSecondary
                    Text {
                        anchors.centerIn: parent
                        text: logView.count + " events"
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSizeXs
                        font.family: Theme.fontFamilyMono
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: "Filter:"
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeBase
                }
                
                ComboBox {
                    id: filterCombo
                    Layout.preferredWidth: Theme.comboBoxWidth
                    Layout.preferredHeight: Theme.comboBoxHeight
                    model: ["All", "Critical", "Warning", "Info", "Debug"]
                    onCurrentTextChanged: root.filterLevel = currentText
                    
                    background: Rectangle {
                        color: Theme.cardBackgroundSecondary
                        border.color: Theme.border
                        radius: Theme.radiusSm
                    }
                    
                    contentItem: Text {
                        text: filterCombo.displayText
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeBase
                        font.family: Theme.fontFamilyMono
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: Theme.spacingSm
                        rightPadding: filterCombo.indicator.width + filterCombo.spacing
                    }
                    
                    indicator: Canvas {
                        id: indicatorCanvas
                        x: filterCombo.width - width - Theme.spacingSm
                        y: filterCombo.topPadding + (filterCombo.availableHeight - height) / 2
                        width: Theme.comboBoxIndicatorWidth
                        height: Theme.comboBoxIndicatorHeight
                        contextType: "2d"
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()
                            ctx.strokeStyle = Theme.textPrimary
                            ctx.fillStyle = Theme.textPrimary
                            ctx.lineWidth = 1.5
                            ctx.beginPath()
                            ctx.moveTo(0, 0)
                            ctx.lineTo(width / 2, height)
                            ctx.lineTo(width, 0)
                            ctx.closePath()
                            ctx.fill()
                        }
                    }
                    
                    popup: Popup {
                        y: filterCombo.height
                        width: filterCombo.width
                        implicitHeight: contentItem.implicitHeight
                        padding: Theme.spacingXs
                        
                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: filterCombo.popup.visible ? filterCombo.delegateModel : null
                            currentIndex: filterCombo.highlightedIndex
                            
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }
                        
                        background: Rectangle {
                            color: Theme.cardBackground
                            border.color: Theme.border
                            radius: Theme.radiusSm
                        }
                    }
                    
                    delegate: ItemDelegate {
                        width: filterCombo.width
                        text: modelData
                        
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardBackgroundSecondary : "transparent"
                            radius: Theme.radiusXs
                        }
                        
                        contentItem: Text {
                            text: modelData
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSizeBase
                            font.family: Theme.fontFamilyMono
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: Theme.spacingSm
                        }
                    }
                }
                
                ReusableButton {
                    text: "Clear"
                    Layout.preferredHeight: Theme.buttonHeightSm
                    onClicked: root.clearRequested()
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
                    isVisible: root.filterLevel === "All" || root.filterLevel === level
                }
            }
        }
    }
}

