# Lesson 5: Signals and Slots

## Learning Goals

- Understand Qt's signals and slots mechanism for event-driven programming
- Learn the modern (Qt 5+) connection syntax
- Connect signals to lambda functions for inline event handling
- Understand the difference between direct and queued connections
- Master QObject as the foundation of Qt's object system

## Introduction

Signals and slots are Qt's mechanism for communication between objects. This design pattern enables loose coupling: objects can communicate without knowing about each other's internal implementation. A signal is emitted when a particular event occurs, and a slot is a function that gets called in response to that signal.

Unlike traditional callbacks, Qt's signals and slots are type-safe, flexible, and can cross thread boundaries. They form the backbone of event-driven programming in Qt applications.

## QObject: The Foundation

`QObject` is the base class for all Qt objects that can use signals and slots. Any class that needs this functionality must inherit from QObject and include the `Q_OBJECT` macro:

```cpp
class Counter : public QObject
{
    Q_OBJECT
public:
    explicit Counter(QObject *parent = nullptr);
signals:
    void valueChanged(int newValue);
public slots:
    void increment();
};
```

The `Q_OBJECT` macro enables Qt's meta-object system, which provides signals, slots, properties, and runtime type information.

## Modern Connection Syntax

Qt 5 introduced a type-safe connection syntax using function pointers instead of the old string-based SIGNAL/SLOT macros:

```cpp
// Modern syntax (preferred)
connect(button, &QPushButton::clicked, this, &MyClass::handleClick);

// Old syntax (avoid)
connect(button, SIGNAL(clicked()), this, SLOT(handleClick()));
```

The modern syntax provides compile-time checking and supports lambda functions directly.

## Connecting to Lambda Functions

One of the most powerful features is connecting signals to lambda functions for inline event handling:

```cpp
QPushButton *button = new QPushButton("Click Me");
int clickCount = 0;
connect(button, &QPushButton::clicked, [&clickCount]() {
    clickCount++;
    qDebug() << "Clicked" << clickCount << "times";
});
```

Lambdas are perfect for simple event handlers that don't warrant a separate function, and they can capture local variables.

## Signal Parameters and Slots

Signals can pass parameters to slots. The slot's signature must match or be compatible with the signal:

```cpp
// Signal with parameter
signals:
    void valueChanged(int newValue);

// Matching slot
public slots:
    void onValueChanged(int value) {
        qDebug() << "New value:" << value;
    }

// Connection
connect(sender, &Sender::valueChanged, receiver, &Receiver::onValueChanged);
```

## Direct vs Queued Connections

Qt supports different connection types that control when and how slots are invoked:

- **Direct Connection** (Qt::DirectConnection): Slot is called immediately when signal is emitted, like a normal function call
- **Queued Connection** (Qt::QueuedConnection): Slot is called via the event loop when control returns to the receiver's thread
- **Auto Connection** (Qt::AutoConnection, default): Direct if same thread, queued if different threads

The connection type is specified as the fifth parameter:

```cpp
connect(sender, &Sender::signal, receiver, &Receiver::slot, Qt::QueuedConnection);
```

Queued connections are essential for thread-safe communication across threads.

## Example Walkthrough

Our example demonstrates:
1. A counter that emits signals when incremented
2. Multiple slots responding to the same signal
3. Lambda connections with captures
4. Direct vs queued connection behavior

The application shows how one signal can trigger multiple slots, and how lambdas provide convenient inline handlers.

## Expected Output

The application displays a window with increment buttons and labels showing:
- Current counter value
- Number of times buttons were clicked
- Messages demonstrating when slots are called

Each button click triggers multiple responses: the counter updates, labels refresh, and debug messages appear showing the order of execution.

## Try It

1. Build and run the application
2. Click the increment button and observe multiple slots being called
3. Add your own lambda connection to log additional information
4. Create a custom QObject subclass with your own signals
5. Experiment with Qt::QueuedConnection to see deferred execution
6. Try disconnecting a signal: `disconnect(sender, &Sender::signal, ...)`

## Key Takeaways

- Signals and slots enable loose coupling between objects
- QObject and Q_OBJECT macro are required for signals/slots
- Modern syntax uses function pointers for type safety
- Lambdas provide convenient inline event handlers
- One signal can connect to multiple slots
- Connection types control execution timing (direct vs queued)
- Signals and slots can safely cross thread boundaries with queued connections
