# Lesson 9: Custom Models
Demonstrates custom QAbstractTableModel implementation with Task data structure.

## Building
```bash
cd 09-custom-models
docker build -t qt-lesson-09 .
```

## Running
**macOS:**
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-09
```

## Features
- Custom TaskModel subclassing QAbstractTableModel
- Task struct with title, priority, completion status
- Color-coded priorities (High=red, Medium=yellow, Low=green)
- Inline editing for title and priority
- Checkbox for completion status
- Add/remove tasks with proper model notifications
