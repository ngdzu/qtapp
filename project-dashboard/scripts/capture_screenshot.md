# Capture Screenshot

Builds the sensor simulator Docker image, runs it headlessly, and captures a screenshot of the application window.

## Prerequisites

- Docker installed and running
- X11 virtual framebuffer support (handled automatically in the container)

## Usage

Run from the `project-dashboard` directory:

```bash
cd project-dashboard
./scripts/capture_screenshot.sh
```

## What It Does

The script automatically:

1. **Builds the Docker image** (if needed or if source changed)
   - Uses `project-dashboard/sensor-simulator/Dockerfile`
   - Builds both builder and runtime stages
   - Caches layers for faster subsequent builds

2. **Launches the simulator** in a virtual X server
   - Starts Xvfb (X Virtual Framebuffer) on display `:99`
   - Runs the sensor simulator application headlessly
   - Waits for the application window to appear

3. **Captures the window**
   - Uses `xdotool` to find the "Sensor Simulator" window
   - Captures only the application window (not the full screen)
   - Converts the capture to PNG format

4. **Saves the screenshot**
   - Output: `project-dashboard/sensor-simulator/media/screenshot.png`
   - Overwrites any existing screenshot file

## Output

- Screenshot saved to: `project-dashboard/sensor-simulator/media/screenshot.png`
- The screenshot shows the current state of the UI after any QML changes

## Notes

- **Must be run from `project-dashboard` directory** - the script resolves paths relative to this location
- First run may take longer as it builds the Docker image
- Subsequent runs are faster due to Docker layer caching
- The script installs screenshot dependencies (`x11-apps`, `imagemagick`, `xdotool`) inside the container if needed
- If the window cannot be found, the script falls back to capturing the full root window

## Troubleshooting

### Docker daemon not running

```
ERROR: Cannot connect to the Docker daemon
```

**Solution:** Start Docker Desktop or your Docker daemon.

### Window not found

```
Warning: Could not locate Sensor Simulator window; capturing full root window.
```

**Solution:** 
- Check that the application launched successfully (look for QML errors in output)
- Increase the sleep time in the script if the app needs more time to start
- Verify the window title matches "Sensor Simulator"

### QML errors in output

If you see QML errors, fix them before the screenshot will be useful. Common issues:
- Missing imports
- Type errors
- Property assignment errors

## Integration

This script is automatically referenced by the `.cursor/rules/qml_guidelines.mdc` rule file, which ensures screenshots are captured after editing QML files.

