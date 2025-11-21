# Lesson 8: Model/View Architecture

This lesson demonstrates Qt's Model/View architecture using QStringListModel with QListView and QStandardItemModel with QTableView.

## Prerequisites

**For GUI Applications on macOS:** Configure X11 forwarding.

1. Install XQuartz: `brew install --cask xquartz`
2. Enable network connections in XQuartz Preferences → Security
3. Allow connections: `xhost + localhost`

## Building

Build the shared base image:
```bash
cd /Users/dustinwind/Development/Qt/qtapp
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Build lesson image:
```bash
cd 08-model-view-architecture
docker build -t qt-lesson-08 .
```

## Running

**macOS:**
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-08
```

**Linux:**
```bash
docker run --rm -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-08
```

## Expected Behavior

Window with two sections demonstrating Model/View separation.

**Left: Task List**
- QListView displaying tasks from QStringListModel
- Add Task: Opens input dialog for new task
- Remove Task: Deletes selected task
- Double-click task to edit inline
- Selection label shows selected task

**Right: Contact Table**
- QTableView displaying contacts from QStandardItemModel
- Three columns: Name, Email, Phone
- Add Contact: Opens dialogs for name, email, phone
- Remove Contact: Deletes selected row
- Click headers to sort columns
- Alternating row colors
- Selection label shows full contact details

## Key Features

- Model/View separation: data independent of presentation
- QStringListModel: lightweight string list management
- QStandardItemModel: flexible multi-column data
- Inline editing: double-click list items
- Selection tracking: displays selected item details
- Sortable columns: click table headers
- Visual polish: alternating row colors, stretched columns

## Try It

1. Add several tasks and observe QStringListModel updating
2. Double-click a task to edit it inline
3. Remove tasks and see model update
4. Add contacts with different information
5. Sort table by clicking column headers
6. Select different rows and observe selection labels
7. Try selecting and removing items

## Troubleshooting

If window doesn't appear, verify XQuartz is running and `xhost + localhost` was executed.
GL warnings are cosmetic and can be ignored.
