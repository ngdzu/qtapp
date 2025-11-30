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

        // Footer navigation bar (matching React footer)
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: 90
                height: 32
                radius: 4
                color: "#09090b"
                border.color: panelBorder
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "MONITOR"
                    color: "#a1a1aa"
                    font.pixelSize: 10
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Rectangle {
                width: 100
                height: 32
                radius: 4
                color: "#09090b"
                border.color: panelBorder
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "AI ANALYSIS"
                    color: "#a1a1aa"
                    font.pixelSize: 10
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Rectangle {
                width: 90
                height: 32
                radius: 4
                color: "#047857"
                border.color: "#047857"
                border.width: 1 // emerald-700 active
                Text {
                    anchors.centerIn: parent
                    text: "TRENDS"
                    color: "#ffffff"
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Rectangle {
                width: 100
                height: 32
                radius: 4
                color: "#b91c1c"
                border.color: "#7f1d1d"
                border.width: 1 // red-700/900
                Text {
                    anchors.centerIn: parent
                    text: "CODE BLUE"
                    color: "#ffffff"
                    font.pixelSize: 10
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                width: 70
                height: 32
                radius: 4
                color: "#09090b"
                border.color: panelBorder
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "MENU"
                    color: "#a1a1aa"
                    font.pixelSize: 10
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Rectangle {
                width: 80
                height: 32
                radius: 4
                color: "#09090b"
                border.color: panelBorder
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "LOGOUT"
                    color: "#a1a1aa"
                    font.pixelSize: 10
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }
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
            border.width: 1
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                // Title (matching React h3 text-emerald-500 font-bold text-sm tracking-wider mb-2)
                Text {
                    text: panel.title
                    color: panel.accentColor
                    font.bold: true
                    font.pixelSize: 14 // text-sm
                    font.letterSpacing: 1.5 // tracking-wider
                }

                // Chart area (flex-1 w-full h-full)
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ChartView {
                        id: chart
                        anchors.fill: parent
                        theme: ChartView.ChartThemeDark
                        backgroundColor: panelBg
                        legend.visible: false
                        antialiasing: true

                        // Grid styling to match React CartesianGrid strokeDasharray="3 3" stroke="#333"
                        backgroundRoundness: 0

                        ValueAxis {
                            id: xAxis
                            gridVisible: true
                            gridLineColor: gridColor
                            labelsColor: "#71717a" // zinc-500
                            labelsFont.pixelSize: 10
                            tickCount: 6
                        }

                        ValueAxis {
                            id: yAxis
                            gridVisible: true
                            gridLineColor: gridColor
                            labelsColor: "#71717a"
                            labelsFont.pixelSize: 10
                            tickCount: 5
                        }

                        AreaSeries {
                            id: area
                            name: panel.title
                            upperSeries: LineSeries {
                                id: line
                                color: panel.strokeColor
                                width: 2
                            }
                            lowerSeries: LineSeries {
                                color: panel.strokeColor
                                width: 0
                            }
                            // Gradient fill matching React linearGradient stop offset="5%" stopOpacity={0.3}
                            color: Qt.rgba((panel.strokeColor.r), (panel.strokeColor.g), (panel.strokeColor.b), 0.2)
                            axisX: xAxis
                            axisY: yAxis
                        }

                        // Populate data from controller
                        function refresh() {
                            trendsController.setSelectedMetric(panel.vitalMetricName);
                            trendsController.loadTrendData();
                            var pts = trendsController.trendData;
                            line.clear();
                            var minX = 1e18;
                            var maxX = 0;
                            var minY = 1e9;
                            var maxY = -1e9;
                            for (var i = 0; i < pts.length; ++i) {
                                var p = pts[i];
                                var x = p.timestamp;
                                var y = p.value;
                                line.append(x, y);
                                if (x < minX)
                                    minX = x;
                                if (x > maxX)
                                    maxX = x;
                                if (y < minY)
                                    minY = y;
                                if (y > maxY)
                                    maxY = y;
                            }
                            xAxis.min = minX;
                            xAxis.max = maxX;
                            yAxis.min = Math.floor(minY - 2);
                            yAxis.max = Math.ceil(maxY + 2);
                        }

                        Component.onCompleted: refresh()
                        Connections {
                            target: trendsController
                            function onTrendDataChanged() {
                                chart.refresh();
                            }
                        }
                    }
                }
            }
        }
    }

    // Expose component type
    property alias TrendPanel: trendPanelComponent
}
