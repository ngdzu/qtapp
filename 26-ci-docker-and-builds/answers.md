# Lesson 26 Quiz Answers: CI, Docker, and Builds

## 1. What is the primary benefit of multi-stage Docker builds for Qt applications, and how much size reduction can typically be achieved?

**Answer:** Dramatically reduced image size by separating build and runtime dependencies.

**Explanation:** Multi-stage builds keep the Qt SDK, compilers, CMake, and build tools in the builder stage (discarded after build), while the final image contains only runtime libraries and the executable. A single-stage Qt build might be 2-3GB with the full SDK, while multi-stage reduces this to 100-200MB (10-20x reduction). This speeds up deployment, reduces storage costs, and minimizes attack surface.

## 2. In the following Dockerfile snippet, what's wrong with this approach?

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y qt6-base-dev cmake g++
COPY . .
RUN cmake -B build && cmake --build build
CMD ["./build/myapp"]
```

**Answer:** Includes unnecessary build tools in final image, making it bloated.

**Explanation:** This single-stage build includes the entire Qt SDK (`qt6-base-dev`), CMake, and GCC in the final image, even though they're only needed during compilation. The corrected approach uses multi-stage builds:

```dockerfile
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y qt6-base-dev cmake g++
COPY . .
RUN cmake -B build && cmake --build build

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libqt6core6 libqt6widgets6
COPY --from=builder /build/myapp /usr/local/bin/
CMD ["myapp"]
```

Now only runtime libraries ship in the final image.

## 3. How do you make GitHub Actions cache Qt installation between builds to speed up CI runs?

**Answer:** Use the `cache` parameter in the `install-qt-action`.

**Explanation:** The `jurplel/install-qt-action` supports built-in caching. Enable it like this:

```yaml
- name: Install Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.5.0'
    cached: true
```

This caches the Qt installation in GitHub's cache storage. First build downloads Qt (~500MB, 2-3 minutes), subsequent builds restore from cache (~30 seconds). Cache expires after 7 days of non-use. You can also manually use `actions/cache@v3` to cache the Qt directory, but the action's built-in caching is simpler.

## 4. What environment variable can you check to detect if your Qt application is running inside a Docker container?

**Answer:** Check for the existence of `/.dockerenv` file or `DOCKER_CONTAINER` environment variable.

**Explanation:** Docker creates `/.dockerenv` file at the container root. You can check this in Qt:

```cpp
bool isDocker = QFile::exists("/.dockerenv");
```

Alternatively, check environment variables that CI systems or your Dockerfile set:

```cpp
bool isDocker = !qEnvironmentVariable("DOCKER_CONTAINER").isEmpty();
```

Or examine `/proc/1/cgroup` for "docker" string (Linux-specific). The most reliable cross-platform method is setting your own environment variable in the Dockerfile: `ENV DOCKER_CONTAINER=1`.

## 5. When using GitLab CI with artifacts, what's the advantage of separating build and test into different jobs/stages?

**Answer:** Enables parallel execution, faster feedback, and better resource utilization.

**Explanation:** Separating stages provides several benefits:

```yaml
build-job:
  stage: build
  script: cmake --build build
  artifacts:
    paths: [build/]

test-job:
  stage: test
  script: ctest
  dependencies: [build-job]
```

- **Failure isolation**: If tests fail, you can rerun just the test job without rebuilding
- **Parallel testing**: Multiple test jobs can consume the same build artifact
- **Different environments**: Build with SDK image, test with minimal runtime image
- **Faster iteration**: Fix tests and rerun without waiting for full rebuild
- **Resource optimization**: Use powerful builders for compilation, lightweight runners for testing

## 6. In a GitHub Actions build matrix testing Qt 6.5 and 6.6 on 3 platforms (Linux/Windows/macOS), how many build jobs run, and why might this fail if you have a free GitHub account?

**Answer:** 6 jobs run (2 Qt versions × 3 platforms), may hit free tier concurrency limits.

**Explanation:** The matrix creates all combinations:

```yaml
strategy:
  matrix:
    qt-version: ['6.5.0', '6.6.0']
    os: [ubuntu-latest, windows-latest, macos-latest]
