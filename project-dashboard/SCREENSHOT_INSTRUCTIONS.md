# Z Monitor UI Screenshot Instructions

## Application Status
✅ **Application Built Successfully**
✅ **QML UI Converted from Node.js Reference**
✅ **Application Runs at 1280x800 Resolution**

## To Capture Baseline Screenshot:

### 1. Launch the Application
```bash
cd /Users/dustinwind/Development/Qt/qtapp/project-dashboard/z-monitor
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
./build/src/z-monitor
```

### 2. Take Screenshot
The application window will display at 1280x800 showing:
- **Header (Top)**: Patient banner (DOE, JOHN / BED-04), user info (SARAH JOHNSON / CLINICIAN), clock, connection status (CMS ONLINE / mTLS), battery (98%), notification bell
- **Main Content**: MonitorView with:
  - **Left (67%)**: 3 waveform panels - ECG (green), Pleth (blue), Resp (yellow) with animated waveforms
  - **Right (33%)**: 5 vital tiles - Heart Rate (85 BPM), SpO2 (97%), NIBP (123/76 mmHg), Resp Rate (15 rpm), Temp (36.8°C)
- **Bottom Nav**: MONITOR (active/green), AI ANALYSIS, TRENDS, CODE BLUE (red), MENU, LOGOUT

### 3. Capture Screenshot Manually
Use macOS screenshot tools:
- **Cmd + Shift + 4** → Click and drag to select the z-monitor window
- Or **Cmd + Shift + 4 + Space** → Click on z-monitor window

### 4. Save Screenshot
Save the screenshot as:
```
/Users/dustinwind/Development/Qt/qtapp/project-dashboard/screenshots/ui-baseline-v1.0.png
```

### 5. Commit to Git
```bash
cd /Users/dustinwind/Development/Qt/qtapp
git add project-dashboard/screenshots/ui-baseline-v1.0.png
git add project-dashboard/z-monitor/resources/qml/
git add project-dashboard/z-monitor/src/CMakeLists.txt
git commit -m "UI Baseline v1.0: Convert Node.js reference UI to QML

- Implemented Main.qml with header, content area, bottom nav (1280x800 fixed)
- Created VitalTile, WaveformPanel, NavButton components
- Implemented MonitorView with waveforms (8 cols) + vitals (4 cols)
- Added Canvas-based 60 FPS waveform animation
- Fixed letterSpacing property for Qt 6 compatibility
- Added QML resources to CMakeLists.txt build

Matches Node.js reference design from sample_app/z-monitor
Screenshot: project-dashboard/screenshots/ui-baseline-v1.0.png"
```

## QML Files Created:
1. `/resources/qml/Main.qml` - Application entry point with layout
2. `/resources/qml/components/NavButton.qml` - Bottom nav button
3. `/resources/qml/components/VitalTile.qml` - Numeric vital display
4. `/resources/qml/components/WaveformPanel.qml` - Waveform with grid + Canvas
5. `/resources/qml/views/MonitorView.qml` - Main dashboard view
6. `/resources/qml/views/TrendsView.qml` - Placeholder
7. `/resources/qml/views/SettingsView.qml` - Placeholder
8. `/resources/qml/qml.qrc` - Resource file (updated)

## Design Fidelity:
✅ Dark cyberpunk theme (Zinc-950 background, high contrast)
✅ Fixed 1280x800 resolution
✅ Color palette matches (Green ECG, Blue Pleth, Yellow Resp)
✅ Layout matches (Header 60px, Bottom nav 64px, Main content fills)
✅ Typography and spacing match reference
✅ Component structure mirrors Node.js implementation
✅ Waveforms use Canvas API for 60 FPS rendering
