# Lesson 12: Qt Quick/QML
Introduction to QML declarative UI with property bindings and mouse interaction.

## Prerequisites

This is a GUI application requiring X11. On macOS, install and start XQuartz:

```bash
brew install --cask xquartz
open -a XQuartz
```

In XQuartz preferences (XQuartz → Settings → Security), enable "Allow connections from network clients", then restart XQuartz.

Allow X11 connections:
```bash
xhost + 127.0.0.1
```

## Building
```bash
cd 12-qtquick-qml-intro
docker build -t qt-lesson-12 .
```

## Running

**macOS:**
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e QT_QUICK_BACKEND=software qt-lesson-12
```

**Linux:**
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e QT_QUICK_BACKEND=software qt-lesson-12
```

## Expected Behavior

A window will appear with a blue gradient background containing:
- "Hello Qt Quick!" text in white
- A white box that changes to light green when you hover over it
- The box rotates 45 degrees each time you click it
- A "Click Me" button that logs to the console when clicked
