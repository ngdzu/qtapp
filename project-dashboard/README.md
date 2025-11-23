# Medical Device Dashboard

A modern, real-time dashboard for a medical device, built with Qt 6 and QML.

## Overview

This project demonstrates:
- **Modern QML UI:** Dark theme, custom components, and real-time visualizations.
- **C++ Backend:** Data simulation and business logic separated from UI.
- **Architecture:** Model-View-Controller (MVC) pattern with `DashboardController`.
- **Testing:** Mock objects for data services.

## Build & Run

### Prerequisites
- Docker
- X11 Server (for macOS/Linux GUI)

### Build
```bash
docker build -t project-dashboard .
```

### Run (macOS)
```bash
# Allow X11 connections
xhost +localhost

# Run container with Qt software rendering
docker run -it --rm \
    -e DISPLAY=host.docker.internal:0 \
    -e QT_QUICK_BACKEND=software \
    project-dashboard
```

### Run (Linux)
```bash
docker run -it --rm \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=$DISPLAY \
    project-dashboard
```

## Project Structure

- `src/core`: Backend logic and data simulation.
- `src/ui`: QML integration and controllers.
- `resources/qml`: UI definitions.
