---
description: when create/modify Docker related files.
alwaysApply: false
---
# Docker Architecture and Guidelines

## Overview

This document defines the Docker architecture, build patterns, and best practices for the Z Monitor project. All Dockerfiles must follow the multi-stage build pattern to minimize image size and ensure consistent, reproducible builds.

**Key Principles:**
- **Multi-Stage Builds** - Separate builder and runtime stages
- **Base Image Consistency** - Use shared base images for efficiency
- **Storage Efficiency** - Minimize image size through layer optimization
- **Reproducibility** - Builds must be deterministic and consistent
- **Cross-Platform** - Support macOS, Linux, and Windows (via Docker)

---

## 1. Base Images

### Standard Base Images

- **`qtapp-qt-dev-env:latest`** - Qt 6 SDK, CMake, build tools (builder stage)
  - Contains: Qt 6 development packages, CMake, compiler, build tools
  - Use for: Building Qt applications
  - Size: ~2-3 GB (shared across all builds)

- **`qtapp-qt-runtime:latest`** - Qt 6 runtime libraries only (runtime stage)
  - Contains: Qt 6 runtime libraries, minimal system dependencies
  - Use for: Running Qt applications
  - Size: ~611 MB (shared across all runtime images)

### Base Image Rules

- **Never introduce alternative base images** - All projects must use the standard base images
- **Tag consistency** - Always use `:latest` tag (or `:local` for local development)
- **Shared base** - All lessons/applications share the same runtime base for efficiency

### Storage Efficiency

- **All lessons share the runtime base** (~611 MB)
- **Each lesson adds ~40 KB** (the executable only)
- **Total for 28 lessons ≈ 612 MB** (not 28 × 611 MB)
- **Never duplicate base images** - Reuse shared bases

---

## 2. Multi-Stage Build Pattern

### Standard Pattern

All Dockerfiles must use the multi-stage build pattern:

```dockerfile
# Builder stage: Compile the application
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build

# Copy source files
COPY CMakeLists.txt main.cpp ./

# Build and install
RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

# Runtime stage: Minimal runtime image
FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson##

# Copy only the executable from builder
COPY --from=builder /opt/lesson##/<executable> .

# Set entrypoint
CMD ["./<executable>"]
```

### Stage Separation

- **Builder stage**: Contains build tools, compilers, development packages
- **Runtime stage**: Contains only runtime libraries and the executable
- **No build tools in runtime**: Keep runtime image minimal

---

## 3. Build Patterns by Application Type

### 3.1. Console Applications

**Pattern:** Simple executable, no GUI dependencies

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson##
COPY --from=builder /opt/lesson##/<executable> .

CMD ["./<executable>"]
```

**Characteristics:**
- No X11 setup needed
- Minimal runtime dependencies
- Fastest build and smallest image

### 3.2. GUI Applications (Qt Widgets)

**Pattern:** Requires X11 for display

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson##

# Install X11 runtime dependencies (if not in base)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libx11-6 \
    libxcb1 \
    && rm -rf /var/lib/apt/lists/* || true

COPY --from=builder /opt/lesson##/<executable> .

CMD ["./<executable>"]
```

**Run Instructions (README.md):**
- **macOS**: `DISPLAY=host.docker.internal:0 docker run -e DISPLAY=host.docker.internal:0 <image>`
- **Linux**: `docker run -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY <image>`
- **Suppress GL noise**: `-e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false"`

### 3.3. QML Applications (Qt Quick)

**Pattern:** Requires Qt Quick runtime and QML files

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY . .

