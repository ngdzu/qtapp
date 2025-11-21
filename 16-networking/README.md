# Lesson 16: Networking

## Building
```bash
cd 16-networking
docker build -t qt-lesson-16 .
```

## Running
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 qt-lesson-16
```
