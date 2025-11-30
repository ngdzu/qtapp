/**
 * @file WaveformPanel.qml
 * @brief Waveform display panel with label and grid
 *
 * Displays real-time waveform data (ECG, pleth, resp)
 * Uses Canvas for 60 FPS rendering
 * Converted from Node.js WaveformChart.tsx - pixel-perfect match
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string label: "WAVEFORM"
    property color labelColor: "#10b981"
    property color waveformColor: "#10b981"
    property bool showFilter: false
    property var waveformData: []  // Array of {time, value} from controller

    // Grid background
    Canvas {
        id: gridCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            // Draw grid lines matching React border-t border-zinc-800/30 grid
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

        // Rendering properties
        property real scrollSpeed: 2.0  // Pixels per sample
        property real amplitude: 0.6    // Vertical scale (0-1)

        // Repaint when data changes
        Connections {
            target: root
            function onWaveformDataChanged() {
                waveformCanvas.requestPaint();
            }
        }

        // 60 FPS update timer
        Timer {
            interval: 16 // ~60 FPS
            running: true
            repeat: true
            onTriggered: {
                waveformCanvas.requestPaint();
            }
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            // Check if we have data
            if (!root.waveformData || root.waveformData.length === 0) {
                return;
            }

            // Match React Line stroke={color} strokeWidth={2}
            ctx.strokeStyle = root.waveformColor;
            ctx.lineWidth = 2;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";

            var centerY = height / 2;
            var scaleY = (height / 2) * waveformCanvas.amplitude;

            ctx.beginPath();

            // Draw waveform from right to left (scrolling effect)
            var sampleCount = Math.min(root.waveformData.length, Math.floor(width / waveformCanvas.scrollSpeed));

            for (var i = 0; i < sampleCount; i++) {
                // X position: start from right edge, move left
                var x = width - (i * waveformCanvas.scrollSpeed);

                // Get sample (most recent samples are at the end of array)
                var sampleIndex = root.waveformData.length - 1 - i;
                var sample = root.waveformData[sampleIndex];

                // Y position: scale sample value to canvas height
                // Assuming value is already normalized (-1 to 1) or (0 to 1)
                var sampleValue = sample.value || 0;
                var y = centerY - (sampleValue * scaleY);

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
