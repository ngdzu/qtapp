# Lesson 20: Styles, Themes, and Palette

This lesson demonstrates Qt's styling system including QPalette, Qt Style Sheets (QSS), and QStyle.

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
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime-nano -t qtapp-qt-runtime-nano:latest .
```

> **Note:** The dev environment is ~1.33 GB (used only for building) and the runtime is ~242 MB. All lessons share these base images, so each individual lesson only adds ~16 KB (just the executable). This keeps total storage minimal even with 28 lessons!

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`20-styles-themes-and-palette`):

```bash
docker build -t qtapp-lesson20:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson20:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson20:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson20
```

## What You'll See

A theme switcher application where you can:
- Click "Light Theme" button for a bright, clean look
- Click "Dark Theme" button for modern dark mode
- Click "Custom QSS" button for CSS-styled interface
- See sample widgets demonstrating the current theme
- Watch real-time theme switching with instant visual changes

Each theme completely transforms the application's appearance!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS

## Learning Objectives

- Using QPalette to create light and dark themes
- Applying Qt Style Sheets (QSS) for custom styling
- Working with QStyle and the Fusion style
- Styling pseudo-states (hover, pressed, focus)
- Creating cohesive application themes
- Dynamic theme switching at runtime
- CSS-like styling with Qt's QSS syntax

## Notes

- The Dockerfile uses a multi-stage build: lessons use the `qt-runtime-nano` base (~242 MB) which contains only essential Qt libraries needed to run applications
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- Qt Style Sheets are similar to CSS but with Qt-specific properties
- The Fusion style provides a consistent cross-platform look
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration
