# Lesson 2: Modern C++ Prerequisites for Qt

This lesson demonstrates modern C++ features commonly used in Qt: RAII, smart pointers, lambda expressions, and move semantics. This is a console application that prints demonstrations to stdout.

## Building and Running

### One-Time Setup

These steps only need to be done once per machine.

#### 1. Build the shared Qt base images

From the **root directory** of the repository:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime-nano -t qtapp-qt-runtime-nano:latest .
```

> **Note:** The dev environment is ~1.33 GB (used only for building) and the runtime is ~242 MB. All lessons share these base images, so each individual lesson only adds ~16 KB (just the executable). This keeps total storage minimal even with 28 lessons!

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`02-cpp-prereqs`):

```bash
docker build -t qtapp-lesson02:latest .
```

#### Step 2: Run the application

This is a console application, so no X11 setup is needed:

```bash
docker run --rm qtapp-lesson02:latest
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
