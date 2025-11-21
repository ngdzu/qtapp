# Lesson 18: SQL and Models

This lesson demonstrates Qt's SQL module with a complete employee database manager featuring CRUD operations, table views, and custom SQL queries.

## Prerequisites

Before running this lesson, ensure you have X11 forwarding set up:

### macOS
Install XQuartz from https://www.xquartz.org/, launch it, and in Preferences → Security, enable "Allow connections from network clients". Then restart XQuartz.

### Linux
X11 is typically available by default. You may need to run:
```bash
xhost +local:docker
```

## Building

First, ensure the shared base images are built:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then build this lesson:

```bash
docker build -t qtapp-lesson18:latest 18-sql-and-models
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson18:latest
```

### Linux
```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qtapp-lesson18:latest
```

## What This Demo Shows

The application creates an in-memory SQLite database with an `employees` table and demonstrates:

1. **Database Connection**: Using `QSqlDatabase` to create and connect to SQLite
2. **Table Creation**: Executing raw SQL via `QSqlQuery` to create schemas
3. **Data Population**: Inserting sample employee records
4. **QSqlTableModel**: Automatic synchronization between database and view
5. **QTableView**: Displaying database tables with editable cells
6. **CRUD Operations**:
   - **Create**: Add new employees via input fields
   - **Read**: View all employees in the table (double-click cells to edit)
   - **Update**: Edit cells directly, click "Save Changes" to commit
   - **Delete**: Select rows and delete them
7. **Custom Queries**: "Show High Earners" button demonstrates parameterized SQL queries
8. **Transaction Control**: Save/Revert buttons for commit/rollback functionality

## Features Demonstrated

- **QSqlDatabase**: SQLite in-memory database connection
- **QSqlTableModel**: Model-view architecture for database tables
- **QSqlQuery**: Direct SQL query execution with parameter binding
- **QTableView**: Rich table display with sorting, selection, and editing
- **Manual submit strategy**: Changes accumulate until explicitly saved
- **Error handling**: Database and query error reporting
- **UI integration**: Full CRUD interface with status feedback

## Expected Behavior

When you run the application:

1. Window opens showing "Employee Database Manager" title
2. Table displays 5 pre-populated employees
3. You can double-click any cell to edit it
4. Input fields at bottom let you add new employees
5. Buttons provide database operations:
   - **Add Employee**: Insert new record (auto-generates ID)
   - **Delete Selected**: Remove selected row
   - **Save Changes**: Commit all edits to database
   - **Revert**: Discard uncommitted changes
   - **Show High Earners**: Query employees with salary > $70,000
6. Status label shows feedback for each operation

The demo proves Qt's SQL module provides seamless integration between databases and Qt's model-view architecture, making database-driven UIs straightforward to build.
