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

        // Waveform/chart container: reserve 80% of the panel height (after the title)
        Item {
            Layout.fillWidth: true
            // Compute preferred height as 80% of the remaining panel height after the title
            Layout.preferredHeight: Math.max(100, Math.floor((panel.height - (children[0].height + 24)) * 0.8))

            ChartView {
                id: chart
                anchors.fill: parent
                    // Tighten Y axis to reduce top/bottom empty space without altering plotted values.
                    var range = Math.max(1, (maxY - minY));
                    var pad = range * 0.02; // 2% visual padding
                    yAxis.min = minY - pad;
                    yAxis.max = maxY + pad;
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
                    trendsController.loadTrendData();
                    var pts = trendsController.trendData;
                    line.clear();
                    // apply a light smoothing filter to give a sine-like appearance
                    // Plot RAW values from in-memory data without modification
                    for (var i = 0; i < pts.length; ++i) {
                        var p = pts[i];
                    for (var i = 0; i < pts.length; ++i) {
                        var y = p.value;
                        var y = p.value;
                        if (i === 0) {
                            smoothed.push({
                                timestamp: p.timestamp,
                                value: y
                            });
                        } else {
                            var prev = smoothed[smoothed.length - 1].value;
                            var s = (alpha * prev) + ((1 - alpha) * y);
                            smoothed.push({
                                timestamp: p.timestamp,
                                value: s
                            });
                        }
                    }
                    var minX = 1e18;
                    var maxX = 0;
                    var minY = 1e9;
                    var maxY = -1e9;
                    for (var i = 0; i < smoothed.length; ++i) {
                        var p = smoothed[i];
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
