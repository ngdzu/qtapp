# Lesson 22: OpenGL and 3D

This lesson demonstrates OpenGL integration with Qt using QOpenGLWidget to render a 3D rotating cube.

## Prerequisites

### macOS X11 Setup

```bash
brew install --cask xquartz
open -a XQuartz
xhost + 127.0.0.1
```

### Linux X11 Setup

```bash
xhost +local:docker
```

## Building

```bash
cd 22-opengl-and-3d
docker build -t qt-lesson-22 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-22
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-22
```

## Expected Behavior

The application displays:
- A 3D cube rotating continuously
- Six colored faces (red, green, blue, yellow, cyan, magenta)
- Smooth animation at ~60 FPS
- Proper depth testing (back faces hidden)
- Responsive scaling when window is resized

## What You'll Learn

- Setting up QOpenGLWidget for 3D rendering
- Implementing initializeGL(), paintGL(), and resizeGL()
- Drawing 3D geometry with OpenGL
- Applying transformations (translate, rotate)
- Enabling depth testing for proper 3D rendering
- Creating smooth animations with QTimer
- Managing OpenGL context lifecycle with Qt
