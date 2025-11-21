# Lesson 19: Serialization and Settings

This lesson demonstrates JSON serialization with QJsonDocument and persistent settings with QSettings.

## Prerequisites

### macOS X11 Setup

```bash
brew install --cask xquartz
open -a XQuartz
xhost + 127.0.0.1
```

### Linux X11 Setup

```bash
xhost +local:docker
```

## Building

```bash
cd 19-serialization-and-settings
docker build -t qt-lesson-19 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-19
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-19
```

## Expected Behavior

The application displays a form where you can:
- Enter user data (name, age, email)
- Generate JSON representation
- Parse JSON back into the form
- Save settings persistently
- Load saved settings

Settings persist between application runs!

## What You'll Learn

- JSON parsing and generation with QJsonDocument
- Working with QJsonObject and QJsonArray
- Storing application preferences with QSettings
- Reading settings with default values
- Organizing settings into groups
