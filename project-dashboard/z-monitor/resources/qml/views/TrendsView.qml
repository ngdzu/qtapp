/**
 * @file TrendsView.qml
 * @brief Historical trends view with area charts
 *
 * Mirrors the sample React TrendsView layout and style.
 * Panels: HEART RATE, SPO2, RESP with area charts.
 * Header: FILTER chip, clock/date, 1H/6H/24H selector.
 * Footer: Navigation buttons.
 *
 * Data binding: uses `trendsController` context property.
 * For each panel, we set `selectedMetric` and call `loadTrendData()` to populate.
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.3
import "qrc:/qml/components"
import QtQuick.Window 2.15

Item {
    id: root
    anchors.fill: parent
    property color panelBorder: "#27272a" // zinc-800
    property color panelBg: "#18181b"    // zinc-900 (medical-panel)
    property color gridColor: "#333333"

    // Time range selector state
    property int hours: 24

    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        anchors.margins: 16

        // Patient banner (simple replica)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: panelBg
            border.color: panelBorder
            border.width: 1
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 16

                // Avatar placeholder
                Rectangle {
                    width: 32
                    height: 32
                    radius: 16
                    color: "#27272a"
                    border.color: "#3f3f46"
                    border.width: 1
                }

                Column {
                    spacing: 2
                    Text {
                        text: "DOE, JOHN"
                        color: "#fafafa" // zinc-50
                        font.bold: true
                        font.pixelSize: 13
                    }
                    Text {
                        text: "BED-04"
                        color: "#10b981" // emerald-500
                        font.pixelSize: 11
                        font.family: "monospace"
                    }
                }
            }
        }

        // Header with filter chip, clock, and range selector
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Filter chip (bg-emerald-700/30 text-emerald-300)
            Rectangle {
                width: 110
                height: 24
                color: "#064e3b" // emerald-700/30
                radius: 6
                border.color: "#064e3b"

                Text {
                    anchors.centerIn: parent
                    text: "FILTER: MON"
                    color: "#6ee7b7" // emerald-300
                    font.bold: true
                    font.pixelSize: 10
                    font.letterSpacing: 1.2
                }
            }

            // Clock/Date
            Text {
                text: Qt.formatTime(new Date(), "hh:mm:ss") + " | " + Qt.formatDate(new Date(), "dd MMM yyyy").toUpperCase()
                color: "#9ca3af" // zinc-400
                font.pixelSize: 11
                font.family: "monospace"
            }

            Item {
                Layout.fillWidth: true
            }

            // Range buttons (1H/6H/24H)
            RowLayout {
                spacing: 8

                Repeater {
                    model: [1, 6, 24]
                    delegate: Rectangle {
                        width: 44
                        height: 28
                        radius: 4
                        border.color: "#3f3f46" // zinc-700
                        border.width: 1
                        color: (modelData === root.hours) ? "#3f3f46" : "#09090b" // zinc-950

                        Text {
                            anchors.centerIn: parent
                            text: modelData + "H"
                            color: (modelData === root.hours) ? "#ffffff" : "#a1a1aa" // white : zinc-400
                            font.pixelSize: 11
                            font.bold: modelData === root.hours
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.hours = modelData;
                                // Adjust controller start/end
                                var end = new Date();
                                var start = new Date(end.getTime() - modelData * 60 * 60 * 1000);
                                trendsController.setStartTime(start);
                                trendsController.setEndTime(end);
                            }
                        }
                    }
                }
            }
        }

        // Panels area - three stacked trend panels
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // HEART RATE panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                title: "HEART RATE"
                accentColor: "#10b981" // emerald-500
                strokeColor: "#10b981"
                vitalMetricName: "heart_rate"
            }

            // SPO2 panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                title: "SPO2"
                accentColor: "#3b82f6" // blue-500
                strokeColor: "#3b82f6"
                vitalMetricName: "spo2"
            }

            // RESP panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                title: "RESP"
                accentColor: "#eab308" // yellow-500
                strokeColor: "#eab308"
                vitalMetricName: "resp"
            }
        }

        // Footer navigation removed to avoid duplication with Main bottom bar
    }
}