# Install QML-specific dev packages if needed
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6quick6-dev \
    && rm -rf /var/lib/apt/lists/* || true

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson##

# Install QML runtime dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6quick6 \
    libqt6qml6 \
    xvfb \
    x11vnc \
    libgl1-mesa-dri \
    libgl1-mesa-glx \
    libegl1-mesa \
    && rm -rf /var/lib/apt/lists/* || true

# Force software GL for macOS compatibility
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV QT_QUICK_BACKEND=software

# Copy executable and QML resources
COPY --from=builder /opt/lesson##/<executable> .
COPY --from=builder /workspace/qml ./qml

CMD ["./<executable>"]
```

**Characteristics:**
- Requires QML runtime libraries
- May need QML files copied to runtime
- Software GL for cross-platform compatibility

---

## 4. Dockerfile Best Practices

### 4.1. Layer Optimization

- **Combine RUN commands**: Reduce layer count
  ```dockerfile
  # ✅ Good: Combined RUN commands
  RUN apt-get update && \
      DEBIAN_FRONTEND=noninteractive apt-get install -y \
      package1 \
      package2 \
      && rm -rf /var/lib/apt/lists/* || true
  
  # ❌ Bad: Separate RUN commands (more layers)
  RUN apt-get update
  RUN apt-get install -y package1
  RUN apt-get install -y package2
  ```

- **Clean up in same layer**: Remove package lists in the same RUN command
  ```dockerfile
  # ✅ Good: Cleanup in same layer
  RUN apt-get update && \
      apt-get install -y package && \
      rm -rf /var/lib/apt/lists/*
  ```

### 4.2. COPY Optimization

- **Copy only what's needed**: Use `.dockerignore` to exclude unnecessary files
- **Copy in order**: Copy files that change less frequently first (for layer caching)
  ```dockerfile
  # ✅ Good: Dependencies first, source last
  COPY CMakeLists.txt ./
  COPY main.cpp ./
  
  # ❌ Bad: Copy everything at once (breaks cache)
  COPY . .
  ```

### 4.3. Build Arguments

- **Use ARG for build-time variables**:
  ```dockerfile
  ARG BUILD_TYPE=Release
  RUN cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  ```

### 4.4. Environment Variables

- **Use ENV for runtime configuration**:
  ```dockerfile
  ENV QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false"
  ENV LIBGL_ALWAYS_SOFTWARE=1
  ```

### 4.5. Working Directory

- **Set WORKDIR early**: Reduces path repetition
  ```dockerfile
  WORKDIR /build
  # All subsequent commands run in /build
  ```

---

## 5. Application-Specific Patterns

### 5.1. Lessons (Simple Executables)

**Pattern:** Minimal, focused on single concept

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson##
COPY --from=builder /opt/lesson##/<executable> .

CMD ["./<executable>"]
```

**Requirements:**
- Single executable
- Minimal dependencies
- ~40 KB incremental size

### 5.2. Sensor Simulator (QML + WebSocket)

**Pattern:** QML application with additional dependencies

```dockerfile
FROM qtapp-qt-qtquick-dev:local AS builder

WORKDIR /workspace
COPY . .

# Install app-specific dev packages
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6websockets6-dev \
    && rm -rf /var/lib/apt/lists/* || true

RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target install

FROM qtapp-qt-qtquick:local AS runtime

WORKDIR /opt/sensor-simulator

# Install app-specific runtime packages
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6websockets6 \
    libqt6svg6 \
    xvfb \
    x11vnc \
    libgl1-mesa-dri \
    && rm -rf /var/lib/apt/lists/* || true

ENV LIBGL_ALWAYS_SOFTWARE=1

COPY --from=builder /opt/sensor-simulator/sensor_simulator .
COPY --from=builder /workspace/qml ./qml

CMD ["./sensor_simulator"]
```

**Characteristics:**
- Additional Qt modules (WebSockets, SVG)
- QML files copied to runtime
- X11/VNC support for headless operation

### 5.3. Z Monitor (Complex Application)

**Pattern:** Full application with multiple dependencies

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY . .

# Install all development dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6quick6-dev \
    libqt6sql6-dev \
    libsqlcipher-dev \
    && rm -rf /var/lib/apt/lists/* || true

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/z-monitor

# Install runtime dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6quick6 \
    libqt6sql6 \
    libsqlcipher0 \
    && rm -rf /var/lib/apt/lists/* || true

COPY --from=builder /opt/z-monitor/z-monitor .
COPY --from=builder /build/resources ./resources

CMD ["./z-monitor"]
```

---

## 6. Docker Compose Patterns

### 6.1. Simple Service

```yaml
services:
  app:
    image: qtapp-app:local
    build:
      context: ./app
      dockerfile: Dockerfile
    ports:
      - "9002:9002"
    environment:
      - DISPLAY=host.docker.internal:0
      - QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false"
```

### 6.2. Multi-Service Setup

```yaml
services:
  simulator:
    image: qtapp-sensor-simulator:local
    build:
      context: ./sensor-simulator
      dockerfile: Dockerfile
    ports:
      - "9002:9002"
    environment:
      - USE_HOST_X=1
      - DISPLAY=host.docker.internal:0
      - LIBGL_ALWAYS_SOFTWARE=1

  z-monitor:
    image: qtapp-z-monitor:local
    build:
      context: ./z-monitor
      dockerfile: Dockerfile
    depends_on:
      - simulator
    environment:
      - DISPLAY=host.docker.internal:0
```

---

## 7. Platform-Specific Considerations

### 7.1. macOS (XQuartz)

**X11 Setup:**
- Use `DISPLAY=host.docker.internal:0`
- Allow X11 connections in XQuartz preferences
- Use software GL: `LIBGL_ALWAYS_SOFTWARE=1`

**Run Command:**
```bash
DISPLAY=host.docker.internal:0 docker run \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  <image>
```

### 7.2. Linux

**X11 Setup:**
- Mount X11 socket: `-v /tmp/.X11-unix:/tmp/.X11-unix`
- Use host DISPLAY: `-e DISPLAY=$DISPLAY`

**Run Command:**
```bash
docker run \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e DISPLAY=$DISPLAY \
  <image>
```

### 7.3. Windows

**X11 Setup:**
- Use VcXsrv or similar X server
- Configure DISPLAY appropriately
- Use software GL for compatibility

---

## 8. Storage Efficiency Guidelines

### 8.1. Layer Sharing

- **All applications share base images**: Don't create custom base images
- **Runtime base is shared**: All runtime images share `qtapp-qt-runtime:latest`
- **Only executables differ**: Each application adds only its executable

### 8.2. Size Targets

- **Lesson executables**: ~40 KB each
- **Runtime base**: ~611 MB (shared)
- **Total for 28 lessons**: ~612 MB (not 28 × 611 MB)

### 8.3. Optimization Techniques

- **Multi-stage builds**: Separate builder and runtime
- **Minimal runtime**: Only copy executables and required resources
- **Layer caching**: Order COPY commands for optimal caching
- **Cleanup**: Remove package lists and build artifacts

---

## 9. Build Optimization

### 9.1. Build Cache

- **Order matters**: Copy files that change less frequently first
  ```dockerfile
  # ✅ Good: Dependencies first
  COPY CMakeLists.txt ./
  COPY main.cpp ./
  # Changes to main.cpp won't invalidate CMakeLists.txt layer
  
  # ❌ Bad: Everything at once
  COPY . .
  # Any change invalidates entire layer
  ```

### 9.2. Parallel Builds

- **Use build cache**: Leverage Docker's build cache
- **Build arguments**: Use ARG for build-time configuration
- **Multi-stage caching**: Builder stage can be cached separately

### 9.3. Build Time Reduction

- **Minimal dependencies**: Only install what's needed
- **Combined RUN commands**: Reduce layer count
- **Cache package lists**: Structure apt-get commands for caching

---

## 10. Security Best Practices

### 10.1. Base Image Security

- **Use official base images**: Or trusted custom bases
- **Keep bases updated**: Regularly update base images
- **Scan for vulnerabilities**: Use security scanning tools

### 10.2. Runtime Security

- **Minimal runtime**: Only include necessary runtime libraries
- **No build tools in runtime**: Keep runtime image minimal
- **Non-root user**: Consider running as non-root when possible

### 10.3. Secrets Management

- **Never commit secrets**: Use Docker secrets or environment variables
- **Use .dockerignore**: Exclude sensitive files from builds
- **Runtime secrets**: Pass secrets at runtime, not build time

---

## 11. Common Patterns

### 11.1. .dockerignore

Create `.dockerignore` to exclude unnecessary files:

```
.git
.gitignore
*.md
build/
.vscode/
.idea/
*.swp
*.swo
*~
```

### 11.2. Build Scripts

Use build scripts for complex builds:

```bash
#!/bin/bash
docker build -t qtapp-app:local -f Dockerfile .
```

### 11.3. Development vs Production

- **Development**: Use `:local` tags, include debug symbols
- **Production**: Use `:latest` tags, Release builds, optimized

---

## 12. Troubleshooting

### 12.1. Build Failures

- **Check base image**: Ensure base images are available
- **Verify dependencies**: Check that all dependencies are installed
- **Build logs**: Review build logs for specific errors
- **Layer inspection**: Use `docker history` to inspect layers

### 12.2. Runtime Issues

- **X11 connection**: Verify X11 setup for GUI applications
- **Missing libraries**: Check that runtime dependencies are installed
- **Permissions**: Verify file permissions on executables
- **Environment variables**: Check required environment variables

### 12.3. Size Issues

- **Inspect layers**: Use `docker history` to see layer sizes
- **Optimize COPY**: Ensure only necessary files are copied
- **Cleanup**: Verify cleanup commands are working
- **Base image**: Ensure using shared base images

---

## 13. Quick Reference

### Base Images

| Image | Purpose | Size | Contains |
|-------|---------|------|-----------|
| `qtapp-qt-dev-env:latest` | Builder | ~2-3 GB | Qt SDK, CMake, build tools |
| `qtapp-qt-runtime:latest` | Runtime | ~611 MB | Qt runtime libraries |

### Build Pattern

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /build
COPY source files
RUN build commands

FROM qtapp-qt-runtime:latest AS runtime
WORKDIR /opt/app
COPY --from=builder /opt/app/executable .
CMD ["./executable"]
```

### Run Commands

| Platform | Command |
|----------|---------|
| **macOS** | `DISPLAY=host.docker.internal:0 docker run -e DISPLAY=host.docker.internal:0 <image>` |
| **Linux** | `docker run -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY <image>` |

---

## 14. Anti-Patterns to Avoid

### ❌ Don't Use Single-Stage Builds

```dockerfile
# ❌ Bad: Single-stage (includes build tools in runtime)
FROM qtapp-qt-dev-env:latest
WORKDIR /app
COPY . .
RUN cmake .. && cmake --build .
CMD ["./app"]
```

### ❌ Don't Copy Everything

```dockerfile
# ❌ Bad: Copies unnecessary files
COPY . .

# ✅ Good: Copy only what's needed
COPY CMakeLists.txt main.cpp ./
```

### ❌ Don't Create Custom Base Images

```dockerfile
# ❌ Bad: Custom base image
FROM ubuntu:22.04
RUN apt-get install qt6...

# ✅ Good: Use shared base
FROM qtapp-qt-dev-env:latest
```

### ❌ Don't Leave Build Artifacts

```dockerfile
# ❌ Bad: Build artifacts in runtime
COPY --from=builder /build/* .

# ✅ Good: Only executable
COPY --from=builder /opt/app/executable .
```

---

## 15. Related Guidelines

- **Lesson Guidelines**: See `.cursor/rules/lesson_guidelines.mdc` for lesson-specific Dockerfile requirements
- **Common Patterns**: See `.cursor/rules/common_patterns.mdc` for application-specific patterns
- **C++ Guidelines**: See `.cursor/rules/cpp_guidelines.mdc` for code that goes into containers

---

## Enforcement

- **Code Review**: All Dockerfiles must follow multi-stage pattern
- **Base Image Compliance**: Must use standard base images
- **Size Verification**: Runtime images should be minimal
- **Build Verification**: Dockerfiles must build successfully

---

**Remember:** Multi-stage builds are essential for efficiency. Keep builder stages for building, runtime stages minimal. Always use shared base images to maximize storage efficiency. Each application should add only its executable (~40 KB), not duplicate the entire runtime base.
