# Lesson 27: Performance and Profiling

This lesson demonstrates Qt performance benchmarking and profiling techniques. The demo provides interactive benchmarks for string operations, container performance, rendering optimization, and memory management patterns. Students learn to measure, analyze, and optimize Qt application performance.

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
cd 27-performance-and-profiling
docker build -t qt-lesson-27 .
```

**Important:** For realistic performance measurements, rebuild in Release mode (already configured in Dockerfile). Debug builds can be 5-20x slower than Release builds.

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-27
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-27
```

## Expected Behavior

The application displays a tabbed interface with interactive performance benchmarks:

**String Performance Tab:**
- QString construction benchmarks (10,000 iterations)
- String concatenation without reserve() - shows performance impact of reallocations
- String concatenation with reserve() - demonstrates pre-allocation benefits
- QStringList join() - fastest method for building large strings
- Shows actual timing in milliseconds and microseconds per operation
- Displays speedup multipliers comparing optimized vs naive approaches

**Container Performance Tab:**
- QVector append without reserve - demonstrates reallocation overhead
- QVector append with reserve - shows dramatic performance improvement
- QVector prepend - illustrates O(n) cost of shifting elements
- QHash insert and lookup - demonstrates O(1) average performance
- Implicit sharing (copy-on-write) timing:
  - Copy operation: ~nanoseconds (just reference counting)
  - Detach operation: ~milliseconds (deep copy of 100k items)
- Shows speedup factors (e.g., reserve() can be 5-10x faster)

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

Each tab has a "Run Benchmarks" button to execute tests and display results. Results show actual timing data that varies by CPU but demonstrates relative performance differences.

## Learning Objectives

After completing this lesson, you should understand:
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
