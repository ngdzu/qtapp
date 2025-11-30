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
        property real scrollSpeed: 0.8   // Pixels per sample (slowed down from 2.0)
        property real amplitude: 0.8    // Vertical scale (0-1) - 80% of available height
        property var smoothedData: []   // Smoothed data buffer for anti-aliasing
        property var previousSmoothedValues: ({})  // Previous smoothed values by sample index (for exponential smoothing)

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

            // Center waveform vertically in the view
            var centerY = height / 2;
            // Use 80% of available height for waveform amplitude
            var scaleY = (height / 2) * waveformCanvas.amplitude;

            // Calculate sample count for visible area (slower scroll = more samples visible)
            var sampleCount = Math.min(root.waveformData.length, Math.floor(width / waveformCanvas.scrollSpeed));

            // Find min/max for normalization (to center waveform properly)
            var minVal = 0;
            var maxVal = 0;
            for (var k = 0; k < sampleCount; k++) {
                var idx = root.waveformData.length - 1 - k;
                var val = root.waveformData[idx].value || 0;
                if (k === 0) {
                    minVal = val;
                    maxVal = val;
                } else {
                    minVal = Math.min(minVal, val);
                    maxVal = Math.max(maxVal, val);
                }
            }
            var range = Math.max(maxVal - minVal, 0.1); // Prevent division by zero
            var offset = (minVal + maxVal) / 2; // Center offset

            // Exponential smoothing factor (higher = smoother, but more lag)
            // 0.85 means 85% previous value, 15% new value - creates very smooth sine-wave-like curves
            var smoothingFactor = 0.85;
            
            // Build smoothed data array with exponential moving average
            var smoothedPoints = [];
            for (var i = 0; i < sampleCount; i++) {
                // X position: start from right edge, move left
                var x = width - (i * waveformCanvas.scrollSpeed);

                // Get sample (most recent samples are at the end of array)
                var sampleIndex = root.waveformData.length - 1 - i;
                var sample = root.waveformData[sampleIndex];
                var rawValue = sample.value || 0;

                // Apply exponential moving average for smooth sine-wave-like curves
                // Use sample index as key to track previous smoothed values
                var smoothedValue;
                var key = sampleIndex.toString();
                
                if (waveformCanvas.previousSmoothedValues[key] !== undefined) {
                    // Exponential smoothing: blend previous smoothed value with new raw value
                    var prevSmoothed = waveformCanvas.previousSmoothedValues[key];
                    smoothedValue = (smoothingFactor * prevSmoothed) + ((1 - smoothingFactor) * rawValue);
                } else {
                    // First time seeing this sample: use raw value
                    smoothedValue = rawValue;
                }
                
                // Store smoothed value for next frame
                waveformCanvas.previousSmoothedValues[key] = smoothedValue;

                // Normalize and center: subtract offset to center around 0, then scale
                var normalizedValue = (smoothedValue - offset) / range;
                var y = centerY - (normalizedValue * scaleY);

                // Clamp Y to prevent cutoff (with margin for 80% height)
                var margin = height * 0.1; // 10% margin on each side
                y = Math.max(margin, Math.min(height - margin, y));

                smoothedPoints.push({x: x, y: y, value: smoothedValue});
            }
            
            // Clean up old entries from previousSmoothedValues (keep only recent samples)
            var keysToKeep = [];
            for (var k = 0; k < sampleCount; k++) {
                var idx = root.waveformData.length - 1 - k;
                keysToKeep.push(idx.toString());
            }
            // Remove old keys (keep only current visible samples)
            for (var key in waveformCanvas.previousSmoothedValues) {
                if (keysToKeep.indexOf(key) === -1) {
                    delete waveformCanvas.previousSmoothedValues[key];
                }
            }

            // Draw smooth curve using quadratic bezier curves for sine-wave-like appearance
            ctx.beginPath();
            for (var j = 0; j < smoothedPoints.length; j++) {
                var point = smoothedPoints[j];
                
                if (j === 0) {
                    ctx.moveTo(point.x, point.y);
                } else {
                    // Use quadratic bezier curve for smooth transitions
                    var prevPoint = smoothedPoints[j - 1];
                    var controlX = (prevPoint.x + point.x) / 2;
                    var controlY = (prevPoint.y + point.y) / 2;
                    ctx.quadraticCurveTo(controlX, controlY, point.x, point.y);
                }
            }

            ctx.stroke();
        }
    }
}
