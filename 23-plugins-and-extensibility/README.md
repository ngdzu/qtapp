# Lesson 23: Plugins and Extensibility

## Building

```bash
cd 23-plugins-and-extensibility
docker build -t qt-lesson-23 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-23
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-23
```
