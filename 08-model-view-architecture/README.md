# Lesson 8: Model/View Architecture

This lesson demonstrates Qt's Model/View architecture using QStringListModel with QListView and QStandardItemModel with QTableView.

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

From the **lesson directory** (`08-model-view-architecture`):

```bash
docker build -t qtapp-lesson08:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson08:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson08:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson08-modelview
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
