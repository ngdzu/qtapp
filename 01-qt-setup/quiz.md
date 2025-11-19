# Quiz: Qt Setup and Your First Qt Application

## Question 1 (Conceptual)
Why is the `QApplication` instance required in every Qt Widgets application? What would happen if you tried to create a widget without first creating a `QApplication`?

## Question 2 (Conceptual)
What is the purpose of calling `app.exec()` at the end of the `main()` function? What does this function return?

## Question 3 (Practical/Code)
What is wrong with the following code?

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

## Question 4 (Practical/Code)
Fill in the missing line to make the label's text centered:

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Centered Text");
    // ??? missing line here
    label.resize(300, 100);
    label.show();
    
    return app.exec();
}
```

## Question 5 (Practical/Code)
In CMake, what does `find_package(Qt6 COMPONENTS Widgets REQUIRED)` do? What happens if Qt6 is not found on the system?

## Question 6 (Conceptual)
What is the difference between creating a widget on the stack (like `QLabel label;`) versus on the heap (like `QLabel *label = new QLabel;`)? Which approach is safer for a top-level window in Qt?

## Question 7 (Practical/Code)
What does the following CMake line accomplish?

```cmake
target_link_libraries(qt-setup Qt6::Widgets)
```

## Question 8 (Reflection)
How would you extend this minimal application to display two pieces of information (e.g., a title and a message)? What Qt widgets or layout strategies might you use?

## Question 9 (Conceptual)
Why do Qt widgets need to call `show()` explicitly? What is the benefit of this design compared to making widgets visible by default?

## Question 10 (Reflection)
This lesson uses Docker to ensure a consistent build environment. In your own projects, how might containerization help when working with different Qt versions or operating systems?
