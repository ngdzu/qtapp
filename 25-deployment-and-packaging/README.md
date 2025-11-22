# Lesson 25: Deployment and Packaging

This lesson demonstrates Qt application deployment information, showing executable details, platform information, library paths, and platform-specific deployment guidance.

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

From the **lesson directory** (`25-deployment-and-packaging`):

```bash
docker build -t qtapp-lesson25:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson25:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson25:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson25
```

## What You'll See

A comprehensive deployment information application with tabbed interface:

**Executable Info Tab:**
- Application executable path and directory
- File size and executable status
- Qt runtime version vs compiled version
- Build type (Debug/Release)
- Qt modules used in the application

**Platform Tab:**
- Operating system and version
- Kernel type and version
- CPU architecture and build ABI
- Platform-specific requirements (DLLs, frameworks, plugins)
- Compiler information (MSVC, GCC, or Clang version)

**Library Paths Tab:**
- Qt library search paths
- Qt installation directories (prefix, libraries, plugins, binaries)
- Important plugin directories (platforms, imageformats, styles, sqldrivers)
- Explanation of required plugins for deployment

**Deployment Guide Tab:**
- Platform-specific deployment instructions
- Required files and directory structure
- Deployment tool usage (windeployqt/macdeployqt/linuxdeployqt)
- Code signing requirements and procedures
- Static vs dynamic linking comparison
- Best practices checklist

The "Refresh Information" button updates all tabs with current system state!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS

## Learning Objectives

- How to identify deployment dependencies for Qt applications
- Platform-specific deployment requirements (Windows, macOS, Linux)
- Using Qt's deployment tools effectively (windeployqt/macdeployqt/linuxdeployqt)
- The difference between static and dynamic linking
- Code signing requirements for production applications
- Best practices for testing deployments on clean systems
- How to structure application bundles and installers
- Understanding Qt plugin architecture for deployment
- Handling platform-specific DLLs and frameworks

## Notes

- The Dockerfile uses a multi-stage build: lessons use the `qt-runtime-nano` base (~242 MB) which contains only essential Qt libraries needed to run applications
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- This lesson demonstrates deployment concepts applicable to real-world distribution
- Qt provides platform-specific deployment tools to simplify the process
- Always test deployments on clean systems without Qt development tools installed
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration
