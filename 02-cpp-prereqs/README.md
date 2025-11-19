# Lesson 2: Modern C++ Prerequisites for Qt

This lesson demonstrates modern C++ features commonly used in Qt: RAII, smart pointers, lambda expressions, and move semantics. This is a console application that prints demonstrations to stdout.

## Building and Running

### Option 1: Build and run inside Docker (recommended)

**Step 1: Build the shared Qt base image** (only needed once)

From the root directory of the repository, run:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
```

**Step 2: Build the lesson image** (reuses the shared base)

From the lesson directory:

```bash
cd 02-cpp-prereqs
docker build -t qt-lesson-02 .
```

> **Note:** The lesson image shares ~987 MB of layers with the base image, so actual disk usage for this lesson is only ~352 MB.

**Run the application**

This is a console application, so no X11 setup is needed:

```bash
docker run --rm qt-lesson-02
```

### Option 2: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./cpp-prereqs
```

## What You'll See

The program demonstrates four modern C++ concepts:

1. **RAII**: Automatic resource cleanup when objects go out of scope
2. **Smart pointers**: `std::unique_ptr` managing a vector automatically
3. **Lambda expressions**: Connected to Qt signals with variable capture
4. **Move semantics**: Efficient string transfer without copying

Watch the console output showing the order of object creation and destruction.

## Requirements

- **Qt Modules:** Qt6::Core, Qt6::Widgets
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (optional but recommended)

## Notes

- This lesson uses `QCoreApplication` (console app) instead of `QApplication` (GUI app), so no display/X11 is required.
- The code demonstrates concepts you'll use throughout Qt development, especially in lessons 5 (signals/slots) and 17 (threading).
- Smart pointers are shown alongside Qt's parent-child ownership to clarify when to use each approach.

## Troubleshooting

**"Qt6 not found"**: Verify Qt 6 is installed in the container (included in the Dockerfile) or on your local system.

**No output**: Ensure the container isn't running in detached mode. Use `docker run --rm qt-lesson-02` (without `-d` flag).
