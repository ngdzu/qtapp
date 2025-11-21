# Lesson 14: Graphics View Framework

This lesson demonstrates the Graphics View framework for managing and displaying 2D graphical content.

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
cd 14-graphicsview
docker build -t qt-lesson-14 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-14
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-14
```

## Expected Behavior

The application displays a graphics scene with movable shapes:
- Blue rectangle
- Red circle
- Green triangle

All shapes can be selected and dragged.

## What You'll Learn

- QGraphicsScene and QGraphicsView architecture
- Creating and manipulating graphics items
- Making items interactive (movable, selectable)
- Using pens and brushes for styling
