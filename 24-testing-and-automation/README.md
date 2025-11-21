# Lesson 24: Testing and Automation

## Building

```bash
cd 24-testing-and-automation
docker build -t qt-lesson-24 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-24
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-24
```
