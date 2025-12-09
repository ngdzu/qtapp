/**
 * @file BottomNav.qml
 * @brief Bottom navigation bar component for Z Monitor
 *
 * Contains main navigation buttons and code blue emergency button
 *
 * @author Z Monitor Team
 * @date 2025-12-09
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: bottomNav

    // Properties for view state management
    property int currentView: 0
    property var viewState: ({
            Monitor: 0,
            Trends: 1,
            Analysis: 2,
            Settings: 3
        })

    // Theme color properties
    property color colorPanel: "#18181b"
    property color colorBorder: "#27272a"
    property color colorECG: "#10b981"
    property color colorSPO2: "#3b82f6"
    property color colorCritical: "#ef4444"

    // Signals
    signal viewChanged(int newView)
    signal codeBlueClicked
    signal logoutClicked

    color: colorPanel
    border.color: colorBorder
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // Left Navigation Buttons
        Row {
            spacing: 4

            NavButton {
                text: "MONITOR"
                iconBase: "layout-dashboard"
                iconColorSuffix: "emerald"
                active: bottomNav.currentView === bottomNav.viewState.Monitor
                activeColor: bottomNav.colorECG
                onClicked: bottomNav.viewChanged(bottomNav.viewState.Monitor)
            }

            NavButton {
                text: "AI ANALYSIS"
                iconBase: "brain-circuit"
                iconColorSuffix: "purple"
                active: bottomNav.currentView === bottomNav.viewState.Analysis
                activeColor: "#a855f7" // Purple-500
                onClicked: bottomNav.viewChanged(bottomNav.viewState.Analysis)
            }

            NavButton {
                text: "TRENDS"
                iconBase: "activity"
                iconColorSuffix: "blue"
                active: bottomNav.currentView === bottomNav.viewState.Trends
                activeColor: bottomNav.colorSPO2
                onClicked: bottomNav.viewChanged(bottomNav.viewState.Trends)
            }

            Rectangle {
                width: 1
                height: 48
                color: bottomNav.colorBorder
                anchors.verticalCenter: parent.verticalCenter
            }

            // Code Blue Button
            Rectangle {
                width: 120
                height: 48
                color: "#450a0a" // Red-950
                border.color: "#7f1d1d" // Red-900
                border.width: 1
                radius: 4

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/qml/icons/siren.svg"
                        width: 20
                        height: 20
                        sourceSize.width: 20
                        sourceSize.height: 20
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "CODE BLUE"
                        font.pixelSize: 9
                        font.bold: true
                        font.letterSpacing: 1
                        color: bottomNav.colorCritical
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: bottomNav.codeBlueClicked()
                }
            }
        }

        // Spacer
        Item {
            Layout.fillWidth: true
        }

        // Right Navigation Buttons
        Row {
            spacing: 4

            NavButton {
                text: "MENU"
                iconBase: "settings"
                iconColorSuffix: "white"
                active: bottomNav.currentView === bottomNav.viewState.Settings
                onClicked: bottomNav.viewChanged(bottomNav.viewState.Settings)
            }

            NavButton {
                text: "LOGOUT"
                iconBase: "log-out"
                iconColorSuffix: "gray"
                active: false
                onClicked: bottomNav.logoutClicked()
            }
        }
    }
}