# Generates 6 jobs: 
# ubuntu+6.5, ubuntu+6.6, windows+6.5, windows+6.6, macos+6.5, macos+6.6
```

**Free tier limitations:**
- **Public repos**: 20 concurrent jobs, unlimited minutes
- **Private repos**: 2000 minutes/month, macOS uses 10x multiplier (1 min = 10 billable mins)

A 6-job matrix on private repos with macOS can quickly consume monthly quota. Each macOS Qt build takes ~5-10 mins (50-100 billable mins × 2 Qt versions = 100-200 mins per commit). Solution: limit matrix to Linux for private repos or use self-hosted runners.

## 7. What's the purpose of the `COPY --from=builder` command in multi-stage Docker builds?

**Answer:** Copies artifacts from a previous build stage into the final image.

**Explanation:** In multi-stage builds, each `FROM` statement starts a new stage. The `COPY --from=stagename` instruction copies files from a named stage:

```dockerfile
FROM qt-dev:latest AS builder
RUN cmake --build build
# builder stage has: SDK, compilers, source, build artifacts

FROM qt-runtime:latest
COPY --from=builder /build/myapp /usr/local/bin/
# Final image has ONLY the executable, not the SDK/source
```

This is the key to size reduction. The builder stage contains everything needed to compile (2-3GB), but we only extract the compiled executable (~5MB) into the final image. All intermediate build files, source code, and build tools are discarded.

## 8. How can you detect whether your Qt application was built in Debug or Release mode at runtime?

**Answer:** Check `QT_DEBUG` preprocessor macro or use `QLibraryInfo`.

**Explanation:** Use preprocessor conditionals:

```cpp
#ifdef QT_DEBUG
    qDebug() << \"Debug build\";
#else
    qDebug() << \"Release build\";
#endif
```

Or check at runtime via build configuration:

```cpp
bool isDebug = QLibraryInfo::isDebugBuild();
```

For displaying build type to users:

```cpp
QString buildType;
#ifdef QT_DEBUG
    buildType = \"Debug\";
#else
    buildType = \"Release\";
#endif
```

This is useful in CI to verify you're testing the correct build type and in support scenarios to identify whether users are running debug (slow, with symbols) or release (fast, optimized) builds.

## 9. What's a common pitfall when running Qt GUI applications in Docker, and how do you fix it?

**Answer:** Missing X11 display server connection or platform plugin.

**Explanation:** Qt GUI apps need a display server. In Docker, this fails by default:

```
qt.qpa.plugin: Could not find the Qt platform plugin "xcb"
```

**Solutions:**

1. **X11 forwarding (Linux)**:
```bash
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix myapp
```

2. **XQuartz (macOS)**:
```bash
# Start XQuartz, allow network connections, then:
docker run -e DISPLAY=host.docker.internal:0 myapp
```

3. **Install platform plugins in image**:
```dockerfile
RUN apt-get install -y libqt6gui6 qt6-qpa-plugins
```

4. **Headless testing**:
```dockerfile
RUN apt-get install -y xvfb
CMD xvfb-run -a ./myapp
```

For CI/CD, `xvfb` (X virtual framebuffer) is ideal - provides display without actual screen.

## 10. If your CI build passes but the deployed application crashes with "cannot find libQt6Core.so.6", what's the likely cause and solution?

**Answer:** Missing Qt runtime libraries in deployment; use deployment tools or bundle libraries.

**Explanation:** The build environment has Qt libraries, but the deployment target doesn't. This happens when:

- Building on CI with Qt installed, deploying to clean system
- Using Docker build image but not runtime image
- Static linking disabled (Qt default is dynamic)

**Solutions:**

1. **Use deployment tools**:
```bash
# Linux
linuxdeployqt myapp -appimage

# macOS  
macdeployqt MyApp.app -dmg

# Windows
windeployqt myapp.exe
```

2. **Bundle libraries in Docker**:
```dockerfile
FROM ubuntu:22.04
RUN apt-get install -y libqt6core6 libqt6gui6 libqt6widgets6
COPY myapp /usr/local/bin/
```

3. **Static linking** (increases binary size but eliminates dependency):
```cmake
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
```

4. **Set LD_LIBRARY_PATH** (temporary fix, not recommended for production):
```bash
export LD_LIBRARY_PATH=/opt/qt/lib:$LD_LIBRARY_PATH
./myapp
```

Best practice: Use multi-stage Docker with explicit runtime library installation (solution 2) or deployment tools (solution 1).
