/**
 * @file WaveformPanel.qml
 * @brief Waveform display panel with label and grid
 *
 * Displays real-time waveform data (ECG, pleth, resp)
 * Uses Canvas for 60 FPS rendering
 * Converted from Node.js WaveformChart.tsx
 */

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#18181b" // Zinc-900 (medical-panel)
    border.color: "#27272a" // Zinc-800
    border.width: 1
    radius: 4

    property string label: "WAVEFORM"
    property color labelColor: "#10b981"
    property color waveformColor: "#10b981"
    property bool showFilter: false

    Column {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // Header Row
        Row {
            width: parent.width
            height: 20

            Text {
                text: root.label
                font.pixelSize: 12
                font.bold: true
                font.letterSpacing: 1.5
                color: root.labelColor
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: 1
                height: 1
            }

            Text {
                visible: root.showFilter
                text: "FILTER: MON"
                font.pixelSize: 9
                color: "#52525b" // Zinc-600
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
            }
        }

        // Waveform Display Area
        Item {
            width: parent.width
            height: parent.height - 24

            // Grid background
            Canvas {
                id: gridCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);

                    // Draw grid lines
                    ctx.strokeStyle = "#27272a40"; // Zinc-800 with alpha
                    ctx.lineWidth = 1;

                    // Horizontal lines
                    var rows = 4;
                    for (var i = 0; i <= rows; i++) {
                        var y = (height / rows) * i;
                        ctx.beginPath();
                        ctx.moveTo(0, y);
                        ctx.lineTo(width, y);
                        ctx.stroke();
                    }

                    // Vertical lines
                    var cols = 10;
                    for (var j = 0; j <= cols; j++) {
                        var x = (width / cols) * j;
                        ctx.beginPath();
                        ctx.moveTo(x, 0);
                        ctx.lineTo(x, height);
                        ctx.stroke();
                    }
                }
            }

            // Waveform Canvas (overlaid on grid)
            Canvas {
                id: waveformCanvas
                anchors.fill: parent

                // Placeholder waveform simulation
                property real phase: 0

                Timer {
                    interval: 16 // ~60 FPS
                    running: true
                    repeat: true
                    onTriggered: {
                        waveformCanvas.phase += 0.1;
                        waveformCanvas.requestPaint();
                    }
                }

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);

                    ctx.strokeStyle = root.waveformColor;
                    ctx.lineWidth = 2;
                    ctx.beginPath();

                    // Draw simulated waveform
                    var points = 100;
                    for (var i = 0; i < points; i++) {
                        var x = (width / points) * i;
                        var y = height / 2 + Math.sin((i / 10) + phase) * (height / 4);

                        if (i === 0) {
                            ctx.moveTo(x, y);
                        } else {
                            ctx.lineTo(x, y);
                        }
                    }

                    ctx.stroke();
                }
            }
        }
    }
}
