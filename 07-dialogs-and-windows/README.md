# Lesson 7: Dialogs and Windows

This lesson demonstrates Qt's standard dialog classes (QFileDialog, QMessageBox, QInputDialog, QColorDialog) and custom dialog creation, including modal vs. modeless behavior.

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
cd 07-dialogs-and-windows
docker build -t qt-lesson-07 .
```

## Running

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-07
```

**On Linux:**

```bash
docker run --rm -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-07
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
