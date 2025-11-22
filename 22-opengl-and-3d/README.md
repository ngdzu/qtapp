# Lesson 22: OpenGL and 3D

This lesson demonstrates OpenGL integration with Qt using QOpenGLWidget to render a 3D rotating cube.

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

From the **lesson directory** (`22-opengl-and-3d`):

```bash
docker build -t qtapp-lesson22:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson22:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson22:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson22
```

## What You'll See

A 3D OpenGL rendering demonstration featuring:
- A colorful 3D cube rotating continuously in 3D space
- Six colored faces: red, green, blue, yellow, cyan, and magenta
- Smooth animation running at approximately 60 FPS
- Proper depth testing ensuring back faces are hidden correctly
- Responsive scaling when the window is resized

The cube demonstrates fundamental 3D graphics concepts in action!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast" or "No matching fbConfigs or visuals found"). These can be safely ignored - the application runs using software rendering which is perfectly adequate for this demonstration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core, Qt6::OpenGL, Qt6::OpenGLWidgets
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS
- **OpenGL:** Runtime libraries (installed automatically in container)

## Learning Objectives

- Setting up QOpenGLWidget for 3D rendering in Qt applications
- Implementing the OpenGL lifecycle methods: initializeGL(), paintGL(), and resizeGL()
- Drawing 3D geometry with vertices and triangles using OpenGL
- Applying 3D transformations (translation and rotation)
- Enabling depth testing for proper 3D rendering and occlusion
- Creating smooth animations with QTimer for frame updates
- Managing OpenGL context lifecycle within the Qt framework
- Understanding software vs hardware OpenGL rendering

## Notes

- The Dockerfile uses a multi-stage build: this lesson uses the `qt-runtime-nano` base (~242 MB) plus OpenGL libraries (~280 MB total) which contains Qt libraries needed for 3D rendering
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- This lesson requires Qt6::OpenGL and Qt6::OpenGLWidgets modules which are installed at runtime
- Software rendering (Mesa llvmpipe) is used in the container - no GPU acceleration needed
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- GL warnings about missing hardware drivers are expected and harmless in containerized environments
