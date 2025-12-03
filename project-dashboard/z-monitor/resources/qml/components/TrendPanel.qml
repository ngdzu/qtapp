/**
 * @file TrendPanel.qml
 * @brief Reusable trend chart panel component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

Rectangle {
    id: panel
    
    property bool debugInit: { console.info("TrendPanel ROOT CREATED for title:", title); return true; }
    
    property string title
    property color accentColor: "#10b981"
    property color strokeColor: "#10b981"
    property string vitalMetricName
    
    Component.onCompleted: {
        console.info("TrendPanel created for:", title, "metric:", vitalMetricName);
    }

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
            Layout.fillHeight: true
            
            Component.onCompleted: {
                console.info("TrendPanel Item container created for:", panel.title);
            }

            ChartView {
                id: chart
                anchors.fill: parent
                legend.visible: false
                antialiasing: true
                backgroundRoundness: 0
                backgroundColor: panelBg
                plotAreaColor: panelBg
                
                Component.onCompleted: {
                    console.info("ChartView created for:", panel.title);
                    // Delay refresh to ensure context is ready
                    Qt.callLater(refresh);
                }

                ValueAxis {
                    id: xAxis
                    gridVisible: true
                    gridLineColor: gridColor
                    labelsColor: "#71717a"
                    labelsFont.pixelSize: 10
                    tickCount: 6
                    titleText: ""
                    
                    Component.onCompleted: {
                        console.info("xAxis created for:", panel.title);
                    }
                }

                ValueAxis {
                    id: yAxis
                    gridVisible: true
                    gridLineColor: gridColor
                    labelsColor: "#71717a"
                    labelsFont.pixelSize: 10
                    tickCount: 5
                    titleText: ""
                    
                    Component.onCompleted: {
                        console.info("yAxis created for:", panel.title);
                        console.info("About to create AreaSeries for:", panel.title, "strokeColor:", panel.strokeColor);
                    }
                }

                AreaSeries {
                    id: area
                    name: panel.title
                    axisX: xAxis
                    axisY: yAxis
                    
                    property color baseColor: Qt.color(panel.strokeColor)
                    
                    // Convert string color to Qt color, then make semi-transparent for area fill
                    color: Qt.rgba(baseColor.r, baseColor.g, baseColor.b, 0.2)
                    borderColor: panel.strokeColor
                    borderWidth: 2
                    
                    Component.onCompleted: {
                        console.info("AreaSeries created for:", panel.title);
                    }

                    upperSeries: LineSeries {
                        id: line
                        Component.onCompleted: {
                            console.info("LineSeries created for:", panel.title);
                        }
                    }
                }

                function refresh() {
                    console.info("refresh() called for:", panel.title);
                    // Check if trendsController exists in the context
                    if (typeof trendsController === 'undefined' || !trendsController) {
                        console.warn("TrendPanel: trendsController not available for:", panel.title);
                        // Show mock data for demo
                        refreshWithMockData();
                        return;
                    }
                    console.info("trendsController exists for:", panel.title);
                    
                    try {
                        // Set the selected metric in the controller
                        trendsController.setSelectedMetric(panel.vitalMetricName);
                        trendsController.loadTrendData();
                        
                        var pts = trendsController.trendData;
                        line.clear();
                        
                        if (pts.length === 0) {
                            console.log("TrendPanel: No data from controller, using mock data");
                            refreshWithMockData();
                            return;
                        }
                    
                    // Apply exponential smoothing filter for smooth curves
                    var alpha = 0.3; // Smoothing factor
                    var smoothed = [];
                    
                    for (var i = 0; i < pts.length; ++i) {
                        var p = pts[i];
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
                    
                    // Find min/max for axis scaling
                    var minX = 1e18;
                    var maxX = 0;
                    var minY = 1e9;
                    var maxY = -1e9;
                    
                    for (var i = 0; i < smoothed.length; ++i) {
                        var p = smoothed[i];
                        var x = p.timestamp;
                        var y = p.value;
                        
                        line.append(x, y);
                        
                        if (x < minX) minX = x;
                        if (x > maxX) maxX = x;
                        if (y < minY) minY = y;
                        if (y > maxY) maxY = y;
                    }
                    
                    // Set axis ranges with padding
                    xAxis.min = minX;
                    xAxis.max = maxX;
                    
                    var range = Math.max(1, (maxY - minY));
                    var pad = range * 0.1; // 10% padding
                    yAxis.min = minY - pad;
                    yAxis.max = maxY + pad;
                    } catch (error) {
                        console.error("TrendPanel refresh error:", error);
                        refreshWithMockData();
                    }
                }

                function refreshWithMockData() {
                    console.info("refreshWithMockData() called for:", panel.title);
                    line.clear();
                    console.info("LineSeries cleared for:", panel.title);
                    
                    // Generate mock sinusoidal data for demo
                    var now = Date.now();
                    var hoursBack = 24;
                    var points = 50;
                    var interval = (hoursBack * 3600 * 1000) / points;
                    
                    var baseValue = 70;
                    var amplitude = 10;
                    if (panel.vitalMetricName === "spo2") {
                        baseValue = 96;
                        amplitude = 2;
                    } else if (panel.vitalMetricName === "resp") {
                        baseValue = 16;
                        amplitude = 3;
                    }
                    
                    for (var i = 0; i < points; i++) {
                        var x = now - (hoursBack * 3600 * 1000) + (i * interval);
                        var y = baseValue + amplitude * Math.sin(i / 5) + (Math.random() - 0.5) * 2;
                        line.append(x, y);
                    }
                    
                    xAxis.min = now - (hoursBack * 3600 * 1000);
                    xAxis.max = now;
                    yAxis.min = baseValue - amplitude - 5;
                    yAxis.max = baseValue + amplitude + 5;
                }
                
                Connections {
                    target: typeof trendsController !== 'undefined' ? trendsController : null
                    function onTrendDataChanged() {
                        chart.refresh();
                    }
                }
            }
        }
    }
}
