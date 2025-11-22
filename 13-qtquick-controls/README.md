# Lesson 13: Qt Quick Controls

This lesson demonstrates Qt Quick Controls 2, the modern UI toolkit for building fluid, touch-friendly interfaces with QML.

## Building and Running

### One-Time Setup

These steps only need to be done once per machine.

#### 1. Install X11 Server

**For macOS users:**
- Install XQuartz: `brew install --cask xquartz`
- Start XQuartz and enable "Allow connections from network clients" in Preferences > Security

**For Linux users:**
- X11 should be available by default

#### 2. Build the shared Qt base images

From the **root directory** of the repository:

```bash
docker build --target qt-qtquick-dev -t qtapp-qt-qtquick-dev:latest .
docker build --target qt-qtquick -t qtapp-qt-qtquick:latest .
```

> **Note:** The Qt Quick dev environment is used for building QML applications, and the Qt Quick runtime includes QML libraries. This lesson uses the Qt Quick base images which contain additional libraries needed for QML applications.

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`13-qtquick-controls`):

```bash
docker build -t qtapp-lesson13:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e QT_QUICK_BACKEND=software qtapp-lesson13:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -e QT_QUICK_BACKEND=software \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson13:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson13
```

## What You'll See

A Qt Quick Controls application displaying a form with:
- A TextField for entering your name (letters only, with input validation)
- A ComboBox for selecting a favorite color
- A Submit button
- A result area that displays your submission

Try entering your name and selecting a color, then click Submit to see the result.

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration using the software renderer (`QT_QUICK_BACKEND=software`).

## Requirements

- **Qt Modules:** Qt6::Quick, Qt6::Qml, Qt6::QuickControls2
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS

## Learning Objectives

- Using Qt Quick Controls 2 components (Button, TextField, ComboBox)
- Input validation with RegularExpressionValidator
- Layouts with ColumnLayout
- Signal handlers (onClicked)
- Property bindings for dynamic UI updates

## Notes

- The Dockerfile uses a multi-stage build with the `qt-qtquick` runtime base which includes Qt Quick/QML libraries
- The Qt Quick dev environment (`qt-qtquick-dev`) is only needed for building
- Qt Quick applications use the software renderer in containers via `QT_QUICK_BACKEND=software`
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine with software rendering
