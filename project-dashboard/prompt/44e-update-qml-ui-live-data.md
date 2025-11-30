# Task: Update QML UI to Display Live Sensor Data with Waveform Rendering

## Context

**Documentation:**
- Architecture: [37_SENSOR_INTEGRATION.md](../doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md)
- Implementation Guide: [44_SIMULATOR_INTEGRATION_GUIDE.md](../doc/z-monitor/architecture_and_design/44_SIMULATOR_INTEGRATION_GUIDE.md) - Phase 5
- Waveform Display: [41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](../doc/z-monitor/architecture_and_design/41_WAVEFORM_DISPLAY_IMPLEMENTATION.md) (if exists)
- UI/UX Guide: [03_UI_UX_GUIDE.md](../doc/z-monitor/architecture_and_design/03_UI_UX_GUIDE.md)

**Previous Work:**
- ✅ Simulator built and running
- ✅ SharedMemorySensorDataSource implemented
- ✅ Controllers wired to MonitoringService with live data
- ⏳ Need QML UI to display live data with waveform rendering

**Dependencies:**
- `DashboardController` exposes vitals as Q_PROPERTY
- `WaveformController` exposes waveform data as QVariantList
- QML UI baseline implemented (VitalTile, WaveformPanel components exist)

---

## Objective

Update z-monitor QML UI to display live sensor data from controllers. Bind `DashboardController` Q_PROPERTY values to `VitalTile` components (Heart Rate, SpO2, NIBP, Resp Rate, Temperature). Implement real-time waveform rendering using QML Canvas API, binding to `WaveformController` waveform data arrays (ECG, Pleth, Resp waveforms). Replace hardcoded placeholder data with live controller bindings. Implement 60 FPS Canvas rendering with smooth scrolling waveforms. Add connection status indicator in header. Take screenshot of live UI showing real data from simulator.

**Visual Goal:** Real-time patient monitoring display with live vitals updating every second and ECG waveform scrolling smoothly across the screen.

---

## Architecture Overview

### Data Binding Flow

```
DashboardController (Q_PROPERTY)
  ↓ (QML property binding)
VitalTile { value: dashboardController.heartRate }
  ↓ (automatic UI update on property change)
Visual Update (60 Hz)

WaveformController (Q_PROPERTY: QVariantList)
  ↓ (QML property binding)
Canvas { onPaint: render waveformController.ecgSamples }
  ↓ (60 FPS rendering @ 16ms intervals)
Scrolling Waveform Display
```

**Performance Budget:**
- UI refresh rate: 60 FPS (16ms per frame)
- Canvas rendering: < 10ms per frame
- Property binding updates: < 5ms
- Total latency (sensor → UI visible): < 100ms perceived by user

---

## Component Updates

### Component 1: Main.qml (Controller Instantiation)

**File:** `z-monitor/resources/qml/Main.qml`

**Update:**
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

