# Waveform Display Implementation Guide

**Document ID:** DESIGN-041  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a comprehensive guide for implementing waveform display in the Z Monitor, including requirements, sample code, implementation patterns, and best practices for smooth, real-time rendering.

## 1. Overview

Waveforms (ECG, plethysmogram) are critical diagnostic tools that require smooth, real-time rendering at high frame rates. This document explains how to implement waveform display using Qt Quick (QML) Canvas API, with sample code from the Z Monitor project.

**Key Requirements:**
- **Refresh Rate:** 60 FPS (frames per second) for smooth scrolling
- **Sample Rate:** 250 Hz (250 samples per second) for ECG waveforms
- **Smoothness:** No jumping, stuttering, or frame drops
- **Display Window:** 10 seconds of waveform visible at a time
- **Auto-scrolling:** Rightmost edge shows newest data

## 2. Related Documents

- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../requirements/03_FUNCTIONAL_REQUIREMENTS.md) - REQ-FUN-VITAL-011 (Waveform Display)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - REQ-NFR-PERF-101 (Display Refresh Rate)
- **Data Caching:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - WaveformCache architecture
- **Sensor Integration:** [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) - Waveform data source

## 3. Requirements

### 3.1. Performance Requirements

| Requirement | Target | Rationale |
|-------------|--------|-----------|
| **Frame Rate** | 60 FPS minimum | Smooth scrolling required for clinical interpretation |
| **Frame Drops** | < 1% under normal load | Occasional drops acceptable, but must be rare |
| **Sample Rate** | 250 Hz (ECG), 125 Hz (Pleth) | Clinical standard for waveform capture |
| **Display Window** | 10 seconds visible | Standard clinical viewing window |
| **Latency** | < 100ms from sample to screen | Real-time appearance critical |
| **Memory** | ~10 MB for 30-second buffer | Circular buffer for display only |

### 3.2. Visual Requirements

- **Smooth Scrolling:** Waveform scrolls continuously from right to left
- **No Jumping:** No sudden position changes or discontinuities
- **Grid Background:** Subtle grid pattern for amplitude/time reference
- **Color Coding:** ECG (green), Pleth (red), configurable
- **Baseline:** Horizontal baseline at center of display
- **Amplitude Scale:** Clear amplitude markers (e.g., 1 mV = 10 mm)

### 3.3. Functional Requirements

- **Auto-scroll:** Newest data always visible on right edge
- **Pause/Resume:** Ability to pause scrolling for detailed inspection
- **Gain Adjustment:** User-adjustable amplitude scaling
- **Time Scale:** Configurable time window (5s, 10s, 25s)
- **Signal Quality Indicator:** Visual indicator for signal quality (GOOD, FAIR, POOR)

## 4. Architecture Overview

### 4.1. Data Flow

[View Waveform Display Architecture Diagram (Mermaid)](./41_WAVEFORM_DISPLAY.mmd)  
[View Waveform Display Architecture Diagram (SVG)](./41_WAVEFORM_DISPLAY.svg)

**Textual Flow:**
```
Sensor Simulator (250 Hz)
    ↓ WebSocket JSON
WebSocketSensorDataSource
    ↓ Qt Signals (vitalSignsReceived)
MonitoringService
    ↓ Stores samples
WaveformCache (30-second circular buffer)
    ↓ Provides window of samples
WaveformController (QObject)
    ↓ Exposes data to QML
WaveformChart.qml (Canvas rendering)
    ↓ 60 FPS refresh
Display (smooth scrolling waveform)
```

### 4.2. Component Responsibilities

**WaveformCache (C++):**
- Circular buffer storing 30 seconds of waveform samples (250 Hz × 30s = 7,500 samples)
- Provides windowed access (e.g., last 2,500 samples for 10-second display)
- Thread-safe (accessed from real-time thread)

**WaveformController (C++ QObject):**
- Bridges C++ data to QML
- Exposes waveform samples as Q_PROPERTY
- Emits signals when new data available
- Runs on main/UI thread

