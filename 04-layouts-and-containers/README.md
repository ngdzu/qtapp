# Lesson 4: Layouts and Containers

This lesson demonstrates Qt's layout management system: QHBoxLayout, QVBoxLayout, and QGridLayout for creating responsive user interfaces.

## Prerequisites

This lesson requires X11 for GUI display. Make sure you have:
- **macOS**: XQuartz installed and running
- **Linux**: X server running (usually default)

## Building the Lesson

From the root directory of the repository, ensure the shared Qt base images are built:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then navigate to the lesson directory and build the lesson image:

```bash
cd 04-layouts-and-containers
docker build -t qt-lesson-04 .
```

## Running the Application

### macOS

Make sure XQuartz is running, then:

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-04
```

### Linux

```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-04
```

## What to Expect

A window titled "Qt Layouts Demo" will appear with three sections:

1. **Horizontal Layout**: Three buttons ("Left", "Center", "Right") arranged horizontally
2. **Vertical Layout**: A form with "Name" and "Email" labels, text input fields, and a "Submit" button stacked vertically
3. **Grid Layout**: A 3x3 grid of numbered buttons (1-9) plus a "0" button spanning the bottom row (like a calculator keypad)

Try resizing the window to see how layouts automatically adjust widget positions and sizes.

## Building Locally (without Docker)

If you have Qt 6 installed on your system:

```bash
mkdir build
cd build
cmake ..
cmake --build .
./layouts-and-containers
```

## Learning Objectives

After completing this lesson, you should understand:
- How QHBoxLayout, QVBoxLayout, and QGridLayout organize widgets
- The difference between manual positioning and layout-based UI design
- How layouts handle window resizing automatically
- How to use QGroupBox to visually group related widgets
- Basic grid layout usage with row/column positioning and spanning
