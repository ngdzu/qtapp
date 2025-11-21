# Lesson 17: Threading and Concurrency

## Building

```bash
cd 17-threading-and-concurrency
docker build -t qt-lesson-17 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-17
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-17
```
