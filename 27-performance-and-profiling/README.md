# Lesson 27: Performance and Profiling

This lesson demonstrates Qt performance benchmarking and profiling techniques with interactive benchmarks for string operations, container performance, rendering optimization, and memory management.

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

From the **lesson directory** (`27-performance-and-profiling`):

```bash
docker build -t qtapp-lesson27:latest .
```

> **Important:** For realistic performance measurements, the Dockerfile builds in Release mode. Debug builds can be 5-20x slower than Release builds.

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson27:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson27:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./lesson27
```

## What You'll See

A comprehensive performance benchmarking application with interactive tabbed interface:

**String Performance Tab:**
- QString construction benchmarks (10,000 iterations)
- String concatenation without reserve() - shows performance impact of reallocations
- String concatenation with reserve() - demonstrates pre-allocation benefits
- QStringList join() - fastest method for building large strings
- Actual timing in milliseconds and microseconds per operation
- Speedup multipliers comparing optimized vs naive approaches

**Container Performance Tab:**
- QVector append without reserve - demonstrates reallocation overhead
- QVector append with reserve - shows dramatic performance improvement
- QVector prepend - illustrates O(n) cost of shifting elements
- QHash insert and lookup - demonstrates O(1) average performance
- Implicit sharing (copy-on-write) timing:
  - Copy operation: ~nanoseconds (just reference counting)
  - Detach operation: ~milliseconds (deep copy of 100k items)
- Speedup factors (e.g., reserve() can be 5-10x faster)

**Rendering Performance Tab:**
- Multiple update() calls - shows Qt's automatic coalescing
- Batch updates with setUpdatesEnabled(false) - explicit control
- Timing comparison between approaches
- Signal/slot overhead reference guide:
  - Direct function call: ~5 ns
  - Signal/slot DirectConnection: ~30 ns
  - Signal/slot QueuedConnection: ~500 ns
- Best practices for rendering optimization

**Memory Management Tab:**
- QObject parent-child ownership examples with code
- Common memory leak patterns and fixes
- Layout ownership demonstration
- Live memory allocation test:
  - Allocates 10,000 QObjects with proper parenting
  - Times allocation and deletion
  - Demonstrates automatic cleanup via parent deletion
- Profiling tools reference (Valgrind, Instruments, Dr. Memory, AddressSanitizer)
- Best practices checklist for memory management

Each tab has a "Run Benchmarks" button to execute tests and display actual timing results!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS
- **Build Mode:** Release mode recommended for accurate benchmarks

## Learning Objectives

- How to use QElapsedTimer for precise performance measurements
- The performance impact of QString operations and how to optimize them
- When and why to use reserve() with Qt containers
- Implicit sharing (copy-on-write) behavior and its performance characteristics
- How to optimize widget rendering with update coalescing
- Qt's parent-child memory management pattern
- Signal/slot overhead and when direct calls are necessary
- The difference between Debug and Release build performance
- How to interpret benchmark results and identify bottlenecks
- Best practices for memory profiling and leak detection

## Notes

- The Dockerfile uses a multi-stage build: lessons use the `qt-runtime-nano` base (~242 MB) which contains only essential Qt libraries needed to run applications
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- **Critical:** Always use Release builds for performance benchmarks - Debug builds include instrumentation that dramatically slows execution
- Benchmark results vary by CPU but demonstrate relative performance differences
- Qt's implicit sharing uses copy-on-write for efficient memory usage
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration

## Performance Tips

**Always Profile First:**
- Don't optimize without measuring
- Use QElapsedTimer for accurate timing
- Profile in Release mode, not Debug
- Focus on hot paths (code that runs frequently)

**Common Optimizations:**
- Pre-allocate containers with reserve()
- Avoid QString concatenation in loops
- Use const references to prevent container detachment
- Batch widget updates with setUpdatesEnabled()
- Let Qt manage memory via parent-child relationships

**Profiling Tools:**
- **Linux**: Valgrind (memory), perf (CPU), Heaptrack (heap)
- **macOS**: Instruments (Leaks, Allocations, Time Profiler)
- **Windows**: Visual Studio Profiler, Dr. Memory
- **Cross-platform**: AddressSanitizer (-fsanitize=address)