// Import C++ controllers
import ZMonitor.Controllers 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    title: "Z Monitor - Patient Monitoring System"
    
    // Instantiate controllers (registered from C++)
    DashboardController {
        id: dashboardController
    }
    
    WaveformController {
        id: waveformController
    }
    
    // Connection status indicator
    ConnectionStatus {
        id: connectionStatus
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        
        connected: dashboardController.connected  // Assuming controller exposes this
        sensorType: "Shared Memory Simulator"
    }
    
    // Main content
    MonitorView {
        anchors.fill: parent
        dashboardController: dashboardController
        waveformController: waveformController
    }
}
```

---

### Component 2: MonitorView.qml (Vital Bindings)

**File:** `z-monitor/resources/qml/views/MonitorView.qml`

**Update:**
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: root
    
    // Controller references (passed from Main.qml)
    property var dashboardController
    property var waveformController
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Vitals Grid
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            rowSpacing: 16
            columnSpacing: 16
            
            // Heart Rate
            VitalTile {
                Layout.fillWidth: true
                label: "HEART RATE"
                value: dashboardController.heartRate.toString()
                unit: "BPM"
                color: "#10b981"
                normalRange: "60-100"
                
                // Optional: Add alarm indication
                alarmActive: dashboardController.heartRate < 60 || dashboardController.heartRate > 100
            }
            
            // SpO2
            VitalTile {
                Layout.fillWidth: true
                label: "SpO₂"
                value: dashboardController.spo2.toString()
                unit: "%"
                color: "#3b82f6"
                normalRange: "95-100"
                alarmActive: dashboardController.spo2 < 95
            }
            
            // Respiration Rate
            VitalTile {
                Layout.fillWidth: true
                label: "RESPIRATION RATE"
                value: dashboardController.respirationRate.toString()
                unit: "BPM"
                color: "#8b5cf6"
                normalRange: "12-20"
                alarmActive: dashboardController.respirationRate < 12 || dashboardController.respirationRate > 20
            }
            
            // NIBP (placeholder - not implemented in simulator yet)
            VitalTile {
                Layout.fillWidth: true
                label: "NIBP"
                value: dashboardController.nibp
                unit: "mmHg"
                color: "#f59e0b"
                normalRange: "120/80"
            }
            
            // Temperature (placeholder)
            VitalTile {
                Layout.fillWidth: true
                label: "TEMPERATURE"
                value: dashboardController.temperature.toFixed(1)
                unit: "°C"
                color: "#ef4444"
                normalRange: "36.5-37.5"
            }
            
            // Last Update
            Text {
                Layout.fillWidth: true
                text: "Last Update: " + Qt.formatDateTime(dashboardController.lastUpdate, "hh:mm:ss")
                color: "#9ca3af"
                font.pixelSize: 12
            }
        }
        
        // Waveform Panel
        WaveformPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            waveformController: root.waveformController
        }
    }
}
```

---

### Component 3: VitalTile.qml (Ensure Binding Support)

**File:** `z-monitor/resources/qml/components/VitalTile.qml`