**WaveformChart.qml (QML Canvas):**
- Renders waveform using Canvas 2D API
- Updates at 60 FPS using Timer or Animation
- Handles smooth scrolling and grid rendering

## 5. Sample Implementation

### 5.1. WaveformChart.qml Component

This is the main QML component for rendering waveforms. It uses Qt Quick Canvas API for high-performance 2D rendering.

```qml
import QtQuick 2.15
import QtQuick.Layouts 1.12

/**
 * @brief WaveformChart - Real-time waveform display component
 * 
 * Displays ECG or plethysmogram waveforms with smooth scrolling at 60 FPS.
 * Uses Canvas API for efficient rendering.
 * 
 * Properties:
 * - dataPoints: Array of waveform samples (updated from C++ controller)
 * - waveformColor: Color of waveform line (default: green for ECG)
 * - timeWindowSeconds: Number of seconds to display (default: 10)
 * - sampleRate: Samples per second (default: 250 for ECG)
 */
Rectangle {
    id: root
    
    // === PROPERTIES ===
    
    /**
     * @property var dataPoints
     * @brief Array of waveform sample values (numbers)
     * 
     * This property is bound to the C++ WaveformController's samples property.
     * The controller updates this array whenever new samples arrive from the sensor.
     * 
     * Example: [0.5, 1.2, 0.8, -0.3, 1.5, ...]
     * Values are typically in millivolts (ECG) or percentage (Pleth).
     */
    property var dataPoints: []
    
    /**
     * @property int maxPoints
     * @brief Maximum number of points to keep in buffer
     * 
     * For 10-second window at 250 Hz: 10 × 250 = 2,500 points
     * We keep slightly more (3,000) to allow smooth scrolling transitions.
     */
    property int maxPoints: 3000
    
    /**
     * @property color waveformColor
     * @brief Color of the waveform line
     * 
     * Default: Green for ECG (clinical standard)
     * Can be changed to red for plethysmogram or other waveforms.
     */
    property color waveformColor: "#00ff88"  // Green (ECG standard)
    
    /**
     * @property int timeWindowSeconds
     * @brief Number of seconds of waveform to display
     * 
     * Common values: 5s, 10s (default), 25s
     * 10 seconds is standard for ECG monitoring.
     */
    property int timeWindowSeconds: 10
    
    /**
     * @property int sampleRate
     * @brief Samples per second (Hz)
     * 
     * ECG: 250 Hz (standard clinical rate)
     * Plethysmogram: 125 Hz
     */
    property int sampleRate: 250
    
    /**
     * @property real gain
     * @brief Amplitude scaling factor
     * 
     * 1.0 = normal scale
     * 2.0 = double amplitude (zoom in)
     * 0.5 = half amplitude (zoom out)
     */
    property real gain: 1.0
    
    // === INTERNAL STATE ===
    
    /**
     * @brief Internal buffer for accumulating waveform samples
     * 
     * This buffer accumulates samples as they arrive. When it exceeds maxPoints,
     * older samples are removed (FIFO queue behavior).
     */
    property var waveformBuffer: []
    
    /**
     * @brief Number of samples to display in current window
     * 
     * Calculated from timeWindowSeconds × sampleRate
     * Example: 10 seconds × 250 Hz = 2,500 samples
     */
    readonly property int windowSize: timeWindowSeconds * sampleRate
    
    // === PUBLIC API ===
    
    /**
     * @brief Add new waveform samples to the buffer
     * 
     * Called by C++ controller whenever new samples arrive.
     * 
     * @param samples Array of numbers (waveform sample values)
     * 
     * Example:
     * waveformChart.addSamples([0.5, 1.2, 0.8, -0.3, 1.5])
     */
    function addSamples(samples) {
        // Append new samples to buffer
        for (var i = 0; i < samples.length; i++) {
            waveformBuffer.push(samples[i])
        }
        
        // Keep only last maxPoints (FIFO - remove oldest)
        if (waveformBuffer.length > maxPoints) {
            var removeCount = waveformBuffer.length - maxPoints
            waveformBuffer = waveformBuffer.slice(removeCount)
        }
        
        // Request repaint (triggers onPaint handler)
        canvas.requestPaint()
    }
    
    /**
     * @brief Clear the waveform buffer
     * 
     * Useful when switching patients or resetting display.
     */
    function clear() {
        waveformBuffer = []
        canvas.requestPaint()
    }
    
    // === VISUAL STYLING ===
    
    color: "#1a1a1a"  // Dark background (medical monitor standard)
    border.color: "#333333"
    border.width: 1
    radius: 4
    
    // === GRID BACKGROUND ===
    
    /**
     * Grid provides visual reference for amplitude and time.
     * Rendered on separate canvas layer (z: 0) behind waveform.
     */
    Canvas {
        id: gridCanvas
        anchors.fill: parent
        z: 0  // Behind waveform
        
        Component.onCompleted: {
            // Paint grid once on component creation
            requestPaint()
        }
        
        onPaint: {
            var ctx = getContext("2d")
            
            // Grid color: Subtle, low opacity
            ctx.strokeStyle = Qt.rgba(0.3, 0.3, 0.3, 0.3)  // 30% opacity gray
            ctx.lineWidth = 0.5
            
            // Vertical lines (time markers)
            // Every 20 pixels = approximately 1 second at typical display width
            var verticalSpacing = 20
            for (var x = 0; x < width; x += verticalSpacing) {
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)
                ctx.stroke()
            }
            
            // Horizontal lines (amplitude markers)
            // Center line (baseline) is thicker
            var horizontalSpacing = 20
            for (var y = 0; y < height; y += horizontalSpacing) {
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
                
                // Center line (baseline) is thicker
                if (Math.abs(y - height / 2) < 1) {
                    ctx.lineWidth = 1.0
                    ctx.strokeStyle = Qt.rgba(0.5, 0.5, 0.5, 0.5)
                } else {
                    ctx.lineWidth = 0.5
                    ctx.strokeStyle = Qt.rgba(0.3, 0.3, 0.3, 0.3)
                }
                ctx.stroke()
            }
        }
    }
    
    // === HEADER (Waveform Label) ===
    
    /**
     * Header shows waveform type and sample rate information.
     * Positioned at top of component.
     */
    RowLayout {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: 24
        
        Text {
            text: "ECG Lead II"  // Or "Plethysmogram" for SpO2
            color: waveformColor
            font.pixelSize: 12
            font.bold: true
            font.letterSpacing: 1
        }
        
        Item { Layout.fillWidth: true }  // Spacer
        
        Text {
            text: sampleRate + " Hz / " + timeWindowSeconds + " s"
            color: "#888888"
            font.pixelSize: 11
            font.family: "monospace"
        }
    }
    
    // === WAVEFORM CANVAS (Main Rendering) ===
    
    /**
     * Canvas component renders the actual waveform line.
     * This is where the 60 FPS rendering happens.
     */
    Canvas {
        id: canvas
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        z: 10  // Above grid
        
        Component.onCompleted: {
            // Initial paint (shows grid even before data arrives)
            requestPaint()
        }
        
        /**
         * @brief Paint handler - Called whenever requestPaint() is invoked
         * 
         * This function:
         * 1. Clears the canvas
         * 2. Extracts the window of samples to display (last N samples)
         * 3. Normalizes sample values to fit display height
         * 4. Draws smooth line connecting all points
         * 
         * Performance: This runs at 60 FPS, so it must be fast (< 16ms).
         */
        onPaint: {
            var ctx = getContext("2d")
            
            // Clear entire canvas (removes previous frame)
            ctx.clearRect(0, 0, width, height)
            
            // Need at least 2 points to draw a line
            if (waveformBuffer.length < 2) {
                return
            }
            
            // Extract window of samples to display
            // Show last N samples where N = windowSize (e.g., 2,500 for 10 seconds)
            var startIndex = Math.max(0, waveformBuffer.length - windowSize)
            var slice = waveformBuffer.slice(startIndex)
            
            // Normalize data to fit display height
            // Expected range: -50 to +150 (millivolts for ECG)
            // This range covers typical ECG amplitudes (P, QRS, T waves)
            var minValue = -50
            var maxValue = 150
            var range = maxValue - minValue
            
            // Apply gain (user-adjustable amplitude scaling)
            var effectiveRange = range / gain
            
            // Configure line style
            ctx.strokeStyle = waveformColor
            ctx.lineWidth = 2
            ctx.lineJoin = "round"  // Smooth line connections
            ctx.lineCap = "round"
            
            // Optional: Add subtle glow effect (shadow)
            ctx.shadowColor = Qt.rgba(
                waveformColor.r,
                waveformColor.g,
                waveformColor.b,
                0.3
            )
            ctx.shadowBlur = 3
            
            // Draw waveform line
            ctx.beginPath()
            
            for (var i = 0; i < slice.length; i++) {
                // Calculate X position (left to right, newest on right)
                // X = 0 at left edge, X = width at right edge
                var x = (i / (slice.length - 1)) * width
                
                // Calculate Y position (normalized to display height)
                // Y = 0 at top, Y = height at bottom
                // Center (baseline) is at height / 2
                var normalizedValue = (slice[i] - minValue) / effectiveRange
                var y = height - (normalizedValue * height)
                
                // Clamp Y to visible area (prevent drawing outside canvas)
                y = Math.max(0, Math.min(height, y))
                
                // Move to first point, then draw lines to subsequent points
                if (i === 0) {
                    ctx.moveTo(x, y)
                } else {
                    ctx.lineTo(x, y)
                }
            }
            
            // Stroke the path (draw the line)
            ctx.stroke()
        }
    }
    
    // === SMOOTH SCROLLING TIMER ===
    
    /**
     * Timer ensures canvas repaints at 60 FPS (every ~16.67ms)
     * 
     * This creates smooth scrolling effect even when new samples
     * arrive at irregular intervals (e.g., network jitter).
     */
    Timer {
        id: refreshTimer
        interval: 16  // ~60 FPS (1000ms / 60 = 16.67ms)
        running: true
        repeat: true
        
        onTriggered: {
            // Request repaint every frame
            // This creates smooth scrolling even if no new data arrived
            canvas.requestPaint()
        }
    }
    
    // === SCAN LINE EFFECT (Optional) ===
    
    /**
     * Scan line creates "sweeping" effect on right edge
     * Mimics traditional medical monitor appearance.
     */
    Rectangle {
        anchors.top: canvas.top
        anchors.right: parent.right
        anchors.bottom: canvas.bottom
        width: 3
        z: 20  // Above waveform
        opacity: 0.6
        
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: root.color }
        }
    }
}
```

