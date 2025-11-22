# Lesson 3: Qt Widgets Basics

This lesson demonstrates the fundamentals of Qt Widgets: creating a main window with QPushButton and QLabel, and understanding the event loop.

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

From the **lesson directory** (`03-widgets-basics`):

```bash
docker build -t qtapp-lesson03:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson03:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson03:latest
```

## What to Expect

A window titled "Qt Widgets Basics" will appear with:
- A button labeled "Click Me"
- A label showing "Clicks: 0"

Each time you click the button, the label updates to show the number of clicks (Clicks: 1, Clicks: 2, etc.).

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./widgets-basics
```

## Learning Objectives

After completing this lesson, you should understand:
- How QWidget serves as the base for all UI components
- How to create and configure QPushButton and QLabel widgets
- The role of QApplication and the event loop
- Basic signal-slot connections with lambdas
- Parent-child widget relationships and automatic memory management
