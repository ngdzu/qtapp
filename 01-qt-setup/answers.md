# Answers: Qt Setup and Your First Qt Application

## Question 1 (Conceptual)
**Q:** Why is the `QApplication` instance required in every Qt Widgets application? What would happen if you tried to create a widget without first creating a `QApplication`?

**A:** `QApplication` initializes the Qt framework and manages the event loop.

Without `QApplication`, Qt widgets cannot function properly because they rely on the application-wide event system, resource management, and platform integration that `QApplication` provides. Attempting to create widgets without it typically results in runtime errors or crashes with messages about missing `QApplication` instance.

**Pitfall:** A common mistake is creating widgets before instantiating `QApplication`, which leads to undefined behavior or crashes. Always create `QApplication` as the first object in `main()`.

---

## Question 2 (Conceptual)
**Q:** What is the purpose of calling `app.exec()` at the end of the `main()` function? What does this function return?

**A:** `app.exec()` starts the Qt event loop, which processes events until the application quits.

The event loop handles user input, window events, timers, and signals/slots. The function blocks until the application terminates (e.g., when the user closes all windows or calls `QApplication::quit()`). It returns an integer exit code: 0 indicates successful termination, while non-zero values indicate errors.

---

## Question 3 (Practical/Code)
**Q:** What is wrong with the following code?

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Hello!");
    label.resize(200, 50);
    
    return app.exec();
}
```

**A:** The label is never shown, so no window appears.

Widgets are hidden by default in Qt. You must call `label.show()` to make the widget visible. Without this call, the application runs but displays nothing.

**Corrected code:**
```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Hello!");
    label.resize(200, 50);
    label.show();  // <-- This line was missing
    
    return app.exec();
}
```

---

## Question 4 (Practical/Code)
**Q:** Fill in the missing line to make the label's text centered.

**A:** `label.setAlignment(Qt::AlignCenter);`

This sets the label's text alignment to center both horizontally and vertically within the widget's rectangle. The `Qt::AlignCenter` flag is a combination of `Qt::AlignHCenter | Qt::AlignVCenter` from the `Qt::Alignment` enum.

**Complete code:**
```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Centered Text");
    label.setAlignment(Qt::AlignCenter);  // <-- Missing line
    label.resize(300, 100);
    label.show();
    
    return app.exec();
}
```

---

## Question 5 (Practical/Code)
**Q:** In CMake, what does `find_package(Qt6 COMPONENTS Widgets REQUIRED)` do? What happens if Qt6 is not found on the system?

**A:** It locates the Qt6 installation and loads the Widgets module for use in the project.

CMake searches for Qt6 in standard locations and platform-specific paths. The `REQUIRED` keyword means CMake will halt configuration with an error if Qt6 or the Widgets component is not found, preventing accidental builds without the necessary dependencies.

---

## Question 6 (Conceptual)
**Q:** What is the difference between creating a widget on the stack (like `QLabel label;`) versus on the heap (like `QLabel *label = new QLabel;`)? Which approach is safer for a top-level window in Qt?

**A:** Stack widgets are automatically destroyed when they go out of scope; heap widgets require manual memory management or parent-child ownership.

For top-level windows in `main()`, stack allocation is safer and simpler because the widget is automatically cleaned up when `main()` exits. Heap allocation requires either explicit `delete` or relying on Qt's parent-child ownership (where a parent widget automatically deletes its children). In this lesson's simple case, stack allocation is preferred.

**Pitfall:** Forgetting to `delete` heap-allocated widgets causes memory leaks. Use stack allocation for simple cases or ensure proper ownership via Qt's parent mechanism.

---

## Question 7 (Practical/Code)
**Q:** What does the following CMake line accomplish?

```cmake
target_link_libraries(qt-setup Qt6::Widgets)
```

**A:** It links the `qt-setup` executable with the Qt6 Widgets library.

This command tells the linker to include the Qt Widgets library when building the executable, making Qt classes and functions available at link time. Without this, you'd get linker errors about undefined references to Qt symbols.

---

## Question 8 (Reflection)
**Q:** How would you extend this minimal application to display two pieces of information (e.g., a title and a message)? What Qt widgets or layout strategies might you use?

**A:** You could use two `QLabel` widgets inside a `QVBoxLayout` to stack them vertically, or use a single `QLabel` with multi-line text.

For more complex UIs, layouts (covered in lesson 4) are essential. A simple approach:

```cpp
QWidget window;
QVBoxLayout *layout = new QVBoxLayout(&window);
layout->addWidget(new QLabel("Title"));
layout->addWidget(new QLabel("Message"));
window.show();
```

This demonstrates the need for container widgets and layout management, which are core Qt concepts for building real applications.

---

## Question 9 (Conceptual)
**Q:** Why do Qt widgets need to call `show()` explicitly? What is the benefit of this design compared to making widgets visible by default?

**A:** Explicit `show()` gives developers control over when widgets become visible, enabling setup before display.

This design allows you to configure a widget's properties (size, position, child widgets, etc.) before it appears on screen, preventing visual glitches or partial redraws. It also supports scenarios where widgets are created but conditionally shown based on application logic.

---

## Question 10 (Reflection)
**Q:** This lesson uses Docker to ensure a consistent build environment. In your own projects, how might containerization help when working with different Qt versions or operating systems?

**A:** Containers isolate dependencies, ensuring everyone uses the same Qt version, compiler, and libraries regardless of their host OS.

This eliminates "works on my machine" problems, makes CI/CD pipelines reproducible, and simplifies onboarding new developers. For Qt projects, you can lock to a specific Qt version and avoid conflicts with system-installed Qt packages, making builds more predictable and portable across Linux, macOS, and Windows development environments.