### 5.2. WaveformController (C++ QObject)

The C++ controller bridges waveform data from `WaveformCache` to QML:

```cpp
/**
 * @class WaveformController
 * @brief QObject controller for waveform display in QML
 * 
 * Exposes waveform samples from WaveformCache to QML WaveformChart component.
 * Runs on main/UI thread for safe QML property updates.
 */
class WaveformController : public QObject {
    Q_OBJECT
    
    /**
     * @property QVariantList samples
     * @brief Array of waveform sample values exposed to QML
     * 
     * QML can bind to this property:
     * WaveformChart { dataPoints: waveformController.samples }
     * 
     * Updated whenever new samples arrive from WaveformCache.
     */
    Q_PROPERTY(QVariantList samples READ samples NOTIFY samplesChanged)
    
public:
    explicit WaveformController(QObject* parent = nullptr);
    
    /**
     * @brief Update samples from WaveformCache
     * 
     * Called by MonitoringService when new waveform data arrives.
     * 
     * @param newSamples Array of sample values (typically 250 samples for 1 second)
     */
    Q_INVOKABLE void updateSamples(const QVariantList& newSamples);
    
    /**
     * @brief Get current samples (for QML binding)
     */
    QVariantList samples() const { return m_samples; }
    
signals:
    /**
     * @brief Emitted when samples change
     * 
     * QML automatically updates bound properties when this signal fires.
     */
    void samplesChanged();
    
private:
    QVariantList m_samples;  // Current waveform samples
    WaveformCache* m_cache;  // Reference to waveform cache
};
```

