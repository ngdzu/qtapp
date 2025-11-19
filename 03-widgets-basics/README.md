# Lesson 3: Qt Widgets Basics

This lesson demonstrates the fundamentals of Qt Widgets: creating a main window with QPushButton and QLabel, and understanding the event loop.

## Prerequisites

This lesson requires X11 for GUI display. Make sure you have:
- **macOS**: XQuartz installed and running
- **Linux**: X server running (usually default)

Before running the container, enable X11 access by running the helper script from the root directory of the repository:

```bash
../scripts/xhost-allow-for-compose.sh app allow
```

This grants the container permission to display GUI windows on your screen.

## Building the Lesson

From the root directory of the repository, ensure the shared Qt base image is built:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
```

Then navigate to the lesson directory and build the lesson image:

```bash
cd 03-widgets-basics
docker build -t qt-lesson-03 .
```

## Running the Application

### macOS

Make sure XQuartz is running, then:

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-03
```

Note: You may see some GL library warnings - these are cosmetic and don't affect functionality.

### Linux

```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-03
```

## What to Expect

A window titled "Qt Widgets Basics" will appear with:
- A button labeled "Click Me"
- A label showing "Clicks: 0"

Each time you click the button, the label updates to show the number of clicks (Clicks: 1, Clicks: 2, etc.).

## Building Locally (without Docker)

If you have Qt 6 installed on your system:

```bash
mkdir build
cd build
cmake ..
cmake --build .
./widgets-basics
```

## Cleanup

After you're done, revoke X11 access:

```bash
../scripts/xhost-allow-for-compose.sh app revoke
```

## Learning Objectives

After completing this lesson, you should understand:
- How QWidget serves as the base for all UI components
- How to create and configure QPushButton and QLabel widgets
- The role of QApplication and the event loop
- Basic signal-slot connections with lambdas
- Parent-child widget relationships and automatic memory management
