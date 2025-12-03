/**
 * @file WaveformDisplay.qml
 * @brief Real-time waveform display with zoom, pan, and freeze functionality
 *
 * Renders ECG and plethysmograph waveforms at 60 FPS using Qt Quick Canvas.
 * Implements zoom (1x-4x), pan (horizontal scroll), and freeze (pause updates).
 * Ring buffer prevents memory leaks with 10-second display window.
 *
 * Touch/Mouse Gestures:
 * - Pinch (two fingers): Zoom in/out (1x-4x)
 * - Drag (one finger): Pan left/right when frozen
 * - Tap: Freeze waveform (enables panning)
 * - Double-tap: Reset zoom and pan to defaults
 * - Mouse wheel: Zoom in/out (desktop)
 *
 * Performance targets:
 * - 60 FPS rendering (< 16ms per frame)
 * - < 100ms sample-to-screen latency
 * - Smooth scrolling with no discontinuities
 *
 * @see DOC-COMP-028 Waveform Display Implementation
 * @author Z Monitor Team
 * @date 2025-12-02
 */

import QtQuick
import QtQuick.Controls

Item {
    id: root

    /// Waveform data from controller (array of {time, value})
    property var waveformData: []

    /// Waveform label (e.g., "ECG", "Pleth")
    property string label: "WAVEFORM"

    /// Waveform color
    property color waveformColor: "#10b981"

    /// Grid color (background lines)
    property color gridColor: "#27272a40"

    /// Zoom level (1x, 2x, 3x, 4x)
    property real zoomLevel: 1.0

    /// Pan offset in pixels (horizontal scroll)
    property real panOffset: 0.0

    /// Freeze mode (pause waveform updates)
    property bool frozen: false

    /// Gain adjustment (vertical amplitude scale)
    property real gain: 1.0

    /// Time scale in seconds (visible window duration)
    property real timeScale: 10.0

    /// Show signal quality indicator
    property bool showQuality: true

    /// Signal quality (0-1, where 1 is best)
    property real signalQuality: 1.0

    // Accessible properties for screen readers
    Accessible.role: Accessible.Chart
    Accessible.name: label + " waveform display"
    Accessible.description: "Real-time " + label + " waveform, " + 
                            (frozen ? "paused" : "updating") + 
                            ", zoom " + zoomLevel.toFixed(1) + "x"

    // Grid background
    Canvas {
        id: gridCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            ctx.strokeStyle = root.gridColor;
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

            // Vertical lines (adjust for zoom)
            var cols = Math.floor(10 * root.zoomLevel);
            for (var j = 0; j <= cols; j++) {
                var x = (width / cols) * j;
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, height);
                ctx.stroke();
            }
        }

        // Repaint grid when zoom changes
        Connections {
            target: root
            function onZoomLevelChanged() {
                gridCanvas.requestPaint();
            }
        }
    }

    // Waveform canvas
    Canvas {
        id: waveformCanvas
        anchors.fill: parent

        // Ring buffer for efficient memory usage
        property var ringBuffer: []
        property int maxBufferSize: Math.ceil(root.timeScale * 250) // 10s * 250 Hz

        // Previous smoothed values for exponential smoothing
        property var smoothedValues: ({})

        // Smoothing factor (0.85 = 85% previous, 15% new)
        property real smoothingFactor: 0.85

        // Scroll speed in pixels per sample
        property real scrollSpeed: 0.8

        // Connections to update canvas when data changes
        Connections {
            target: root
            function onWaveformDataChanged() {
                if (!root.frozen) {
                    waveformCanvas.updateBuffer();
                    waveformCanvas.requestPaint();
                }
            }
        }

        // 60 FPS update timer
        Timer {
            id: renderTimer
            interval: 16 // ~60 FPS
            running: !root.frozen
            repeat: true
            onTriggered: {
                waveformCanvas.requestPaint();
            }
        }

        /**
         * @brief Update ring buffer with new data
         */
        function updateBuffer() {
            if (!root.waveformData || root.waveformData.length === 0) {
                return;
            }

            // Add new samples to ring buffer
            for (var i = 0; i < root.waveformData.length; i++) {
                ringBuffer.push(root.waveformData[i]);
            }

            // Trim buffer to max size
            while (ringBuffer.length > maxBufferSize) {
                ringBuffer.shift();
            }
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            // Check if we have data
            if (!ringBuffer || ringBuffer.length === 0) {
                return;
            }

            ctx.strokeStyle = root.waveformColor;
            ctx.lineWidth = 2;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";

            // Center waveform vertically
            var centerY = height / 2;
            var scaleY = (height / 2) * 0.8 * root.gain;

            // Calculate visible samples based on zoom
            var visibleWidth = width / root.zoomLevel;
            var sampleCount = Math.min(ringBuffer.length, Math.floor(visibleWidth / scrollSpeed));

            // Apply pan offset (clamp to valid range)
            var maxPan = Math.max(0, (ringBuffer.length - sampleCount) * scrollSpeed);
            var actualPanOffset = Math.max(0, Math.min(root.panOffset, maxPan));

            // Calculate sample offset from pan
            var sampleOffset = Math.floor(actualPanOffset / scrollSpeed);

            // Find min/max for normalization
            var minVal = 0, maxVal = 0;
            var startIdx = Math.max(0, ringBuffer.length - sampleCount - sampleOffset);
            var endIdx = Math.min(ringBuffer.length, startIdx + sampleCount);

            for (var k = startIdx; k < endIdx; k++) {
                var val = ringBuffer[k].value || 0;
                if (k === startIdx) {
                    minVal = val;
                    maxVal = val;
                } else {
                    minVal = Math.min(minVal, val);
                    maxVal = Math.max(maxVal, val);
                }
            }

            var range = Math.max(maxVal - minVal, 0.1);
            var offset = (minVal + maxVal) / 2;

            // Build smoothed points
            var smoothedPoints = [];
            for (var i = 0; i < sampleCount; i++) {
                var x = width - (i * scrollSpeed * root.zoomLevel) + actualPanOffset;
                
                var sampleIndex = endIdx - 1 - i;
                if (sampleIndex < 0 || sampleIndex >= ringBuffer.length) {
                    continue;
                }

                var sample = ringBuffer[sampleIndex];
                var rawValue = sample.value || 0;

                // Exponential smoothing
                var key = sampleIndex.toString();
                var smoothedValue;
                
                if (smoothedValues[key] !== undefined) {
                    var prevSmoothed = smoothedValues[key];
                    smoothedValue = (smoothingFactor * prevSmoothed) + ((1 - smoothingFactor) * rawValue);
                } else {
                    smoothedValue = rawValue;
                }
                
                smoothedValues[key] = smoothedValue;

                // Normalize and scale
                var normalizedValue = (smoothedValue - offset) / range;
                var y = centerY - (normalizedValue * scaleY);

                // Clamp Y
                var margin = height * 0.1;
                y = Math.max(margin, Math.min(height - margin, y));

                smoothedPoints.push({x: x, y: y});
            }

            // Clean up old smoothed values
            var keysToKeep = [];
            for (var j = startIdx; j < endIdx; j++) {
                keysToKeep.push(j.toString());
            }
            
            for (var key in smoothedValues) {
                if (keysToKeep.indexOf(key) === -1) {
                    delete smoothedValues[key];
                }
            }

            // Draw smooth curve using quadratic bezier
            ctx.beginPath();
            for (var m = 0; m < smoothedPoints.length; m++) {
                var point = smoothedPoints[m];
                
                if (m === 0) {
                    ctx.moveTo(point.x, point.y);
                } else {
                    var prevPoint = smoothedPoints[m - 1];
                    var controlX = (prevPoint.x + point.x) / 2;
                    var controlY = (prevPoint.y + point.y) / 2;
                    ctx.quadraticCurveTo(controlX, controlY, point.x, point.y);
                }
            }

            ctx.stroke();
        }
    }

    // Signal quality indicator
    Rectangle {
        id: qualityIndicator
        visible: root.showQuality
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        width: 60
        height: 20
        color: "#18181b"
        border.color: getQualityColor()
        border.width: 1
        radius: 4

        function getQualityText() {
            if (root.signalQuality > 0.8) return "GOOD";
            if (root.signalQuality > 0.5) return "FAIR";
            return "POOR";
        }

        function getQualityColor() {
            if (root.signalQuality > 0.8) return "#10b981";
            if (root.signalQuality > 0.5) return "#f59e0b";
            return "#ef4444";
        }

        Text {
            anchors.centerIn: parent
            text: parent.getQualityText()
            color: parent.getQualityColor()
            font.pixelSize: 11
            font.family: "monospace"
            font.bold: true
        }

        Accessible.role: Accessible.Indicator
        Accessible.name: "Signal quality: " + getQualityText()
    }

    // Freeze indicator
    Rectangle {
        visible: root.frozen
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 8
        width: 60
        height: 20
        color: "#18181b"
        border.color: "#f59e0b"
        border.width: 1
        radius: 4

        Text {
            anchors.centerIn: parent
            text: "FROZEN"
            color: "#f59e0b"
            font.pixelSize: 11
            font.family: "monospace"
            font.bold: true
        }

        Accessible.role: Accessible.Indicator
        Accessible.name: "Waveform frozen (paused)"
    }

    // Label overlay
    Text {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 8
        text: root.label
        color: root.waveformColor
        font.pixelSize: 14
        font.family: "monospace"
        font.bold: true
        opacity: 0.8

        Accessible.ignored: true
    }

    // Touch gestures: pinch-to-zoom and pan
    PinchArea {
        id: pinchArea
        anchors.fill: parent
        
        property real initialZoom: 1.0
        
        onPinchStarted: {
            initialZoom = root.zoomLevel;
        }
        
        onPinchUpdated: function(pinch) {
            // Pinch to zoom (two-finger gesture)
            var newZoom = initialZoom * pinch.scale;
            root.zoomLevel = Math.max(1.0, Math.min(4.0, newZoom));
        }
        
        // Mouse area for wheel zoom and single-finger pan
        MouseArea {
            anchors.fill: parent
            
            property real dragStartX: 0
            property real panStart: 0
            property bool isDragging: false
            property real pressTime: 0

            onPressed: function(mouse) {
                dragStartX = mouse.x;
                panStart = root.panOffset;
                isDragging = false;
                pressTime = Date.now();
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    var delta = dragStartX - mouse.x;
                    
                    // Pan if moved more than 5 pixels (works when frozen or running)
                    if (Math.abs(delta) > 5) {
                        isDragging = true;
                        root.panOffset = Math.max(0, panStart + delta);
                    }
                }
            }
            
            onReleased: function(mouse) {
                var pressDuration = Date.now() - pressTime;
                
                // If not dragging and quick tap (< 200ms), toggle freeze
                if (!isDragging && pressDuration < 200) {
                    root.frozen = !root.frozen;
                }
            }

            onWheel: function(wheel) {
                // Zoom with mouse wheel
                if (wheel.angleDelta.y > 0) {
                    root.zoomLevel = Math.min(4.0, root.zoomLevel + 0.25);
                } else {
                    root.zoomLevel = Math.max(1.0, root.zoomLevel - 0.25);
                }
            }
            
            onDoubleClicked: {
                // Double-tap/click to reset view
                root.resetView();
            }
        }
    }

    /**
     * @brief Reset zoom and pan to defaults
     */
    function resetView() {
        root.zoomLevel = 1.0;
        root.panOffset = 0.0;
    }

    /**
     * @brief Toggle freeze mode
     */
    function toggleFreeze() {
        root.frozen = !root.frozen;
    }

    /**
     * @brief Clear waveform buffer
     */
    function clearBuffer() {
        waveformCanvas.ringBuffer = [];
        waveformCanvas.smoothedValues = ({});
        waveformCanvas.requestPaint();
    }
}
