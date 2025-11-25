# Project Dashboard

This repository contains the Z Monitor project - a comprehensive patient monitoring system with supporting tools and simulators.

## Project Structure

- **`z-monitor/`** - The Z Monitor application (main medical device application)
  - Patient monitoring system that reads sensor data and displays vital signs
  - See `z-monitor/README.md` for build and run instructions

- **`sensor-simulator/`** - WebSocket-based sensor simulator
  - Provides simulated sensor data for development and testing
  - See `sensor-simulator/README.md` for details

- **`doc/`** - Comprehensive project documentation
  - Architecture, design, security, and implementation guides

- **`scripts/`** - Utility scripts
  - Screenshot capture, Mermaid diagram generation, etc.

## What is Z Monitor?

**Z Monitor** is a patient monitoring device application that:
- Reads real-time data from medical sensors (ECG, SpO2, respiration, infusion pumps)
- Displays vital signs and waveforms on an embedded touch-screen interface
- Manages multi-level alarms and notifications for clinical staff
- Securely transmits telemetry data to a central monitoring server
- Stores encrypted patient data locally with archival capabilities

The Z Monitor is designed for embedded touch-screen devices (8-inch, 1280x800 resolution) and provides continuous patient monitoring with modern UI/UX, comprehensive alarm management, and enterprise-grade security.

## Overview

This project demonstrates:
- **Modern QML UI:** Dark theme, custom components, and real-time visualizations.
- **C++ Backend:** Data simulation and business logic separated from UI.
- **Architecture:** Model-View-Controller (MVC) pattern with `DashboardController`.
- **Testing:** Mock objects for data services.

## Quick Start

### Building Z Monitor

See `z-monitor/README.md` for detailed build and run instructions.

### Building Sensor Simulator

See `sensor-simulator/README.md` for detailed build and run instructions.

### Docker Compose (simulator)

You can start the simulator and related services using Docker Compose. From the `project-dashboard` directory run:

```bash
docker compose -f docker-compose.simulator.yml up --build
```

This will build the image(s) and start the simulator service defined in `docker-compose.simulator.yml`. The simulator opens a WebSocket at `ws://localhost:9002` by default (verify the compose file if you have custom port mappings).

If you prefer to run the container for GUI inspection using an in-container X server and VNC (useful when host X11 fails), run the preview command shown below — it starts `Xvfb` and `x11vnc` inside the container and maps VNC to host port `15900`:

```bash
# Preview run (Xvfb + x11vnc); connect with a VNC client to localhost:15900
docker run -d --rm -p 9003:9002 -p 15900:5900 --name qt-sim-preview qtapp-sensor-simulator:local \
    sh -c "Xvfb :99 -screen 0 1280x800x24 & x11vnc -display :99 -nopw -forever -shared -rfbport 5900 & DISPLAY=:99 ./sensor_simulator"
```

macOS note: running the container with `DISPLAY=host.docker.internal:0` requires a working X server on the host (XQuartz). If the GUI fails to initialize due to OpenGL/RHI errors, try one of the following:

- Enable indirect GLX in XQuartz and restart it:
    ```bash
    defaults write org.macosforge.xquartz.X11 enable_iglx -bool true
    # then restart XQuartz
    ```
- Allow local X11 connections from the host: `xhost +localhost` (run in a terminal where XQuartz is running).
- If host X11 still fails, use the preview VNC approach above.


## Documentation

Comprehensive documentation is available in the `doc/` directory:
- Architecture and design documents
- Security guidelines
- Database schema
- Threading model
- API specifications (auto-generated from code)
- API documentation generation guide
- And more

### API Documentation

API documentation is automatically generated from source code comments using Doxygen. See `doc/26_API_DOCUMENTATION.md` for:
- Doxygen configuration and setup
- Comment style guidelines
- Documentation generation workflow
- Integration with build system

**Generate API documentation locally:**
```bash
cd project-dashboard
cmake --build build --target docs
# Documentation available in docs/api/html/
```

**Automatic generation:**
- **GitHub Actions:** Documentation is automatically generated nightly and on code changes
- **Pre-commit hook:** Optional lightweight check for Doxygen comments (warning only, doesn't block commits)
- **Workflow:** See `.github/workflows/doxygen-docs.yml` for CI/CD integration

## Testing

- Read the full testing workflow in `doc/18_TESTING_WORKFLOW.md`.
- Run tests via the unified script:
  ```bash
  ./scripts/run_tests.sh unit         # Unit tests
  ./scripts/run_tests.sh integration  # Integration tests
  ./scripts/run_tests.sh e2e          # End-to-end tests
  ./scripts/run_tests.sh coverage     # Coverage report
  ./scripts/run_tests.sh all          # Lint + unit + integration + coverage
  ```
- Coverage report output: `build_coverage/coverage/index.html`.