### 5.3. Integration Example

How to use `WaveformChart` in a QML view:

```qml
import QtQuick 2.15
import QtQuick.Layouts 1.12

/**
 * DashboardView.qml - Main monitoring view
 * 
 * Shows real-time vital signs and waveforms.
 */
Item {
    id: dashboardView
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Vital signs cards (heart rate, SpO2, etc.)
        RowLayout {
            // ... vital signs display ...
        }
        
        // ECG Waveform
        WaveformChart {
            id: ecgWaveform
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            
            // Bind to C++ controller
            dataPoints: waveformController.samples
            
            // Configure for ECG
            waveformColor: "#00ff88"  // Green
            timeWindowSeconds: 10
            sampleRate: 250
            gain: 1.0
        }
        
        // Plethysmogram Waveform
        WaveformChart {
            id: plethWaveform
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            
            // Bind to C++ controller
            dataPoints: plethController.samples
            
            // Configure for Pleth
            waveformColor: "#ff4444"  // Red
            timeWindowSeconds: 10
            sampleRate: 125
            gain: 1.0
        }
    }
    
    // C++ controllers (registered in main.cpp)
    WaveformController {
        id: waveformController
        // Automatically receives data from MonitoringService
    }
    
    WaveformController {
        id: plethController
        // Separate controller for plethysmogram
    }
}
```

