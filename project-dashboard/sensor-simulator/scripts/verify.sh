#!/bin/bash
#
# Verification script for sensor simulator
#
# This script verifies that the simulator creates the expected resources:
# - Unix domain socket at /tmp/z-monitor-sensor.sock
# - Shared memory ring buffer (via shm_open on macOS)
# - Integration test passes
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "=== Sensor Simulator Verification ==="
echo ""

# Check build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Build directory not found: $BUILD_DIR"
    echo "   Run ./scripts/build_local.sh first"
    exit 1
fi
echo "✅ Build directory exists"

# Check executable exists
if [ ! -f "$BUILD_DIR/sensor_simulator" ]; then
    echo "❌ Executable not found: $BUILD_DIR/sensor_simulator"
    exit 1
fi
echo "✅ Executable exists"

# Check integration test exists
if [ ! -f "$BUILD_DIR/integration_test" ]; then
    echo "❌ Integration test not found: $BUILD_DIR/integration_test"
    exit 1
fi
echo "✅ Integration test exists"

# Run integration test
echo ""
echo "Running integration test..."
if "$BUILD_DIR/integration_test" > /dev/null 2>&1; then
    echo "✅ Integration test PASSED"
else
    echo "❌ Integration test FAILED"
    exit 1
fi

# Start simulator in background
echo ""
echo "Starting simulator in background..."
rm -f /tmp/z-monitor-sensor.sock  # Clean up any old socket
"$BUILD_DIR/sensor_simulator" > /tmp/simulator.log 2>&1 &
SIMULATOR_PID=$!
sleep 2  # Give it time to initialize

# Check if process is still running
if ! kill -0 $SIMULATOR_PID 2>/dev/null; then
    echo "❌ Simulator process died"
    cat /tmp/simulator.log
    exit 1
fi
echo "✅ Simulator process running (PID: $SIMULATOR_PID)"

# Check for Unix socket
if [ -S /tmp/z-monitor-sensor.sock ]; then
    echo "✅ Unix socket created: /tmp/z-monitor-sensor.sock"
else
    echo "❌ Unix socket not found: /tmp/z-monitor-sensor.sock"
    kill $SIMULATOR_PID 2>/dev/null || true
    cat /tmp/simulator.log
    exit 1
fi

# Check simulator log for success messages
if grep -q "Shared memory initialized" /tmp/simulator.log; then
    echo "✅ Shared memory initialized"
else
    echo "❌ Shared memory initialization failed"
    kill $SIMULATOR_PID 2>/dev/null || true
    cat /tmp/simulator.log
    exit 1
fi

if grep -q "ControlServer: Listening" /tmp/simulator.log; then
    echo "✅ Control server listening"
else
    echo "❌ Control server failed to start"
    kill $SIMULATOR_PID 2>/dev/null || true
    cat /tmp/simulator.log
    exit 1
fi

# Clean up
echo ""
echo "Cleaning up..."
kill $SIMULATOR_PID 2>/dev/null || true
rm -f /tmp/z-monitor-sensor.sock
rm -f /tmp/simulator.log

echo ""
echo "=== All Verifications Passed ✅ ==="
echo ""
echo "Summary:"
echo "  - Build:          PASSED"
echo "  - Integration:    PASSED"
echo "  - Socket:         PASSED"
echo "  - Shared Memory:  PASSED"
echo "  - Control Server: PASSED"
