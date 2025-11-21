# Lesson 20: Styles, Themes, and Palette

This lesson demonstrates Qt's styling system including QPalette, Qt Style Sheets (QSS), and QStyle.

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
cd 20-styles-themes-and-palette
docker build -t qt-lesson-20 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-20
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-20
```

## Expected Behavior

The application displays a theme switcher with:
- Light Theme button (bright, clean look)
- Dark Theme button (modern dark mode)
- Custom QSS button (CSS-styled interface)
- Sample widgets showing the current theme
- Real-time theme switching

Click the theme buttons to see instant visual changes!

## What You'll Learn

- Using QPalette to create light and dark themes
- Applying Qt Style Sheets (QSS) for custom styling
- Working with QStyle and the Fusion style
- Styling pseudo-states (hover, pressed, focus)
- Creating cohesive application themes
- Dynamic theme switching at runtime