## 6. Code Explanation

### 6.1. For Readers Unfamiliar with the Project

**What is QML?**
- QML (Qt Modeling Language) is a declarative language for building user interfaces
- Similar to HTML/CSS but for desktop/mobile applications
- Uses JavaScript for logic, Canvas API for custom graphics

**What is Canvas?**
- Qt Quick Canvas provides 2D drawing API (like HTML5 Canvas)
- `getContext("2d")` returns a 2D drawing context
- `onPaint` handler is called whenever `requestPaint()` is invoked
- Similar to `paintEvent()` in Qt Widgets

**How does data flow?**
1. **Sensor Simulator** generates waveform samples at 250 Hz (4ms intervals)
2. **WebSocketSensorDataSource** receives samples via WebSocket
3. **MonitoringService** stores samples in **WaveformCache** (circular buffer)
4. **WaveformController** (C++) exposes samples as Q_PROPERTY
5. **WaveformChart.qml** binds to controller's `samples` property
6. **Canvas** renders waveform line at 60 FPS

**Why 60 FPS?**
- Human eye perceives smooth motion at 60 FPS
- Lower frame rates (30 FPS) appear choppy
- Medical monitors require smooth scrolling for clinical interpretation
- 60 FPS = 16.67ms per frame (must complete rendering in < 16ms)

**Why 250 Hz sample rate?**
- Clinical standard for ECG capture
- Captures all important waveform features (P, QRS, T waves)
- Higher rates (500 Hz) unnecessary for monitoring
- Lower rates (125 Hz) may miss rapid changes

### 6.2. Key Concepts

**Circular Buffer:**
- Fixed-size buffer that overwrites oldest data when full
- Example: Buffer of 7,500 samples (30 seconds at 250 Hz)
- When buffer full, new sample overwrites oldest sample
- Provides sliding window of recent data

**Normalization:**
- Converts raw sample values (e.g., -50 to +150 mV) to screen coordinates (0 to height pixels)
- Formula: `y = height - ((value - min) / range) * height`
- Centers waveform at `height / 2` (baseline)

**Smooth Scrolling:**
- Timer triggers repaint every 16ms (60 FPS)
- Even if no new data, canvas repaints (creates scrolling effect)
- New data arrives at 4ms intervals (250 Hz), but display updates at 16ms (60 FPS)
- This decouples data rate from display rate

**Window Extraction:**
- Only displays last N samples (e.g., 2,500 for 10 seconds)
- `slice = buffer.slice(-windowSize)` extracts last N samples
- As new samples arrive, window shifts right (oldest samples drop off left)

## 7. Refresh Rate Requirements

### 7.1. Why 60 FPS?

**Human Perception:**
- Human eye can detect motion up to ~60 FPS
- Below 30 FPS: Motion appears choppy
- 60 FPS: Smooth, fluid motion
- Higher (120 FPS): Diminishing returns, unnecessary CPU/GPU load

**Clinical Requirements:**
- Medical monitors require smooth scrolling for accurate interpretation
- Stuttering or frame drops can mask important waveform features
- IEC 60601-1-8 (medical device standard) recommends smooth display

**Performance Target:**
- **60 FPS = 16.67ms per frame**
- Canvas rendering must complete in < 16ms
- If rendering takes > 16ms, frame drops occur

### 7.2. How to Achieve 60 FPS

**1. Efficient Rendering:**
```qml
onPaint: {
    var ctx = getContext("2d")
    ctx.clearRect(0, 0, width, height)  // Fast clear
    
    // Only draw visible window (not entire buffer)
    var slice = waveformBuffer.slice(-windowSize)
    
    // Use single path (not multiple paths)
    ctx.beginPath()
    for (var i = 0; i < slice.length; i++) {
        // ... calculate x, y ...
        if (i === 0) ctx.moveTo(x, y)
        else ctx.lineTo(x, y)
    }
    ctx.stroke()  // Single stroke call (fast)
}
```

**2. Timer-Based Refresh:**
```qml
Timer {
    interval: 16  // ~60 FPS
    running: true
    repeat: true
    onTriggered: canvas.requestPaint()
}
```

**3. Avoid Expensive Operations:**
- ❌ Don't recalculate grid every frame (paint once)
- ❌ Don't use complex gradients or shadows (if performance issues)
- ❌ Don't process entire buffer (only visible window)
- ✅ Use simple line drawing (fast)
- ✅ Pre-calculate constants (min, max, range)
- ✅ Limit window size (2,500 samples max)

**4. Profile and Optimize:**
- Use Qt Creator's QML Profiler to measure frame time
- Target: < 16ms per frame
- If > 16ms: Reduce window size, simplify rendering, optimize loops

### 7.3. Frame Drop Detection

Monitor frame drops in development:

```qml
property int frameCount: 0
property int droppedFrames: 0
property int lastFrameTime: 0

Timer {
    interval: 16
    running: true
    repeat: true
    
    onTriggered: {
        var currentTime = Date.now()
        var elapsed = currentTime - lastFrameTime
        
        // If elapsed > 20ms, we likely dropped a frame
        if (elapsed > 20 && lastFrameTime > 0) {
            droppedFrames++
            console.warn("Frame drop detected:", elapsed, "ms")
        }
        
        lastFrameTime = currentTime
        frameCount++
        canvas.requestPaint()
    }
}
```

## 8. Smoothness Requirements

### 8.1. No Jumping

**Problem:** Waveform "jumps" when new data arrives
- **Cause:** Abrupt position change when buffer updates
- **Solution:** Smooth interpolation between frames

**Implementation:**
```qml
// Instead of abrupt buffer update:
waveformBuffer = newBuffer  // ❌ Causes jump

// Use smooth transition:
property var targetBuffer: []
property var currentBuffer: []

function updateBuffer(newBuffer) {
    targetBuffer = newBuffer
    // Smoothly interpolate currentBuffer toward targetBuffer
    // over several frames (e.g., 3-5 frames)
}
```

**Alternative (Simpler):**
- Keep buffer large enough (3,000 samples) to allow smooth scrolling
- New samples append to end, oldest drop from beginning
- No position jumps if buffer size > window size

### 8.2. No Stuttering

**Problem:** Waveform stutters (pauses, then jumps forward)
- **Cause:** Irregular data arrival or slow rendering
- **Solution:** Timer-based refresh + efficient rendering

**Implementation:**
```qml
// Timer ensures consistent refresh rate
Timer {
    interval: 16  // 60 FPS
    running: true
    repeat: true
    onTriggered: canvas.requestPaint()
}

// Even if data arrives irregularly, display updates smoothly
```

### 8.3. Continuous Scrolling

**Problem:** Waveform appears static, doesn't scroll
- **Cause:** Not updating display frequently enough
- **Solution:** Timer-based refresh + window shifting

