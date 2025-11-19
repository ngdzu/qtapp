# Lesson 2: Modern C++ Prerequisites for Qt

## Learning Goals
By the end of this lesson, you will:
- Understand RAII and how Qt leverages it for automatic resource management
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) alongside Qt's parent-child ownership
- Write and connect lambda expressions to Qt signals
- Understand move semantics and when Qt uses them for performance

## Introduction

Qt is a modern C++ framework that embraces C++11/14/17 features. While Qt has its own conventions (like parent-child ownership for `QObject`), understanding modern C++ idioms makes you a more effective Qt developer. This lesson covers four essential concepts that appear throughout Qt applications.

## RAII (Resource Acquisition Is Initialization)

RAII is a C++ idiom where resource lifetime is tied to object lifetime. Qt uses this extensively: when a `QObject` is destroyed, its children are automatically deleted.

```cpp
{
    QWidget window;
    QPushButton *button = new QPushButton("Click", &window);
    // button is automatically deleted when window goes out of scope
}
```

The parent-child relationship (`&window` as parent) ensures automatic cleanup—no manual `delete` needed.

## Smart Pointers

While Qt's parent-child ownership handles most memory management, smart pointers are useful for non-`QObject` types or when you need explicit ownership semantics:

```cpp
auto data = std::make_unique<std::vector<int>>(1000);
// Automatically deleted when data goes out of scope

std::shared_ptr<Config> config = loadConfig();
// Reference-counted, deleted when last shared_ptr is destroyed
```

**Rule of thumb**: Use Qt parent-child for `QObject` hierarchies; use smart pointers for plain C++ objects or when ownership is shared/transferred.

## Lambda Expressions

Lambdas are anonymous functions, perfect for Qt signal-slot connections:

```cpp
QPushButton button("Click me");
QObject::connect(&button, &QPushButton::clicked, []() {
    qDebug() << "Button clicked!";
});
```

Lambdas can capture variables: `[&]` captures by reference, `[=]` by value, `[this]` captures the current object.

## Move Semantics

Move semantics (via `std::move`) transfer ownership instead of copying, improving performance for large objects:

```cpp
QString buildMessage() {
    QString msg = "Long message...";
    return msg;  // Automatically moved (RVO), not copied
}

QStringList list;
list.append(std::move(buildMessage()));  // Explicit move
```

Qt containers (`QList`, `QVector`, `QString`) are implicitly shared (copy-on-write) and support move operations for efficiency.

## Expected Output

The example program demonstrates these concepts by creating objects with RAII, using smart pointers for non-Qt data, connecting lambdas to signals, and moving strings efficiently. You'll see console output showing object creation and destruction order.

## Try It

Run the lesson program and observe the console output. Notice how:
1. Objects are destroyed in reverse order of creation (RAII)
2. Smart pointers clean up automatically
3. Lambdas execute when signals are emitted
4. Move operations avoid unnecessary copies

Experiment by modifying lambda captures or adding more smart pointers.

## Next Steps

These C++ foundations prepare you for Qt's advanced features like custom signals/slots (lesson 5), threading (lesson 17), and efficient data handling throughout the course. Mastering these concepts makes Qt code clearer and more maintainable.
