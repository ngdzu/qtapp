#!/bin/bash
#
# configure_qt_path.sh
# Helper script to configure Qt path for CMake builds
#
# Usage:
#   source scripts/configure_qt_path.sh
#   or
#   . scripts/configure_qt_path.sh
#
# This sets CMAKE_PREFIX_PATH environment variable to point to Qt installation.
# The variable persists for the current shell session.

set -e

# Default Qt installation location (macOS)
DEFAULT_QT_PATH="/Users/dustinwind/Qt/6.9.2/macos"

# Check if Qt path is provided as argument
if [ -n "$1" ]; then
    QT_PATH="$1"
else
    QT_PATH="$DEFAULT_QT_PATH"
fi

# Verify Qt installation exists
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Qt installation not found at: $QT_PATH"
    echo "Please provide correct Qt path:"
    echo "  source scripts/configure_qt_path.sh /path/to/Qt/6.x.x/macos"
    return 1 2>/dev/null || exit 1
fi

# Verify Qt6Config.cmake exists
if [ ! -f "$QT_PATH/lib/cmake/Qt6/Qt6Config.cmake" ]; then
    echo "Error: Qt6Config.cmake not found at: $QT_PATH/lib/cmake/Qt6/Qt6Config.cmake"
    echo "Please verify Qt installation is complete."
    return 1 2>/dev/null || exit 1
fi

# Set CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH="$QT_PATH:${CMAKE_PREFIX_PATH}"

echo "✓ Qt path configured: $QT_PATH"
echo "✓ CMAKE_PREFIX_PATH set to: $CMAKE_PREFIX_PATH"
echo ""
echo "You can now run CMake:"
echo "  cd project-dashboard/z-monitor"
echo "  cmake -S . -B build"
echo ""
echo "Note: This configuration is only valid for the current shell session."
echo "To make it permanent, add to your ~/.zshrc or ~/.bashrc:"
echo "  export CMAKE_PREFIX_PATH=\"$QT_PATH:\$CMAKE_PREFIX_PATH\""

