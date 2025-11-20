# Lesson 6: Events and Event Handling

This lesson demonstrates Qt's event system, including mouse event handling, keyboard event handling, the general `event()` function, and event filters.

## Prerequisites

**For GUI Applications on macOS:** You need to configure X11 forwarding to display the Qt application window from the Docker container.

1. Install XQuartz:
```bash
brew install --cask xquartz
```

2. Start XQuartz and enable network connections:
   - Open XQuartz
   - Go to Preferences → Security
   - Check "Allow connections from network clients"
   - Restart XQuartz

3. Allow connections from localhost:
```bash
xhost + localhost
```

## Building

First, ensure the shared base image is built:

```bash
cd /Users/dustinwind/Development/Qt/qtapp
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then build the lesson image:

```bash
cd 06-events-and-handling
docker build -t qt-lesson-06 .
```

## Running

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-06
```

**On Linux:**

```bash
docker run --rm -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-06
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
