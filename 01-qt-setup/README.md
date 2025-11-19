# Lesson 1: Qt Setup

This lesson demonstrates a minimal Qt Widgets application: a simple window displaying "Hello, Qt!".

## Building and Running

### Option 1: Build and run inside Docker (recommended)

**Step 1: Build the shared Qt base image** (only needed once)

From the root directory of the repository, run:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
```

**Step 2: Build the lesson image** (reuses the shared base)

From the lesson directory:

```bash
cd 01-qt-setup
docker build -t qt-lesson-01 .
```

> **Note:** The lesson image shares ~987 MB of layers with the base image, so actual disk usage for this lesson is only ~352 MB instead of 1.34 GB. Building additional lessons will reuse the same base, keeping total storage minimal.

**Run the application (macOS with XQuartz)**

Install XQuartz if not already installed: `brew install --cask xquartz`. Start XQuartz and enable "Allow connections from network clients" in Preferences > Security.

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 qt-lesson-01
```

**Run the application (Linux)**

IMPORTANT: Run the helper script at least once to grant X11 access. From the root qtapp directory:

```bash
../scripts/xhost-allow-for-compose.sh app allow
```

Then run the container with X11 socket mounted:

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qt-lesson-01
```

Clean up X11 permissions after testing:

```bash
../scripts/xhost-allow-for-compose.sh app revoke
```

### Option 2: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./qt-setup
```

## What You'll See

A small window (300×100 pixels) with the centered text "Hello, Qt!" and the window title "Lesson 1: Qt Setup". Click the close button to exit.

## Requirements

- **Qt Modules:** Qt6::Widgets
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (optional but recommended)
- **X11:** For GUI display on Linux/macOS

## Notes

- The Dockerfile uses a multi-stage build to keep the final image lean (~200 MB runtime vs ~1 GB if including build tools).
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server.
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`.

## Troubleshooting

**"Could not connect to display"**: Ensure X11 server is running and `xhost` permissions are set correctly.

**"Qt6 not found"**: Verify Qt 6 is installed in the container (it's included in the Dockerfile) or on your local system.

**"qt.qpa.plugin: Could not load the Qt platform plugin"**: This usually means Qt GUI libraries are missing. The Dockerfile includes `qt6-base-dev` to prevent this.
