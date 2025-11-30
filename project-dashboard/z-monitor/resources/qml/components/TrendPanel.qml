/**
 * @file TrendPanel.qml
 * @brief Reusable trend chart panel component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts 2.3

Rectangle {
    id: panel
    property string title
    property color accentColor: "#10b981"
    property color strokeColor: "#10b981"
    property string vitalMetricName

    // Local styling defaults (match TrendsView)
    property color panelBorder: "#27272a"
    property color panelBg: "#18181b"
    property color gridColor: "#333333"

    color: panelBg
    border.color: panelBorder
    border.width: 1
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: panel.title
            color: panel.accentColor
            font.bold: true
            font.pixelSize: 14
            font.letterSpacing: 1.5
        }

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
                backgroundRoundness: 0

                ValueAxis {
                    id: xAxis
                    gridVisible: true
                    gridLineColor: gridColor
                    labelsColor: "#71717a"
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
                    color: Qt.rgba((panel.strokeColor.r), (panel.strokeColor.g), (panel.strokeColor.b), 0.2)
                    axisX: xAxis
                    axisY: yAxis
                }

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
