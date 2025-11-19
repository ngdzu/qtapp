# Lesson 3: Qt Widgets Basics

## Learning Goals

- Understand QWidget as the base class for all UI components
- Create and configure QPushButton and QLabel widgets
- Learn how QApplication manages the event loop
- Build a simple interactive main window with basic widgets

## Introduction

Qt Widgets are the foundation of traditional desktop GUI applications in Qt. Every visual element you see in a Qt Widgets application—buttons, labels, text fields, windows—is a QWidget or a class derived from it. Understanding how to create, configure, and display these basic widgets is essential for building any Qt desktop application.

In this lesson, we'll explore three fundamental concepts: the QWidget base class, two common widget types (QPushButton and QLabel), and the event loop that makes Qt applications responsive.

## The QWidget Base Class

`QWidget` is the base class for all user interface objects. It provides:
- A rectangular area on screen with position and size
- The ability to receive mouse and keyboard events
- Painting capabilities for custom rendering
- Parent-child relationships for automatic memory management

When you create a QWidget without a parent, it becomes a top-level window. Widgets with parents are displayed inside their parent widget.

## QPushButton and QLabel

`QPushButton` is an interactive button widget that users can click:

```cpp
QPushButton *button = new QPushButton("Click Me");
button->setGeometry(50, 50, 100, 30);
```

`QLabel` displays static text or images:

```cpp
QLabel *label = new QLabel("Hello, Qt!");
label->setAlignment(Qt::AlignCenter);
```

## The Event Loop

Every Qt GUI application needs a `QApplication` object and must call `app.exec()` to start the event loop. The event loop:
- Processes user interactions (mouse clicks, keyboard input)
- Handles system events (window resize, paint requests)
- Delivers signals to connected slots
- Keeps your application responsive

Without the event loop, your window would appear briefly and immediately close.

## Example Walkthrough

Our example creates a simple window with a button and a label. Each button click updates the label to show the click count:

1. Create QApplication to initialize Qt's infrastructure
2. Create a main QWidget as the window
3. Add a QPushButton and QLabel as children
4. Connect the button's clicked signal to update the label
5. Show the window and start the event loop

```cpp
QWidget window;
window.resize(300, 200);
window.setWindowTitle("Widgets Basics");

QPushButton *button = new QPushButton("Click Me", &window);
QLabel *label = new QLabel("Clicks: 0", &window);

// Position widgets manually
button->setGeometry(100, 50, 100, 30);
label->setGeometry(50, 100, 200, 30);
```

## Expected Output

When you run the application, you'll see a window with a button labeled "Click Me" and a label showing "Clicks: 0". Each time you click the button, the label updates to show the number of clicks (Clicks: 1, Clicks: 2, etc.). The window remains open and responsive until you close it.

## Try It

1. Build and run the example application
2. Click the button multiple times and observe the label updates
3. Try modifying the button text or window title
4. Experiment with different button and label positions using `setGeometry()`
5. Add a second button that resets the counter to zero

## Key Takeaways

- QWidget is the foundation for all UI elements in Qt Widgets
- QPushButton and QLabel are basic, commonly-used widget types
- QApplication and the event loop are required for interactive GUI applications
- Widgets can be parented to other widgets for automatic memory management
- Basic positioning can be done with `setGeometry()` (layouts provide better solutions)
