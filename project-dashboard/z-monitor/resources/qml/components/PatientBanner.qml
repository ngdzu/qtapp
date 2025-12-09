/**
 * @file PatientBanner.qml
 * @brief Patient information banner component
 *
 * Displays patient name and bed location in header.
 * Optimized to prevent unnecessary re-renders.
 *
 * @author Z Monitor Team
 * @date 2025-12-09
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import qml 1.0
import "../theme"

// Theme colors are passed in from parent; avoid hard dependency here

Rectangle {
    id: banner
    // Layout handled by parent
    // Colors provided by parent for flexibility
    property color bannerColor: "#18181b"
    property color bannerBorderColor: "#27272a"
    width: TopBarConstants.bannerWidth
    height: TopBarConstants.bannerHeight
    color: banner.bannerColor
    border.color: banner.bannerBorderColor
    border.width: 1
    radius: 6

    // Public API
    property string name: ""
    property string bedLabel: ""
    property color colorText: "#e5e7eb"
    property color accentColor: "#34d399"

    // Clickable area for future ADT actions
    signal bannerClicked

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        spacing: 10

        // Avatar (User icon with purple background)
        Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            color: "#7c3aed" // Purple-600
            radius: 6

            Image {
                anchors.centerIn: parent
                width: 24
                height: 24
                source: "qrc:/qml/icons/user.svg"
                fillMode: Image.PreserveAspectFit
                cache: true
                asynchronous: false
            }
        }

        // Patient Info
        Column {
            Layout.fillWidth: true
            spacing: 2

            Text {
                id: nameText
                text: banner.name || "NO PATIENT"
                font.pixelSize: 14
                font.bold: true
                font.capitalization: Font.AllUppercase
                color: banner.colorText
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                id: bedText
                text: banner.bedLabel || "STANDBY"
                font.pixelSize: 11
                font.family: "Monospace"
                font.bold: true
                color: banner.bedLabel ? banner.accentColor : "#71717a"
                elide: Text.ElideRight
                width: parent.width
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: banner.bannerClicked()
    }
}
