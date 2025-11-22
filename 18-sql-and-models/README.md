# Lesson 18: SQL and Models

This lesson demonstrates Qt's SQL module with a complete employee database manager featuring CRUD operations, table views, and custom SQL queries.

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
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

> **Note:** This lesson uses the standard `qt-runtime` base (not nano) because it requires Qt SQL libraries and SQLite support. The dev environment is ~1.33 GB (used only for building) and the runtime is ~612 MB with SQL support.

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`18-sql-and-models`):

```bash
docker build -t qtapp-lesson18:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson18:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson18:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson18
```

## What You'll See

An Employee Database Manager application demonstrating:

1. **Database Connection**: Using `QSqlDatabase` to create and connect to SQLite
2. **Table Creation**: Executing raw SQL via `QSqlQuery` to create schemas
3. **Data Population**: Table displays 5 pre-populated employees
4. **QSqlTableModel**: Automatic synchronization between database and view
5. **QTableView**: Displaying database tables with editable cells (double-click to edit)
6. **CRUD Operations**:
   - **Create**: Add new employees via input fields
   - **Read**: View all employees in the table
   - **Update**: Edit cells directly, click "Save Changes" to commit
   - **Delete**: Select rows and delete them
7. **Custom Queries**: "Show High Earners" button demonstrates parameterized SQL queries
8. **Transaction Control**: Save/Revert buttons for commit/rollback functionality

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Sql
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS
- **Database:** SQLite (in-memory, included with Qt)

## Learning Objectives

- Using `QSqlDatabase` for SQLite connections
- `QSqlTableModel` for model-view database architecture
- `QSqlQuery` for direct SQL execution with parameter binding
- `QTableView` for rich table display with editing
- Manual submit strategy for transaction control
- Database error handling and reporting
- Full CRUD interface implementation

## Notes

- The Dockerfile uses a multi-stage build with the standard `qt-runtime` base which includes Qt SQL libraries
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- This lesson creates an in-memory SQLite database, so data is not persisted between runs
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration
