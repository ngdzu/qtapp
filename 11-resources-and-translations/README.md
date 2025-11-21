# Lesson 11: Resources
Qt Resource System with embedded images and stylesheets.

## Building
```bash
cd 11-resources-and-translations
docker build -t qt-lesson-11 .
```

## Running
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-11
```
