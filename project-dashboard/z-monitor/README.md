# Z Monitor

A modern, real-time patient monitoring system that reads data from sensors and attached devices, built with Qt 6 and QML.

## What is Z Monitor?

**Z Monitor** is a patient monitoring device application that:
- Reads real-time data from medical sensors (ECG, SpO2, respiration, infusion pumps)
- Displays vital signs and waveforms on an embedded touch-screen interface
- Manages multi-level alarms and notifications for clinical staff
- Securely transmits telemetry data to a central monitoring server
- Stores encrypted patient data locally with archival capabilities
- Provides device configuration including Device ID, Bed ID, and measurement unit preferences (metric/imperial)

The Z Monitor is designed for embedded touch-screen devices (8-inch, 1280x800 resolution) and provides continuous patient monitoring with modern UI/UX, comprehensive alarm management, and enterprise-grade security.

## Build & Run

### Prerequisites
- Docker
- X11 Server (for macOS/Linux GUI)

### Build

```bash
cd z-monitor
docker build -t z-monitor .
```

### Run (macOS)

```bash
# Allow X11 connections
xhost +localhost

# Run container with Qt software rendering
docker run -it --rm \
    -e DISPLAY=host.docker.internal:0 \
    -e QT_QUICK_BACKEND=software \
    z-monitor
```

### Run (Linux)

```bash
docker run -it --rm \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=$DISPLAY \
    z-monitor
```

### Local Build (without Docker)

```bash
cd z-monitor
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./z-monitor
```

## Project Structure

- `src/core`: Backend logic and data services
- `src/ui`: QML integration and controllers
- `resources/qml`: UI definitions and components
- `tests`: Unit tests

## Configuration

The Z Monitor supports the following device configuration options accessible through the Settings View:

- **Device ID**: Unique identifier for the monitoring device, used for telemetry transmission and device identification
- **Bed ID**: Identifier for the bed/room location where the device is deployed, used for patient context and location tracking
- **Measurement Unit**: System preference for displaying measurements in either metric or imperial units

These settings are persisted locally and can be modified by authorized users (Technician role) through the Settings View in the application UI.

## Documentation

See `../doc/` for comprehensive documentation including:
- Architecture overview
- UI/UX guidelines
- Security architecture
- Database design
- Threading model
- Configuration and settings
- And more