**Update (if needed):**
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    
    property string label: ""
    property string value: "--"
    property string unit: ""
    property color color: "#10b981"
    property string normalRange: ""
    property bool alarmActive: false
    
    width: 200
    height: 120
    radius: 8
    color: alarmActive ? "#ef4444" : "#1f2937"
    border.color: alarmActive ? "#dc2626" : root.color
    border.width: 2
    
    // Animate value changes
    Behavior on value {
        NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
    }
    
    // Flash animation for alarms
    SequentialAnimation {
        running: alarmActive
        loops: Animation.Infinite
        
        PropertyAnimation {
            target: root
            property: "opacity"
            from: 1.0
            to: 0.6
            duration: 500
        }
        PropertyAnimation {
            target: root
            property: "opacity"
            from: 0.6
            to: 1.0
            duration: 500
        }
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 8
        
        Text {
            text: root.label
            color: "#9ca3af"
            font.pixelSize: 14
            font.weight: Font.Medium
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter
            
            Text {
                text: root.value
                color: "white"
                font.pixelSize: 36
                font.weight: Font.Bold
            }
            
            Text {
                text: root.unit
                color: "#9ca3af"
                font.pixelSize: 18
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        Text {
            text: "Normal: " + root.normalRange
            color: "#6b7280"
            font.pixelSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
```

---

### Component 4: WaveformPanel.qml (Canvas Rendering)

**File:** `z-monitor/resources/qml/components/WaveformPanel.qml`

**Implementation:**
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    
    property var waveformController
    
    color: "#111827"
    radius: 8
    border.color: "#374151"
    border.width: 1
    
    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // ECG Waveform
        WaveformCanvas {
            id: ecgCanvas
            width: parent.width
            height: (parent.height - 32) / 3
            
            label: "ECG Lead II"
            color: "#10b981"
            sampleRate: 250
            
            // Bind to controller data
            samples: waveformController.ecgSamples
        }
        
        // Pleth Waveform
        WaveformCanvas {
            id: plethCanvas
            width: parent.width
            height: (parent.height - 32) / 3
            
            label: "Pleth"
            color: "#3b82f6"
            sampleRate: 250
            
            samples: waveformController.plethSamples
        }
        
        // Respiration Waveform
        WaveformCanvas {
            id: respirationCanvas
            width: parent.width
            height: (parent.height - 32) / 3
            
            label: "Respiration"
            color: "#8b5cf6"
            sampleRate: 60
            
            samples: waveformController.respirationSamples
        }
    }
}
```

---

### Component 5: WaveformCanvas.qml (Canvas-based Rendering)

**File:** `z-monitor/resources/qml/components/WaveformCanvas.qml`

**Implementation:**
```qml
import QtQuick 2.15

Item {
    id: root
    
    property string label: ""
    property color color: "#10b981"
    property int sampleRate: 250
    property var samples: []  // Array of values from controller
    
    // Rendering parameters
    property real scrollSpeed: 2.0  // Pixels per sample (controls sweep speed)
    property real amplitude: 0.6    // Vertical scale (0.0-1.0)
    
    // Label
    Text {
        id: labelText
        text: root.label
        color: "#9ca3af"
        font.pixelSize: 14
        font.weight: Font.Medium
        anchors.left: parent.left
        anchors.top: parent.top
    }
    
    // Canvas for waveform rendering
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.topMargin: 24
        
        // Redraw when samples change
        onSamplesChanged: {
            requestPaint();
        }
        
        // Redraw at 60 FPS
        Timer {
            interval: 16  // ~60 FPS
            running: true
            repeat: true
            onTriggered: canvas.requestPaint()
        }
        
        onPaint: {
            var ctx = getContext("2d");
            
            // Clear canvas
            ctx.clearRect(0, 0, width, height);
            
            // Draw grid (optional - improves clinical readability)
            drawGrid(ctx);
            
            // Draw waveform
            drawWaveform(ctx);
        }
        
        function drawGrid(ctx) {
            ctx.strokeStyle = "#1f2937";
            ctx.lineWidth = 1;
            
            // Vertical grid lines (every 50px)
            for (var x = 0; x < width; x += 50) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, height);
                ctx.stroke();
            }
            
            // Horizontal grid lines (every 25px)
            for (var y = 0; y < height; y += 25) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(width, y);
                ctx.stroke();
            }
            
            // Center line (stronger)
            ctx.strokeStyle = "#374151";
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(0, height / 2);
            ctx.lineTo(width, height / 2);
            ctx.stroke();
        }
        
        function drawWaveform(ctx) {
            if (!root.samples || root.samples.length === 0) {
                return;
            }
            
            ctx.strokeStyle = root.color;
            ctx.lineWidth = 2;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";
            
            ctx.beginPath();
            
            var centerY = height / 2;
            var scaleY = (height / 2) * root.amplitude;
            
            // Draw waveform from right to left (scrolling effect)
            var sampleCount = Math.min(root.samples.length, Math.floor(width / root.scrollSpeed));
            
            for (var i = 0; i < sampleCount; i++) {
                // X position: start from right edge, move left
                var x = width - (i * root.scrollSpeed);
                
                // Y position: scale sample value (-1 to 1) to canvas height
                var sampleIndex = root.samples.length - 1 - i;
                var sampleValue = root.samples[sampleIndex];
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
    
    // Sample rate indicator
    Text {
        text: root.sampleRate + " Hz"
        color: "#6b7280"
        font.pixelSize: 12
        anchors.right: parent.right
        anchors.top: parent.top
    }
}
```

---

### Component 6: ConnectionStatus.qml (Connection Indicator)

**File:** `z-monitor/resources/qml/components/ConnectionStatus.qml`

**Implementation:**
```qml
import QtQuick 2.15

Rectangle {
    id: root
    
    property bool connected: false
    property string sensorType: "Unknown"
    
    width: 200
    height: 40
    radius: 20
    color: connected ? "#10b98120" : "#ef444420"
    border.color: connected ? "#10b981" : "#ef4444"
    border.width: 2
    
    Row {
        anchors.centerIn: parent
        spacing: 8
        
        Rectangle {
            width: 12
            height: 12
            radius: 6
            color: connected ? "#10b981" : "#ef4444"
            anchors.verticalCenter: parent.verticalCenter
            
            // Pulse animation when connected
            SequentialAnimation {
                running: connected
                loops: Animation.Infinite
                
                PropertyAnimation {
                    target: parent
                    property: "opacity"
                    from: 1.0
                    to: 0.3
                    duration: 1000
                }
                PropertyAnimation {
                    target: parent
                    property: "opacity"
                    from: 0.3
                    to: 1.0
                    duration: 1000
                }
            }
        }
        
        Text {
            text: connected ? "Connected" : "Disconnected"
            color: connected ? "#10b981" : "#ef4444"
            font.pixelSize: 14
            font.weight: Font.Medium
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Text {
            text: "(" + root.sensorType + ")"
            color: "#9ca3af"
            font.pixelSize: 12
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
```

---

## Implementation Steps

### Step 1: Register Controllers with QML

**File:** `z-monitor/src/main.cpp`

**Update:**
```cpp
#include <QQmlContext>
#include "interface/controllers/DashboardController.h"
#include "interface/controllers/WaveformController.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    
    // Create service layer
    auto* sensorDataSource = new SharedMemorySensorDataSource(
        "/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    auto* vitalsCache = new VitalsCache();
    auto* waveformCache = new WaveformCache();
    auto* alarmManager = new AlarmManager();
    auto* monitoringService = new MonitoringService(
        sensorDataSource, vitalsCache, waveformCache, alarmManager);
    
    // Create controllers
    auto* dashboardController = new DashboardController(monitoringService);
    auto* waveformController = new WaveformController(monitoringService, waveformCache);
    
    // Register controllers with QML
    qmlRegisterType<DashboardController>("ZMonitor.Controllers", 1, 0, "DashboardController");
    qmlRegisterType<WaveformController>("ZMonitor.Controllers", 1, 0, "WaveformController");
    
    // Or use context properties (simpler, but less type-safe)
    // engine.rootContext()->setContextProperty("dashboardController", dashboardController);
    // engine.rootContext()->setContextProperty("waveformController", waveformController);
    
    // Start monitoring service
    monitoringService->start();
    
    // Load QML
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    
    return app.exec();
}
```

---

### Step 2: Update QML Files

1. Update `Main.qml` with controller instantiation
2. Update `MonitorView.qml` with live data bindings
3. Ensure `VitalTile.qml` supports animations
4. Implement `WaveformPanel.qml` with Canvas rendering
5. Create `WaveformCanvas.qml` component
6. Create `ConnectionStatus.qml` component

---

### Step 3: Test Live Data Display

**Manual Testing:**
1. Start simulator
2. Start z-monitor
3. Verify vitals update every second (visible changes)
4. Verify waveforms scroll smoothly (no stuttering)
5. Verify connection status shows "Connected"

**Automated Testing:**
```cpp
// Visual regression test (screenshot comparison)
TEST(UIIntegration, LiveDataDisplay) {
    // Start simulator
    QProcess simulator;
    simulator.start("./sensor_simulator");
    QTest::qWait(2000);
    
    // Start z-monitor
    QApplication app;
    QQmlApplicationEngine engine;
    engine.load("qrc:/qml/Main.qml");
    QTest::qWait(5000);
    
    // Capture screenshot
    QPixmap screenshot = QPixmap::grabWindow(engine.rootObjects()[0]->winId());
    screenshot.save("test_output/live_ui_screenshot.png");
    
    // Compare with baseline (visual regression)
    // compareImages("test_output/live_ui_screenshot.png", "baseline/ui_with_live_data.png");
}
```

---

### Step 4: Capture Screenshot

**Commands:**
```bash
# Capture screenshot (macOS)
screencapture -w project-dashboard/screenshots/z-monitor-live-data-v1.0.png
# Click on z-monitor window to capture

# Or use Qt API in code
QPixmap screenshot = QPixmap::grabWindow(root->winId());
screenshot.save("screenshots/z-monitor-live-data-v1.0.png");
```

**Screenshot Requirements:**
- Resolution: 1280x800
- Content:
  - All VitalTiles showing live values (not "--")
  - ECG waveform with PQRST complex pattern visible
  - Pleth and Respiration waveforms visible
  - Connection status showing "Connected"
  - Last Update timestamp showing recent time

---

## Troubleshooting

### Vitals Don't Update in UI

**Symptoms:** VitalTiles show "--" or stale values.

**Solution:**
1. Verify controllers are instantiated correctly in Main.qml
2. Check signal connections (MonitoringService → DashboardController)
3. Verify Q_PROPERTY NOTIFY signals are emitted
4. Add debug logging: `console.log("Heart Rate:", dashboardController.heartRate)`

---

### Waveforms Don't Render

**Symptoms:** Blank Canvas or no waveform visible.

**Solution:**
1. Verify `waveformController.ecgSamples` is not empty (log to console)
2. Check Canvas `onPaint` is called (add `console.log` in drawWaveform)
3. Verify sample values are in correct range (-1 to 1)
4. Check `scrollSpeed` and `amplitude` parameters

---

### Waveforms Stutter

**Symptoms:** Waveform rendering is choppy, not smooth 60 FPS.

**Solution:**
1. Optimize Canvas drawing (reduce point count)
2. Use `requestAnimationFrame` equivalent (Timer with 16ms interval)
3. Profile with Qt QML Profiler (find bottlenecks)
4. Consider using QtQuick.Shapes for hardware-accelerated rendering

---

### Performance Issues

**Symptoms:** UI freezes or laggy, CPU usage high.

**Solution:**
1. Profile with Qt QML Profiler
2. Reduce Canvas redraw frequency (reduce Timer interval)
3. Limit sample count (only render last 10 seconds)
4. Check for unnecessary re-renders (optimize property bindings)

---

## Acceptance Criteria

- [ ] All VitalTile components display live data from DashboardController
- [ ] Values update in real-time (60 Hz update rate visible)
- [ ] Waveforms render smoothly at 60 FPS using Canvas
- [ ] ECG waveform shows realistic PQRST complex pattern
- [ ] Pleth and Resp waveforms render correctly
- [ ] Waveforms scroll right-to-left smoothly (no stuttering)
- [ ] Connection status indicator works (connected/disconnected/stalled states)
- [ ] No QML errors or warnings in console
- [ ] Screenshot captured showing live data (vitals + waveforms)

---

## Verification Checklist

### 1. Functional
- [ ] UI displays live data from simulator
- [ ] Vitals update at 60 Hz (visible changes every second)
- [ ] Waveforms render at 60 FPS (smooth scrolling)
- [ ] Values match simulator output
- [ ] Connection status accurate
- [ ] UI responsive during data updates

### 2. Code Quality
- [ ] QML follows Qt Quick best practices
- [ ] Proper property bindings (no JavaScript updates in loops)
- [ ] Canvas rendering optimized
- [ ] No memory leaks in QML
- [ ] Doxygen comments for complex QML components

### 3. Documentation
- [ ] QML binding patterns documented
- [ ] Waveform rendering documented in `41_WAVEFORM_DISPLAY_IMPLEMENTATION.md`
- [ ] Screenshot captured and stored

### 4. Integration
- [ ] End-to-end test with simulator shows live UI updates
- [ ] Latency acceptable (< 100ms perceived)
- [ ] Visual comparison with Node.js reference UI (if exists)

### 5. Tests
- [ ] QML component tests for VitalTile bindings
- [ ] Canvas rendering smoke test
- [ ] Visual regression test (screenshot comparison)

---

## Performance Targets

- **UI refresh rate:** 60 FPS (16ms per frame)
- **Waveform Canvas rendering:** < 10ms per frame
- **Property binding updates:** < 5ms
- **Total latency (sensor → UI visible):** < 100ms perceived by user

---

## Next Steps

After UI integration complete:
1. Proceed to **44f-e2e-testing-and-validation.md** (comprehensive testing)
2. Measure latency end-to-end
3. Optimize performance hot spots
4. Document any QML patterns for future reference

---

**Estimated Time:** 2-3 hours (includes QML updates, Canvas implementation, testing, screenshot capture)
