# Lesson 10: Item Delegates
Custom item delegates for Model/View with progress bars and combo box editors.

## Building
```bash
cd 10-item-delegates
docker build -t qt-lesson-10 .
```

## Running
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-10
```

## Features
- ProgressDelegate: Visual progress bars with custom painting
- PriorityDelegate: ComboBox editor for priority selection
- Custom size hints for row height
- Demonstrates createEditor(), setEditorData(), setModelData()
