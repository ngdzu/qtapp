/**
 * @file AnalysisView.qml
 * @brief AI Analysis view placeholder
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/qml/components"

Rectangle {
    id: root
    color: "#09090b"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "AI Analysis"
            font.pixelSize: 22
            font.bold: true
            color: "#e9d5ff" // purple-200
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#18181b"
            border.color: "#27272a"
            border.width: 1
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Analysis View - Coming Soon"
                font.pixelSize: 18
                color: "#71717a"
            }
        }
    }
}
