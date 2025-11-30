import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.3

/*
  TrendsView.qml
  Mirrors the sample React TrendsView layout and style.
  Panels: HEART RATE, SPO2, RESP with area charts.
  Header: FILTER chip, clock/date, 1H/6H/24H selector.
  Footer: Navigation buttons (Monitor, AI Analysis, Trends, Code Blue, Menu, Logout).

  Data binding: uses `trendsController` context property.
  For each panel, we set `selectedMetric` and call `loadTrendData()` to populate.
*/

Item {
    id: root
    anchors.fill: parent
    property color panelBorder: "#2A2A2A"
    property color panelBg: "#101010"
    property color gridColor: "#333333"

    // Time range selector state
    property int hours: 24

    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        padding: 16

        // Patient banner (simple replica)
        Rectangle {
            Layout.fillWidth: true
            height: 56
            color: panelBg
            border.color: panelBorder
            radius: 8
            Row {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 16
                Image { source: "qrc:/assets/patient_icon.png"; width: 32; height: 32; fillMode: Image.PreserveAspectFit }
                Column {
                    spacing: 2
                    Text { text: "DOE, JOHN"; color: "#E5E7EB"; font.bold: true; font.pointSize: 14 }
                    Text { text: "BED-04"; color: "#10B981"; font.pointSize: 12 }
                }
            }
        }

        // Header with filter chip, clock, and range selector
        Row {
            Layout.fillWidth: true
            spacing: 12
            Rectangle {
                color: "#064E3B"; radius: 6; border.color: "#064E3B"
                Text { anchors.centerIn: parent; text: "FILTER: MON"; color: "#34D399"; font.bold: true; font.pointSize: 10 }
                width: 110; height: 24
            }
            Text {
                text: Qt.formatTime(new Date(), "hh:mm:ss") + " | " + Qt.formatDate(new Date(), "dd MMM yyyy")
                color: "#9CA3AF"; font.pointSize: 10
            }
            Item { Layout.fillWidth: true }
            // Range buttons
            function setHours(h) { root.hours = h; }
            Repeater {
                model: [1, 6, 24]
                delegate: Rectangle {
                    width: 40; height: 24; radius: 6
                    border.color: "#3F3F46"
                    color: (modelData === root.hours) ? "#3F3F46" : "#0A0A0A"
                    Text { anchors.centerIn: parent; text: modelData + "H"; color: (modelData === root.hours) ? "#FFFFFF" : "#A1A1AA"; font.pointSize: 10 }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.hours = modelData
                            // Optionally adjust controller start/end
                            var end = new Date();
                            var start = new Date(end.getTime() - modelData * 60 * 60 * 1000)
                            trendsController.setStartTime(start)
                            trendsController.setEndTime(end)
                        }
                    }
                }
            }
        }

        // Panels area
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 1
            rowSpacing: 12

            // HEART RATE panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                title: "HEART RATE"
                accentColor: "#10B981" // emerald
                strokeColor: "#10B981"
                vitalMetricName: "heart_rate"
            }

            // SPO2 panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                title: "SPO2"
                accentColor: "#3B82F6" // blue
                strokeColor: "#3B82F6"
                vitalMetricName: "spo2"
            }

            // RESP panel
            TrendPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                title: "RESP"
                accentColor: "#F59E0B" // amber/yellow
                strokeColor: "#F59E0B"
                vitalMetricName: "resp"
            }
        }

        // Footer navigation bar
        Row {
            Layout.fillWidth: true
            spacing: 8
            Rectangle { width: 90; height: 30; radius: 6; color: "#0A0A0A"; border.color: panelBorder; Text { anchors.centerIn: parent; text: "MONITOR"; color: "#D4D4D8"; font.pointSize: 10 } }
            Rectangle { width: 100; height: 30; radius: 6; color: "#0A0A0A"; border.color: panelBorder; Text { anchors.centerIn: parent; text: "AI ANALYSIS"; color: "#D4D4D8"; font.pointSize: 10 } }
            Rectangle { width: 90; height: 30; radius: 6; color: "#047857"; border.color: "#047857"; Text { anchors.centerIn: parent; text: "TRENDS"; color: "#FFFFFF"; font.pointSize: 10 } }
            Rectangle { width: 100; height: 30; radius: 6; color: "#B91C1C"; border.color: "#7F1D1D"; Text { anchors.centerIn: parent; text: "CODE BLUE"; color: "#FFFFFF"; font.pointSize: 10 } }
            Item { Layout.fillWidth: true }
            Rectangle { width: 70; height: 30; radius: 6; color: "#0A0A0A"; border.color: panelBorder; Text { anchors.centerIn: parent; text: "MENU"; color: "#D4D4D8"; font.pointSize: 10 } }
            Rectangle { width: 80; height: 30; radius: 6; color: "#0A0A0A"; border.color: panelBorder; Text { anchors.centerIn: parent; text: "LOGOUT"; color: "#D4D4D8"; font.pointSize: 10 } }
        }
    }

    // TrendPanel component definition
    Component {
        id: trendPanelComponent
        Rectangle {
            id: panel
            property string title
            property color accentColor
            property color strokeColor
            property string vitalMetricName
            color: panelBg
            border.color: panelBorder
            radius: 8
            anchors.leftMargin: 0

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                Text { text: panel.title; color: panel.accentColor; font.bold: true; font.pointSize: 12 }

                // Chart area
                ChartView {
                    id: chart
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: panel.height - 60
                    theme: ChartView.ChartThemeDark
                    backgroundColor: panelBg
                    legend.visible: false
                    antialiasing: true

                    ValueAxis { id: xAxis; gridVisible: true; gridLineColor: gridColor; labelsColor: "#A1A1AA" }
                    ValueAxis { id: yAxis; gridVisible: true; gridLineColor: gridColor; labelsColor: "#A1A1AA" }

                    AreaSeries {
                        id: area
                        name: panel.title
                        upperSeries: LineSeries { id: line; color: panel.strokeColor; width: 2 }
                        lowerSeries: LineSeries { color: panel.strokeColor; width: 0; opacity: 0.0 }
                        color: Qt.rgba(Qt.colorEqual(panel.strokeColor, panel.strokeColor) ? 0.2 : 0.2, 0.2, 0.2, 0.2)
                        axisX: xAxis
                        axisY: yAxis
                    }

                    // Populate data from controller
                    function refresh() {
                        trendsController.setSelectedMetric(panel.vitalMetricName)
                        trendsController.loadTrendData()
                        var pts = trendsController.trendData
                        line.clear()
                        var minX = 1e18
                        var maxX = 0
                        var minY = 1e9
                        var maxY = -1e9
                        for (var i=0; i<pts.length; ++i) {
                            var p = pts[i]
                            var x = p.timestamp
                            var y = p.value
                            line.append(x, y)
                            if (x < minX) minX = x
                            if (x > maxX) maxX = x
                            if (y < minY) minY = y
                            if (y > maxY) maxY = y
                        }
                        xAxis.min = minX
                        xAxis.max = maxX
                        yAxis.min = Math.floor(minY - 2)
                        yAxis.max = Math.ceil(maxY + 2)
                    }

                    Component.onCompleted: refresh()
                    Connections {
                        target: trendsController
                        function onTrendDataChanged() { chart.refresh() }
                    }
                }
            }
        }
    }

    // Expose as type for use above
    alias TrendPanel: trendPanelComponent
}
/**
 * @file TrendsView.qml
 * @brief Historical trends view (placeholder)
 */

import QtQuick
import QtQuick.Controls
import "qrc:/qml/components"

Rectangle {
    color: "#09090b"

    Text {
        anchors.centerIn: parent
        text: "Trends View - Coming Soon"
        font.pixelSize: 24
        color: "#71717a"
    }
}
