# Answers: Qt Widgets Basics

## Question 1
**What is the purpose of the QWidget class in Qt?**

**Answer:** QWidget is the base class for all user interface objects in Qt.

**Explanation:** Every visible element in a Qt Widgets application (buttons, labels, text fields, windows) inherits from QWidget. It provides fundamental functionality like drawing, event handling, geometry management, and parent-child relationships. When you create custom UI components, you typically inherit from QWidget or one of its subclasses.

## Question 2
**What happens if you create a QWidget without specifying a parent widget?**

**Answer:** It becomes a top-level window (an independent window).

**Explanation:** Widgets without parents are treated as separate windows by the operating system. They appear in the taskbar and can be moved independently. Widgets with parents are displayed inside their parent widget's area and move with the parent. This is demonstrated in the lesson where `QWidget window` (no parent) is a window, while `new QPushButton("Click", &window)` (with parent) appears inside the window.

## Question 3
**Why is the event loop important in Qt GUI applications?**

**Answer:** The event loop processes user interactions and keeps the application responsive.

**Explanation:** Without `app.exec()`, the program would reach the end of main() and terminate immediately. The event loop continuously processes events like mouse clicks, keyboard input, window repaints, and timer events. It's what makes your GUI interactive—when you click a button, the event loop receives the click event and delivers it to the button, which then emits the `clicked` signal.

## Question 4
**What is the output of the following code when you click the button twice?**

```cpp
QLabel *label = new QLabel("Count: 0");
QPushButton *button = new QPushButton("Click");
int count = 0;
QObject::connect(button, &QPushButton::clicked, [&]() {
    count++;
    label->setText(QString("Count: %1").arg(count));
});
```

**Answer:** The label will display "Count: 2" after two clicks.

**Explanation:** Each button click increments the `count` variable and updates the label text. First click: count becomes 1, label shows "Count: 1". Second click: count becomes 2, label shows "Count: 2". The lambda captures `count` and `label` by reference, so it can modify them directly.

**Code is correct as written.**

## Question 5
**Spot the bug in this code:**

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("My App");
    window.show();
    // Missing something here
}
```

**Answer:** Missing `return app.exec();` to start the event loop.

**Explanation:** Without calling `app.exec()`, the program shows the window briefly and then immediately exits when main() returns. The event loop is what keeps the application running and responsive.

**Corrected code:**
```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("My App");
    window.show();
    return app.exec();
}
```

## Question 6
**Fill in the missing line to connect a button click to show "Hello!" in a label:**

```cpp
QPushButton *button = new QPushButton("Say Hello");
QLabel *label = new QLabel("");
// Missing line here
```

**Answer:**
```cpp
QObject::connect(button, &QPushButton::clicked, [label]() {
    label->setText("Hello!");
});
```

**Explanation:** We connect the button's `clicked` signal to a lambda that sets the label text. The lambda captures `label` so it can call `setText()`. You could also use `[&]` to capture everything by reference, or `[=]` to capture by value.

## Question 7
**What is the difference between QPushButton and QLabel in terms of user interaction?**

**Answer:** QPushButton is interactive (responds to clicks), while QLabel is typically non-interactive (displays static content).

**Explanation:** QPushButton is designed for user interaction—it responds to mouse clicks and keyboard activation, emits signals when clicked, and provides visual feedback (highlighting, pressing effects). QLabel is designed for displaying text or images and doesn't respond to clicks by default. You can enable interaction on a QLabel with event handling, but that's not its primary purpose.

## Question 8
**What does `setGeometry(50, 100, 200, 30)` do to a widget?**

**Answer:** It sets the widget's position to (50, 100) and size to 200×30 pixels.

**Explanation:** The four parameters are: x position, y position, width, height. Position is relative to the parent widget's top-left corner. So `setGeometry(50, 100, 200, 30)` places the widget 50 pixels from the left edge, 100 pixels from the top edge, with a width of 200 pixels and height of 30 pixels. While this manual positioning works, layouts (covered in lesson 4) provide more flexible positioning.

## Question 9
**Why do widgets created with `new` and a parent pointer not require manual `delete`?**

**Answer:** Qt's parent-child ownership automatically deletes child widgets when the parent is destroyed.

**Explanation:** When you create a widget with a parent (`new QPushButton("Click", &window)`), Qt establishes a parent-child relationship. When the parent widget is destroyed, Qt automatically deletes all its children. This prevents memory leaks and simplifies memory management. In our example, when `window` goes out of scope and is destroyed, the button and label are automatically deleted.

**Pitfall to avoid:** Don't manually delete widgets that have parents unless you really know what you're doing—double deletion will crash your program.

## Question 10
**How would you use QPushButton and QLabel in a simple calculator application? Describe one possible use case.**

**Answer:** Use QPushButton widgets for number and operator buttons, and QLabel to display the current calculation or result.

**Explanation:** In a calculator app, you'd create QPushButton instances for digits 0-9, operators (+, -, ×, ÷), equals, and clear. When a user clicks a number button, you'd append that digit to the display. The QLabel would show the current input and results, updating as the user types. For example: user clicks "5", label shows "5"; clicks "+", label shows "5+"; clicks "3", label shows "5+3"; clicks "=", label shows "8". Each button's clicked signal would be connected to logic that updates the label text appropriately.
