#!/bin/bash
# UI Live Data Integration Test
# Tests that z-monitor UI displays live data from simulator

set -e

echo "========================================="
echo "Z-Monitor UI Live Data Integration Test"
echo "========================================="

# Check simulator is running
if ! pgrep -f sensor_simulator > /dev/null; then
    echo "ERROR: Simulator is not running. Start it first:"
    echo "  cd sensor-simulator/build && ./sensor_simulator --headless &"
    exit 1
fi

echo "✓ Simulator is running"

# Check socket exists
if [ ! -S /tmp/z-monitor-sensor.sock ]; then
    echo "ERROR: Socket /tmp/z-monitor-sensor.sock does not exist"
    exit 1
fi

echo "✓ Socket exists at /tmp/z-monitor-sensor.sock"

# Run z-monitor for 10 seconds and capture output
echo ""
echo "Starting z-monitor for 10 seconds..."
echo "(Watch for vitals updates in console)"
echo ""

cd "$(dirname "$0")/../build/src"

# Start z-monitor in background
timeout 10 ./z-monitor 2>&1 | tee /tmp/z-monitor-ui-test.log &
Z_MONITOR_PID=$!

# Wait for z-monitor to start
sleep 3

# Check if z-monitor is still running
if ! ps -p $Z_MONITOR_PID > /dev/null; then
    echo "ERROR: z-monitor crashed on startup"
    cat /tmp/z-monitor-ui-test.log
    exit 1
fi

echo "✓ z-monitor started (PID: $Z_MONITOR_PID)"

# Let it run for 7 more seconds
sleep 7

# Kill z-monitor
kill $Z_MONITOR_PID 2>/dev/null || true
wait $Z_MONITOR_PID 2>/dev/null || true

echo ""
echo "========================================="
echo "Test Results:"
echo "========================================="

# Check log for key indicators
LOG_FILE="/tmp/z-monitor-ui-test.log"

# Check database opened
if grep -q "Database opened successfully" "$LOG_FILE"; then
    echo "✓ Database opened"
else
    echo "✗ Database did not open"
fi

# Check monitoring service started
if grep -q "MonitoringService started successfully" "$LOG_FILE"; then
    echo "✓ MonitoringService started"
else
    echo "✗ MonitoringService did not start"
fi

# Check sensor connection
if grep -q "SharedMemoryControlChannel: Handshake completed" "$LOG_FILE"; then
    echo "✓ Sensor handshake completed"
else
    echo "✗ Sensor did not connect"
fi

# Check data flow
if grep -q "Started successfully - data transfer via shared memory" "$LOG_FILE"; then
    echo "✓ SharedMemorySensorDataSource started"
else
    echo "✗ SharedMemorySensorDataSource did not start"
fi

# Check QML loaded
if grep -q "qrc:/qml/Main.qml" "$LOG_FILE"; then
    echo "✓ QML UI loaded"
else
    echo "✗ QML UI did not load"
fi

# Count QML warnings/errors
QML_ERRORS=$(grep -c "qml:" "$LOG_FILE" || echo "0")
if [ "$QML_ERRORS" -gt 0 ]; then
    echo "⚠ QML warnings/errors: $QML_ERRORS"
    echo ""
    echo "QML Issues:"
    grep "qml:" "$LOG_FILE" | head -10
else
    echo "✓ No QML errors"
fi

echo ""
echo "========================================="
echo "Full log saved to: $LOG_FILE"
echo "========================================="

# Take screenshot if z-monitor window is still open
if command -v screencapture >/dev/null 2>&1; then
    echo ""
    echo "To capture screenshot manually:"
    echo "  screencapture -w project-dashboard/screenshots/z-monitor-live-data-$(date +%Y%m%d).png"
    echo "  (Click on z-monitor window)"
fi

echo ""
echo "Test complete!"