**Implementation:**
```qml
// Timer triggers repaint every frame
Timer {
    interval: 16
    running: true
    repeat: true
    onTriggered: canvas.requestPaint()
}

// Each frame, extract window (creates scrolling effect)
onPaint: {
    var slice = waveformBuffer.slice(-windowSize)
    // Render slice (newest data on right)
}
```

## 9. Best Practices

### 9.1. Performance Optimization

1. **Limit Window Size:**
   - Display only necessary samples (e.g., 2,500 for 10 seconds)
   - Don't render entire buffer (7,500 samples)

2. **Pre-calculate Constants:**
   ```qml
   readonly property real minValue: -50
   readonly property real maxValue: 150
   readonly property real range: maxValue - minValue
   ```

3. **Use Simple Drawing:**
   - Single path with `moveTo()` and `lineTo()`
   - Avoid complex shapes, gradients, or filters

4. **Grid Rendering:**
   - Render grid once (not every frame)
   - Use separate canvas layer (z: 0)

### 9.2. Memory Management

1. **Circular Buffer:**
   - Fixed size (e.g., 3,000 samples)
   - Automatically removes oldest samples

2. **Garbage Collection:**
   - Avoid creating new arrays in `onPaint` (use existing buffer)
   - Reuse variables where possible

### 9.3. Thread Safety

1. **UI Thread Only:**
   - WaveformController runs on main/UI thread
   - WaveformCache provides thread-safe access

2. **Signal/Slot:**
   - Use Qt signals to pass data from background thread to UI thread
   - Don't access QML properties from background threads

## 10. Testing

### 10.1. Performance Testing

**Measure Frame Time:**
```qml
property int lastPaintTime: 0

onPaint: {
    var startTime = Date.now()
    
    // ... rendering code ...
    
    var endTime = Date.now()
    var frameTime = endTime - startTime
    
    if (frameTime > 16) {
        console.warn("Frame time exceeded:", frameTime, "ms")
    }
    
    lastPaintTime = frameTime
}
```

**Target:** < 16ms per frame (60 FPS)

### 10.2. Visual Testing

1. **Smooth Scrolling:**
   - Waveform should scroll continuously from right to left
   - No pauses, jumps, or stuttering

2. **Frame Drops:**
   - Monitor frame drop rate (< 1% target)
   - Use Qt Creator QML Profiler

3. **Amplitude Accuracy:**
   - Verify waveform amplitude matches expected values
   - Test with known test signals (e.g., 1 mV square wave)

## 11. Troubleshooting

### 11.1. Waveform Not Updating

**Symptoms:** Waveform appears static
**Causes:**
- Timer not running
- `requestPaint()` not called
- Data not arriving from controller

**Solution:**
```qml
// Verify timer is running
Timer {
    id: refreshTimer
    running: true  // ← Must be true
    repeat: true
    interval: 16
    onTriggered: canvas.requestPaint()
}

// Verify data binding
WaveformChart {
    dataPoints: waveformController.samples  // ← Check binding
}
```

### 11.2. Frame Drops

**Symptoms:** Stuttering, choppy scrolling
**Causes:**
- Rendering too slow (> 16ms)
- Window size too large
- Complex rendering operations

**Solution:**
- Reduce window size (fewer samples to render)
- Simplify rendering (remove shadows, gradients)
- Profile with QML Profiler to find bottleneck

### 11.3. Waveform Jumping

**Symptoms:** Abrupt position changes
**Causes:**
- Buffer size < window size
- Abrupt buffer updates

**Solution:**
- Increase buffer size (e.g., 3,000 samples for 2,500 window)
- Use smooth interpolation for buffer updates

## 12. Related Documents

- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../requirements/03_FUNCTIONAL_REQUIREMENTS.md) - REQ-FUN-VITAL-011
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - REQ-NFR-PERF-101
- **Data Caching:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - WaveformCache design
- **Sensor Integration:** [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) - Waveform data source
- **Thread Model:** [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Threading architecture

---

*This implementation guide provides a complete reference for implementing smooth, real-time waveform display in the Z Monitor application. The sample code can be adapted for ECG, plethysmogram, or other waveform types.*

