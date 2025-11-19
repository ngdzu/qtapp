# Lesson 1: Qt Setup and Your First Qt Application

## Learning Goals
By the end of this lesson, you will:
- Understand the Qt framework architecture and its core components
- Set up a minimal Qt Widgets application with CMake
- Learn how the Qt event loop and application lifecycle work
- Build and run a simple "Hello Qt" application inside a container

## Introduction to Qt

Qt is a powerful cross-platform C++ framework for building desktop, mobile, and embedded applications. At its core, Qt provides an event-driven architecture built around the `QApplication` class and an event loop that processes user interactions, system events, and custom signals.

The most fundamental Qt application requires three components:
1. A `QApplication` instance that manages the event loop
2. At least one widget (like `QLabel` or `QPushButton`) to display
3. A call to `QApplication::exec()` to start the event loop

## Your First Qt Application

Let's examine a minimal Qt Widgets application:

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Hello, Qt!");
    label.setWindowTitle("Qt Setup");
    label.resize(300, 100);
    label.show();
    
    return app.exec();
}
```

**Key points:**
- `QApplication app(argc, argv)` creates the application instance and initializes Qt's internals
- `QLabel` is a simple widget that displays text
- `show()` makes the widget visible (widgets are hidden by default)
- `app.exec()` starts the event loop and blocks until the application quits

## CMake Integration

Modern Qt projects use CMake for cross-platform builds. The essential CMake setup requires:

```cmake
cmake_minimum_required(VERSION 3.16)
project(qt-setup)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(qt-setup main.cpp)
target_link_libraries(qt-setup Qt6::Widgets)
```

The `find_package(Qt6 COMPONENTS Widgets REQUIRED)` line locates Qt 6 and makes the Widgets module available. The `target_link_libraries` command links your executable with the Qt Widgets library.

## Expected Output

When you run the application, a small window appears with the text "Hello, Qt!" centered in a 300×100 pixel window. Clicking the window's close button terminates the application and `app.exec()` returns.

## Try It

Build and run this lesson inside the provided Docker container. Notice how Qt automatically handles window decoration, text rendering, and event processing with minimal code. Experiment by changing the label text, window size, or adding `label.setAlignment(Qt::AlignCenter)` to center the text.

## Next Steps

This foundation prepares you for deeper Qt concepts: layouts (lesson 4), signals and slots (lesson 5), and event handling (lesson 6). Understanding the application lifecycle and event loop is crucial for all Qt development.
