# Docker Setup for Qt Learning Course

## Shared Base Image Strategy

To minimize disk space and build times, all lessons use a shared Qt base image (`qtapp-qt-dev-env:latest`) that contains Qt 6, CMake, and all common build tools.

### Benefits
- **Storage efficiency**: Base image (~987 MB) is shared across all lessons
- **Unique storage per lesson**: Only ~350 MB per lesson for source code and compiled executable
- **Fast builds**: Lessons build in 1-2 seconds instead of 50+ seconds
- **Consistency**: All lessons use the exact same Qt version and build environment

## Setup Instructions

### 1. Build the Shared Base Image (once)

From the root `qtapp` directory:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
```

This creates the `qtapp-qt-dev-env:latest` image that all lessons will reference.

### 2. Build Individual Lessons

From any lesson directory (e.g., `01-qt-setup`):

```bash
cd 01-qt-setup
docker build -t qt-lesson-01 .
```

The lesson Dockerfile uses `FROM qtapp-qt-dev-env:latest`, so it reuses the base image layers.

### 3. Run a Lesson

**On macOS (with XQuartz):**
```bash
xhost + 127.0.0.1
docker run --rm -e DISPLAY=host.docker.internal:0 qt-lesson-01
```

**On Linux:**
```bash
xhost +local:docker
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-01
xhost -local:docker
```

## Disk Usage

Check actual disk usage:

```bash
docker system df -v | grep qt
```

Example output:
```
qt-lesson-01         1.34GB    987.7MB    351.8MB    (only 351.8MB unique)
qtapp-qt-dev-env     1.34GB    987.7MB    351.8MB    (shared base)
```

The "SHARED SIZE" shows the base image layers that are reused across all images.

## Updating the Base Image

If you need to update Qt version or add packages to the base:

1. Edit the root `Dockerfile` (the `qt-dev-env` stage)
2. Rebuild the base image:
   ```bash
   docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
   ```
3. Rebuild affected lessons:
   ```bash
   cd 01-qt-setup
   docker build -t qt-lesson-01 .
   ```

## Cleanup

Remove unused images and layers:

```bash
# Remove all lesson images
docker images | grep qt-lesson | awk '{print $3}' | xargs docker rmi

# Clean up dangling images and build cache
docker system prune -a
```

## Lesson Dockerfile Template

Each lesson's Dockerfile follows this pattern:

```dockerfile
# Reuse shared Qt base image
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy lesson source
COPY CMakeLists.txt main.cpp ./

# Build
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    mkdir -p /opt/lessonXX && \
    cp executable-name /opt/lessonXX/

# Runtime stage (reuses base for Qt libs)
FROM qtapp-qt-dev-env:latest AS runtime

WORKDIR /opt/lessonXX

COPY --from=builder /opt/lessonXX/executable-name .

CMD ["./executable-name"]
```

## Troubleshooting

**"qtapp-qt-dev-env:latest not found"**: Build the base image first (step 1 above).

**Large disk usage**: Run `docker system df -v` to check actual usage. The "UNIQUE SIZE" column shows real disk usage per image.

**Slow builds**: If rebuilding the base image takes too long, check that you're not modifying the `qt-dev-env` stage unnecessarily. Most lesson changes should only affect the lesson-specific layers.
