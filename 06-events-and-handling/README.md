# Lesson 6: Events and Event Handling

This lesson demonstrates Qt's event system, including mouse event handling, keyboard event handling, the general `event()` function, and event filters.

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

From the **lesson directory** (`06-events-and-handling`):

```bash
docker build -t qtapp-lesson06:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson06:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson06:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson06-events
```

## Expected Behavior

The application displays a window with:

1. **Instruction label** explaining the controls
2. **Colored widget** (200x200 square) that responds to:
   - **Mouse clicks**: Changes to a random color and displays click coordinates
   - **Arrow keys**: Moves the widget in the pressed direction (10 pixels per press)
   - **Escape key**: Closes the window
   - **Mouse enter/leave**: Logs messages when mouse enters or exits widget area

3. **Console output** showing:
   - Event filter messages for every key press across the entire application
   - Mouse click positions and button information
   - Widget movement coordinates
   - Mouse enter/leave notifications

### Key Features Demonstrated

- **Widget-specific event handlers**: `mousePressEvent()`, `keyPressEvent()`, `paintEvent()`
- **General event handling**: `event()` function for Enter/Leave events
- **Event filters**: Global `eventFilter()` installed on QApplication to log all key presses
- **Event acceptance**: Using `accept()` to stop event propagation
- **Focus policy**: `setFocusPolicy(Qt::StrongFocus)` to enable keyboard input
- **Custom painting**: Drawing colored background and text in `paintEvent()`

## Try It

1. Click on the colored widget multiple times and observe color changes
2. Click at different positions to see coordinates update
3. Use arrow keys to move the widget around the window
4. Watch the console to see event filter logging all key presses
5. Hover mouse over the widget to see Enter/Leave events in console
6. Press Escape to close the application

## Learning Points

- Events flow through Qt in a specific order: event filters → `event()` → specific handlers → base class
- Override specific handlers (`mousePressEvent()`, etc.) for targeted event handling
- Use `event()` for general event handling or custom event types
- Event filters enable global event monitoring without modifying individual widgets
- Focus policy must be set for widgets to receive keyboard events
- Events can be accepted (stop propagation) or ignored (propagate to parent)
- Always call base class implementations for unhandled events

## Troubleshooting

If the window doesn't appear:

1. Verify XQuartz is running
2. Check `xhost + localhost` was executed
3. Ensure DISPLAY is set correctly: `echo $DISPLAY`
4. On macOS, try: `DISPLAY=:0` if `host.docker.internal:0` doesn't work

If you see GL warnings, they're cosmetic and can be ignored. The `QT_LOGGING_RULES` environment variable suppresses them.
