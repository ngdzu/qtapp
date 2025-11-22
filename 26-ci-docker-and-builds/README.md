# Lesson 26: CI, Docker, and Builds

This lesson demonstrates build environment detection, Docker container identification, and CI/CD platform recognition.

## Building and Running

### One-Time Setup

These steps only need to be done once per machine.

#### 1. Install X11 Server

**For macOS users:**
- Install XQuartz: `brew install --cask xquartz`
- Start XQuartz and enable "Allow connections from network clients" in Preferences > Security

**For Linux users:**
- X11 should be available by default

#### 2. Build the shared Qt base images

From the **root directory** of the repository:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime-nano -t qtapp-qt-runtime-nano:latest .
```

> **Note:** The dev environment is ~1.33 GB (used only for building) and the runtime is ~242 MB. All lessons share these base images, so each individual lesson only adds ~16 KB (just the executable). This keeps total storage minimal even with 28 lessons!

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`26-ci-docker-and-builds`):

```bash
docker build -t qtapp-lesson26:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e DOCKER_CONTAINER=1 qtapp-lesson26:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -e DOCKER_CONTAINER=1 \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson26:latest
```

> **Note:** The `-e DOCKER_CONTAINER=1` flag demonstrates environment variable detection in the CI/CD tab.

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson26
```

## What You'll See

A comprehensive build and environment detection application with tabbed interface:

**Build Configuration Tab:**
- Qt compiled version vs runtime version
- Build type (Debug or Release)
- Optimization level and debug symbols status
- Compiler information (GCC, Clang, or MSVC with version)
- Build ABI and CPU architecture
- Build timestamp from compilation
- Qt modules used in the application

**Docker/Container Tab:**
- Container detection status with multiple evidence sources
- Checks for `/.dockerenv` file
- Checks for `DOCKER_CONTAINER` environment variable
- Examines `/proc/1/cgroup` for container signatures
- Explains multi-stage Docker build benefits
- Size comparison (build stage vs runtime stage)

**CI/CD Environment Tab:**
- Detects CI/CD platform (GitHub Actions, GitLab CI, Jenkins, or none)
- Displays platform-specific environment variables:
  - GitHub Actions: repository, workflow, run ID, actor, ref
  - GitLab CI: project name, pipeline ID, job name, commit SHA, branch
  - Jenkins: job name, build number, Jenkins URL
- Lists all detected CI environment variables
- Shows typical CI pipeline stages (Build → Test → Package → Deploy)

**System Info Tab:**
- Operating system and kernel information
- Machine hostname
- Qt installation paths (prefix, libraries, plugins)
- Library search paths
- Build tool versions (CMake, Make) if accessible

The "Refresh Information" button updates all tabs with current state!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS

## Learning Objectives

- How to detect build configuration (Debug/Release) at runtime
- Multiple methods for detecting Docker container execution
- How to identify CI/CD platforms via environment variables
- The benefits of multi-stage Docker builds for Qt applications
- How to report build environment information for debugging
- Best practices for CI/CD pipeline design
- Environment variable conventions across different CI systems
- Container detection techniques for adaptive behavior

## CI/CD Integration Examples

This application demonstrates patterns useful for:
- **Build verification**: Confirming Debug/Release mode in CI
- **Environment debugging**: Identifying why builds fail in CI but work locally
- **Container detection**: Adjusting behavior when running in Docker
- **CI platform detection**: Enabling platform-specific features
- **Artifact tagging**: Using build info for version strings and metadata

## Notes

- The Dockerfile uses a multi-stage build: lessons use the `qt-runtime-nano` base (~242 MB) which contains only essential Qt libraries needed to run applications
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- This lesson demonstrates real-world CI/CD integration patterns
- Container detection is useful for environment-specific configuration
- CI environment variables follow standard conventions across platforms
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration
