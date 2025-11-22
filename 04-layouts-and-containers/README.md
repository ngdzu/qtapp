# Lesson 4: Layouts and Containers

This lesson demonstrates Qt's layout management system: QHBoxLayout, QVBoxLayout, and QGridLayout for creating responsive user interfaces.

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

From the **lesson directory** (`04-layouts-and-containers`):

```bash
docker build -t qtapp-lesson04:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson04:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson04:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./layouts-and-containers
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
