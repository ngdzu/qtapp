# Lesson 5: Signals and Slots

This lesson demonstrates Qt's signals and slots mechanism: QObject foundation, modern connection syntax, lambda connections, and connection types.

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

From the **lesson directory** (`05-signals-and-slots`):

```bash
docker build -t qtapp-lesson05:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson05:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson05:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./signals-and-slots
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
