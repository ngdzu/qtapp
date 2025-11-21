# Lesson 26: CI, Docker, and Builds

This lesson demonstrates build environment detection, Docker container identification, and CI/CD platform recognition. The demo shows how Qt applications can detect their build configuration, whether they're running in Docker, and which CI system (if any) is executing them.

## Prerequisites

For GUI applications on macOS, you need to set up X11 forwarding:
1. Install XQuartz: `brew install --cask xquartz`
2. Start XQuartz and enable "Allow connections from network clients" in Preferences > Security
3. Run: `xhost + localhost`

## Building

First, ensure the base images are built:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then build this lesson:

```bash
cd 26-ci-docker-and-builds
docker build -t qt-lesson-26 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e DOCKER_CONTAINER=1 qt-lesson-26
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -e DOCKER_CONTAINER=1 qt-lesson-26
```

**Note:** The `-e DOCKER_CONTAINER=1` flag demonstrates environment variable detection in the CI/CD tab.

## Expected Behavior

The application displays a tabbed interface with build and environment information:

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

The window includes a "Refresh Information" button to update all tabs with current state.

## Learning Objectives

After completing this lesson, you should understand:
- How to detect build configuration (Debug/Release) at runtime
- Multiple methods for detecting Docker container execution
- How to identify CI/CD platforms via environment variables
- The benefits of multi-stage Docker builds for Qt applications
- How to report build environment information for debugging
- Best practices for CI/CD pipeline design
- Environment variable conventions across different CI systems

## CI/CD Integration Examples

This application demonstrates patterns useful for:
- **Build verification**: Confirming Debug/Release mode in CI
- **Environment debugging**: Identifying why builds fail in CI but work locally
- **Container detection**: Adjusting behavior when running in Docker
- **CI platform detection**: Enabling platform-specific features
- **Artifact tagging**: Using build info for version strings and metadata
