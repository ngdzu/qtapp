# Sensor Simulator Screenshots

## Screenshot Status

**Note:** The sensor simulator builds and runs successfully on macOS, but GUI screenshots cannot be captured in a terminal/headless environment. The simulator requires a display server (X11 or macOS window server) to render the QML UI.

## Verification Results

The following components have been verified programmatically:

✅ **Build:** Simulator compiles successfully with Qt 6.9.2 on macOS
✅ **Integration Tests:** All structure compatibility tests pass
✅ **Unix Socket:** Creates `/tmp/z-monitor-sensor.sock` successfully
✅ **Shared Memory:** Initializes 8.4 MB ring buffer (2048 frames × 4096 bytes)
✅ **Control Server:** Listens on Unix socket and ready to exchange memfd descriptor

## How to Capture Screenshots

To capture screenshots of the running simulator UI:

1. **Build the simulator:**
   ```bash
   cd project-dashboard/sensor-simulator
   ./scripts/build_local.sh
   ```

2. **Run the simulator with display:**
   ```bash
   cd build
   ./sensor_simulator
   ```
   
   The QML UI will open showing:
   - Vitals display (Heart Rate, SpO2, Respiration Rate)
   - ECG waveform visualization
   - Control buttons (Critical, Warning, Notification, Demo)
   - Log console with telemetry stream

3. **Capture screenshot:**
   - macOS: Cmd+Shift+4, then press Space, click window
   - Save as: `simulator-baseline-v1.0.png` in this directory

## Expected UI Components

Based on the QML source code (`qml/Main.qml`), the UI should display:

- **Header:** "Telemetry Simulator" title
- **Vitals Cards:** Three cards showing HR, SpO2, RR with live values
- **Waveform Chart:** Real-time ECG waveform scrolling left-to-right
- **Control Buttons:** Trigger Critical, Warning, Notification, Play Demo
- **Log Console:** Filterable log stream (ALL/CRITICAL/WARNING/INFO/INTERNAL)
- **Status Indicator:** Connection status (socket listening)

## Technical Details

- **Resolution:** 1280x720 (default window size)
- **Update Rate:** 60 Hz vitals, 250 Hz waveforms
- **Transport:** Shared memory ring buffer + Unix socket control channel
- **Platform:** macOS (POSIX shm_open polyfill)
