# Lesson 7: Dialogs and Windows

This lesson demonstrates Qt's standard dialog classes (QFileDialog, QMessageBox, QInputDialog, QColorDialog) and custom dialog creation, including modal vs. modeless behavior.

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

From the **lesson directory** (`07-dialogs-and-windows`):

```bash
docker build -t qtapp-lesson07:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson07:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson07:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson07-dialogs
```

## Expected Behavior

The application displays a main window with instruction text, a read-only text area for logging results, and three rows of buttons:

### Row 1: File Dialogs
- **Open File**: Opens native file browser filtered for text files
  - Shows selected file path in log area
  - Logs "cancelled" if user clicks Cancel
- **Save File**: Opens save dialog with "untitled.txt" as default name
  - Shows selected save path in log area
  - Filters for text files

### Row 2: Message Boxes
- **Info Message**: Shows information icon with explanatory text
- **Warning**: Shows warning icon with alert message
- **Question**: Shows Yes/No question dialog
  - Logs which button user clicked

### Row 3: Input and Custom Dialogs
- **Text Input**: Prompts for name input
  - Shows entered text in log area
- **Number Input**: Prompts for age (0-120, default 25)
  - Shows entered number in log area
- **Color Picker**: Opens color selection dialog
  - Changes main window background color
  - Shows selected color hex code in log
- **Custom Dialog**: Opens custom login form
  - Username and password fields
  - OK button disabled until both fields filled
  - Shows username and masked password in log

### Key Features Demonstrated

- **Modal dialogs**: All dialogs block interaction with main window until closed
- **Dialog return values**: File paths, button clicks, input values, colors
- **Cancellation handling**: All dialogs check for user cancellation
- **Custom dialog**: Subclass of QDialog with validation and QDialogButtonBox
- **Button validation**: OK button enabled/disabled based on input completeness
- **Parent-child relationships**: All dialogs properly parented to avoid memory leaks
- **Platform-native appearance**: File dialogs use system file browser

## Try It

1. Click "Open File" and navigate your filesystem, try canceling
2. Click "Save File" and note the default filename and filter
3. Try each message box type and observe icons and button layouts
4. Use "Text Input" and try entering text, then canceling
5. Use "Number Input" and try the min/max bounds (0-120)
6. Click "Color Picker" and change the background color multiple times
7. Click "Custom Dialog" and try:
   - Clicking OK with empty fields (button should be disabled)
   - Filling only username (OK still disabled)
   - Filling both fields (OK becomes enabled)
   - Clicking Cancel vs. OK

## Learning Points

- Qt provides standard dialogs for common tasks, reducing development time
- Modal dialogs (`exec()`) block until closed; modeless (`show()`) don't block
- Always check dialog return values for cancellation (empty strings, invalid colors, etc.)
- `QDialogButtonBox` provides platform-native button ordering and standard signals
- Custom dialogs subclass `QDialog` and connect accept/reject signals
- Form validation can disable OK buttons until input is complete
- Parent widgets own child dialogs, preventing memory leaks
- Dialog results are returned via `exec()` return value or getter methods
- Different platforms may show dialogs differently (native vs. Qt dialogs)

## Troubleshooting

If the window doesn't appear:

1. Verify XQuartz is running
2. Check `xhost + localhost` was executed
3. Ensure DISPLAY is set correctly: `echo $DISPLAY`
4. On macOS, try: `DISPLAY=:0` if `host.docker.internal:0` doesn't work

If you see GL warnings, they're cosmetic and can be ignored. The `QT_LOGGING_RULES` environment variable suppresses them.

## Notes

File dialogs in the container won't have access to your host filesystem. They'll show the container's filesystem, which is minimal. This is normal Docker behavior and doesn't affect learning dialog usage patterns. In a real application outside Docker, file dialogs would access the full system filesystem.
