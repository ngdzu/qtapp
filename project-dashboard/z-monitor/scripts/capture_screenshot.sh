#!/usr/bin/env bash
set -euo pipefail

# Build & screenshot helper for Z Monitor.
# Usage: `cd project-dashboard/z-monitor && ./scripts/capture_screenshot.sh`
# Or: `cd project-dashboard/z-monitor && ./scripts/capture_screenshot.sh /path/to/output.png`

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
SCREENSHOTS_DIR="${APP_DIR}/../../project-dashboard/screenshots"
DEFAULT_OUTPUT="${SCREENSHOTS_DIR}/ui-baseline-v1.0.png"
OUTPUT_FILE="${1:-$DEFAULT_OUTPUT}"

# Create screenshots directory if it doesn't exist
mkdir -p "$(dirname "${OUTPUT_FILE}")"

echo "==> Building Z Monitor..."
pushd "${APP_DIR}" >/dev/null

# Set Qt path
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:${CMAKE_PREFIX_PATH:-}"

# Build the application
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1 | grep -E "error|Error|ERROR|\[100%\]" || true
cmake --build . --config Release -j 12 --target z-monitor 2>&1 | grep -E "error|Error|ERROR|\[100%\]|Built target" || true

popd >/dev/null

echo ""
echo "==> Build complete. Launching Z Monitor..."
echo ""

# Start the application
export QT_LOGGING_RULES="qt.*=false"
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:${CMAKE_PREFIX_PATH:-}"

echo "📸 MANUAL SCREENSHOT INSTRUCTIONS:"
echo "   1. Wait for the UI to fully render (waveforms should be animating)"
echo "   2. Take screenshot: Cmd+Shift+4, then Space, then click on Z Monitor window"
echo "   3. Save to: ${OUTPUT_FILE}"
echo "   4. Press Ctrl+C in this terminal to exit the application"
echo ""

"${APP_DIR}/build/src/z-monitor" 2>&1 | grep -v "^qt\."

echo ""
echo "==> Application closed."

# Check if screenshot was created
if [ -f "${OUTPUT_FILE}" ]; then
    echo "==> ✅ Screenshot found at ${OUTPUT_FILE}"
    
    # Show image dimensions
    if command -v sips >/dev/null 2>&1; then
        DIMENSIONS=$(sips -g pixelWidth -g pixelHeight "${OUTPUT_FILE}" | grep -E 'pixelWidth|pixelHeight' | awk '{print $2}' | tr '\n' 'x' | sed 's/x$//')
        echo "==> Image size: ${DIMENSIONS}"
    fi
else
    echo "==> ⚠️  Screenshot not found at ${OUTPUT_FILE}"
    echo "    Please capture manually and save to this location."
fi
