#!/bin/bash
#
# Build script for sensor simulator (macOS local build)
#
# This script builds the sensor simulator with Qt 6.9.2 for local execution
# on macOS. The simulator uses POSIX shm_open as a memfd polyfill for macOS
# compatibility.
#
# Usage:
#   ./scripts/build_local.sh [clean]
#
# Options:
#   clean - Remove build directory before building
#

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# Qt path (adjust if needed)
QT_PATH="${QT_PATH:-/Users/dustinwind/Qt/6.9.2/macos}"

echo "=== Sensor Simulator Build Script ==="
echo "Project: $PROJECT_DIR"
echo "Build:   $BUILD_DIR"
echo "Qt:      $QT_PATH"
echo ""

# Clean if requested
if [ "$1" = "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cd "$BUILD_DIR"
CMAKE_PREFIX_PATH="$QT_PATH" cmake ..

# Build
echo "Building..."
make -j8

echo ""
echo "=== Build Complete ==="
echo "Executable: $BUILD_DIR/sensor_simulator"
echo "Test:       $BUILD_DIR/integration_test"
echo ""
echo "Run the simulator:"
echo "  cd $BUILD_DIR && ./sensor_simulator"
echo ""
echo "Note: Simulator creates Unix socket at /tmp/z-monitor-sensor.sock"
echo "      and uses POSIX shared memory (shm_open polyfill on macOS)"
