# Lesson 13: Qt Quick Controls

This lesson demonstrates Qt Quick Controls 2, the modern UI toolkit for building fluid, touch-friendly interfaces with QML.

## Prerequisites

### macOS X11 Setup

Install XQuartz and configure it to allow network connections:

```bash
brew install --cask xquartz
open -a XQuartz
```

In XQuartz preferences (Cmd+,):
- Go to Security tab
- Enable "Allow connections from network clients"
- Restart XQuartz

Allow localhost connections:

```bash
xhost + 127.0.0.1
```

### Linux X11 Setup

```bash
xhost +local:docker
```

## Building

First, ensure the base images are built:

```bash
cd /Users/dustinwind/Development/Qt/qtapp
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then build this lesson:

```bash
cd 13-qtquick-controls
docker build -t qt-lesson-13 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  -e QT_QUICK_BACKEND=software \
  qt-lesson-13
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  -e QT_QUICK_BACKEND=software \
  qt-lesson-13
```

## Expected Behavior

The application displays a form with:
- A TextField for entering your name (letters only)
- A ComboBox for selecting a favorite color
- A Submit button
- A result area that displays your submission

Try entering your name and selecting a color, then click Submit to see the result.

## What You'll Learn

- Using Qt Quick Controls 2 components (Button, TextField, ComboBox)
- Input validation with RegularExpressionValidator
- Layouts with ColumnLayout
- Signal handlers (onClicked)
- Property bindings for dynamic UI updates
