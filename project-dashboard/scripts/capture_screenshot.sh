#!/usr/bin/env bash
set -euo pipefail

# Build & screenshot helper for the sensor simulator.
# Usage: `cd project-dashboard && ./scripts/capture_screenshot.sh`

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DASHBOARD_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
APP_DIR="${PROJECT_DASHBOARD_DIR}/sensor-simulator"
RESOURCES_DIR="${APP_DIR}/media"
OUTPUT_FILE="${RESOURCES_DIR}/screenshot.png"
IMAGE_NAME="qtapp-sensor-simulator:local"
CONTAINER_NAME="qt-sensor-sim-capture"

mkdir -p "${RESOURCES_DIR}"

pushd "${APP_DIR}" >/dev/null
echo "==> Building Docker image ${IMAGE_NAME} (context: ${APP_DIR})"
docker build -t "${IMAGE_NAME}" .
popd >/dev/null

echo "==> Removing existing container (if any)"
docker rm -f "${CONTAINER_NAME}" >/dev/null 2>&1 || true

echo "==> Running container and capturing screenshot to ${OUTPUT_FILE}"
docker run --rm --name "${CONTAINER_NAME}" \
    -v "${RESOURCES_DIR}":/opt/sensor-simulator/resources \
    "${IMAGE_NAME}" \
    bash -c '
        set -euo pipefail
        export DISPLAY=:99
        export QT_QPA_PLATFORM=xcb
        OUTPUT="/opt/sensor-simulator/media/screenshot.png"

        need_install=false
        if ! command -v xwd >/dev/null || ! command -v convert >/dev/null || ! command -v xdotool >/dev/null; then
            need_install=true
        fi

        if [ "$need_install" = true ]; then
            echo "Installing screenshot dependencies inside container..."
            apt-get update >/dev/null
            DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
                x11-apps imagemagick xdotool >/dev/null
        fi

        rm -f "$OUTPUT"

        Xvfb :99 -screen 0 1920x1080x24 &
        XVFB_PID=$!
        trap "kill $XVFB_PID >/dev/null 2>&1 || true" EXIT
        sleep 2

        cd /opt/sensor-simulator
        ./sensor_simulator &
        APP_PID=$!

        # Allow the window to initialize and become visible
        sleep 5

        WINDOW_ID=""
        for attempt in $(seq 1 10); do
            WINDOW_ID=$(xdotool search --onlyvisible --name "Sensor Simulator" | head -n 1 || true)
            if [ -n "$WINDOW_ID" ]; then
                break
            fi
            sleep 1
        done

        if [ -n "$WINDOW_ID" ]; then
            echo "Captured window ID $WINDOW_ID"
            xwd -display :99 -silent -id "$WINDOW_ID" | convert xwd:- "$OUTPUT"
        else
            echo "Warning: Could not locate Sensor Simulator window; capturing full root window."
            xwd -display :99 -root -silent | convert xwd:- "$OUTPUT"
        fi

        kill "$APP_PID" >/dev/null 2>&1 || true
    '

echo "==> Screenshot saved to ${OUTPUT_FILE}"

