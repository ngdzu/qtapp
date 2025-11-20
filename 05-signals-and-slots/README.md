# Lesson 5: Signals and Slots

This lesson demonstrates Qt's signals and slots mechanism: QObject foundation, modern connection syntax, lambda connections, and connection types.

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
cd 05-signals-and-slots
docker build -t qt-lesson-05 .
```

## Running the Application

### macOS

Make sure XQuartz is running, then:

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-05
```

### Linux

```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-05
```

## What to Expect

A window titled "Signals and Slots Demo" will appear with:

- A large label showing "Counter: 0" (updated via signal/slot connection)
- A status label showing messages (updated via lambda connections)
- Two buttons: "Increment" and "Reset"

**Behavior:**
- Click "Increment": Counter increases, multiple slots update the UI
- At counts 5 and 10: Special "milestone" message appears (threshold signal)
- Click "Reset": Counter returns to 0, all connected slots are notified
- Watch the terminal for debug output showing slot execution order

The application demonstrates:
- One signal connected to multiple slots
- Lambda functions as slot handlers
- Custom signals with parameters
- Modern connection syntax

## Building Locally (without Docker)

If you have Qt 6 installed on your system:

```bash
mkdir build
cd build
cmake ..
cmake --build .
./signals-and-slots
```

## Learning Objectives

After completing this lesson, you should understand:
- How signals and slots enable loose coupling between objects
- The role of QObject and Q_OBJECT macro in Qt's meta-object system
- Modern connection syntax with function pointers
- Using lambda functions as inline slot handlers
- How one signal can trigger multiple slots
- The difference between direct and queued connections
- How to create custom signals and emit them with parameters
