# Lesson 21: Animation and Transitions

This lesson demonstrates Qt's animation framework with QPropertyAnimation and animation groups.

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
cd 21-animation-and-transitions
docker build -t qt-lesson-21 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-21
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-21
```

## Expected Behavior

The application displays an animation demo with:
- Animated widget (green button)
- Position animation (smooth movement)
- Size animation (grow/shrink)
- Fade animation (opacity changes)
- Parallel animation (multiple properties at once)
- Sequential animation (step-by-step choreography)
- Easing curve selector
- Reset button to restore initial state

Try different easing curves to see how they affect motion!

## What You'll Learn

- Creating property animations with QPropertyAnimation
- Using easing curves for natural motion
- Parallel animations with QParallelAnimationGroup
- Sequential animations with QSequentialAnimationGroup
- Handling animation completion signals
- Setting key values for complex animations
- Managing animation memory with DeleteWhenStopped
