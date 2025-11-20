# Answers: Signals and Slots

## Question 1
**What is the purpose of Qt's signals and slots mechanism?**

**Answer:** Signals and slots enable type-safe communication between objects without tight coupling.

**Explanation:** Signals and slots implement the observer pattern, allowing objects to communicate when events occur without knowing each other's implementation details. A signal is emitted when something happens (like a button click), and connected slots are automatically called. This decouples sender from receiver, making code more maintainable and flexible compared to traditional callback functions.

## Question 2
**Why must a class inherit from QObject and include the Q_OBJECT macro to use signals and slots?**

**Answer:** QObject provides the meta-object system, and Q_OBJECT enables the MOC to generate required code.

**Explanation:** QObject is the base class that provides Qt's meta-object system (signals, slots, properties, runtime type info). The Q_OBJECT macro tells Qt's Meta-Object Compiler (MOC) to generate additional code that makes signals and slots work. Without Q_OBJECT, the MOC won't process your class, and signals/slots won't function. This is why CMAKE_AUTOMOC is enabled in our CMakeLists.txt.

## Question 3
**What is the main advantage of the modern connection syntax over the old SIGNAL/SLOT macro syntax?**

**Answer:** The modern syntax provides compile-time type checking and error detection.

**Explanation:** Modern syntax uses function pointers (`&Class::signal`) instead of strings (`SIGNAL(signal())`). The compiler can verify that signals and slots exist and have compatible signatures, catching errors at compile time rather than runtime. It also enables IDE features like autocomplete and refactoring, and allows direct connection to lambdas without wrapper functions.

**Example:**
```cpp
// Modern: compiler checks if clicked() exists
connect(button, &QPushButton::clicked, this, &MyClass::handleClick);

// Old: typos only discovered at runtime
connect(button, SIGNAL(clcked()), this, SLOT(handleClick()));  // Typo!
```

## Question 4
**What will happen when this button is clicked?**

```cpp
QPushButton *button = new QPushButton("Click");
int count = 0;
QObject::connect(button, &QPushButton::clicked, [&count]() {
    count++;
    qDebug() << count;
});
```

**Answer:** Each click increments `count` and prints the new value (1, 2, 3, ...).

**Explanation:** The lambda captures `count` by reference (`[&count]`), so it can modify the original variable. First click: count becomes 1, prints 1. Second click: count becomes 2, prints 2. The lambda is called each time the button emits the `clicked()` signal. This pattern is useful for simple event handlers that need to maintain state.

## Question 5
**Spot the bug in this code:**

```cpp
class MyClass : public QObject
{
public:
    explicit MyClass(QObject *parent = nullptr);
signals:
    void dataReady(int value);
public slots:
    void processData(int value);
};
```

**Answer:** Missing Q_OBJECT macro in the class definition.

**Explanation:** Without the Q_OBJECT macro, the Meta-Object Compiler won't process this class, and signals/slots won't work. You'll get linker errors about missing vtable or meta-object methods.

**Corrected code:**
```cpp
class MyClass : public QObject
{
    Q_OBJECT  // Required!
public:
    explicit MyClass(QObject *parent = nullptr);
signals:
    void dataReady(int value);
public slots:
    void processData(int value);
};
```

## Question 6
**Fill in the missing line to connect a slider's value change to a label:**

```cpp
QSlider *slider = new QSlider();
QLabel *label = new QLabel("0");
// Missing line: connect slider valueChanged signal to update label text
```

**Answer:**
```cpp
QObject::connect(slider, &QSlider::valueChanged, [label](int value) {
    label->setText(QString::number(value));
});
```

**Explanation:** QSlider emits `valueChanged(int)` when the slider moves. We capture the label pointer in the lambda and use QString::number() to convert the integer value to text. This is a common pattern for synchronizing UI elements: one widget's change automatically updates another.

## Question 7
**What is the difference between Qt::DirectConnection and Qt::QueuedConnection?**

**Answer:** DirectConnection calls the slot immediately; QueuedConnection defers it to the event loop.

**Explanation:** 
- **Qt::DirectConnection**: Slot is invoked immediately when the signal is emitted, like a regular function call. Executes in the sender's thread.
- **Qt::QueuedConnection**: Slot call is queued as an event and invoked later when the receiver's event loop processes it. Essential for thread-safe cross-thread communication.
- **Qt::AutoConnection** (default): Uses DirectConnection if same thread, QueuedConnection if different threads.

QueuedConnection prevents blocking and enables safe communication between threads without explicit locking.

## Question 8
**Can one signal be connected to multiple slots? If yes, in what order are the slots called?**

**Answer:** Yes, one signal can connect to multiple slots, called in the order they were connected.

**Explanation:** Qt allows multiple connections to the same signal. When the signal is emitted, all connected slots are called sequentially in the order the connections were made. This is demonstrated in the lesson example where `valueChanged` is connected to multiple lambdas that update different UI elements. Each connection is independent—you can also connect the same signal to the same slot multiple times (though usually undesirable).

**Example:**
```cpp
connect(button, &QPushButton::clicked, this, &MyClass::handler1);  // Called first
connect(button, &QPushButton::clicked, this, &MyClass::handler2);  // Called second
connect(button, &QPushButton::clicked, this, &MyClass::handler3);  // Called third
```

## Question 9
**What does the `#include "main.moc"` line do when you have Q_OBJECT in a .cpp file?**

**Answer:** It includes the MOC-generated code for classes defined in the .cpp file.

**Explanation:** Qt's Meta-Object Compiler (MOC) generates additional C++ code for classes with Q_OBJECT. Normally, MOC processes .h files and creates separate moc_*.cpp files. When you define a QObject-derived class directly in a .cpp file (like our Counter class), you must manually include the generated .moc file at the end. CMake's AUTOMOC handles running MOC automatically, but you still need the include directive to pull in the generated code.

## Question 10
**How would you use signals and slots to implement a simple notification system where multiple UI elements need to update when data changes? Describe the design pattern you'd use.**

**Answer:** Create a data model class that emits signals when data changes, and connect multiple UI widgets to those signals.

**Explanation:** The Model-View pattern is ideal: create a QObject-derived model class that manages your data and emits signals like `dataChanged()` when modified. Connect each UI element (labels, graphs, tables) to this signal. When data updates, emit once, and all connected views update automatically. This decouples data from presentation.

**Example:**
```cpp
class DataModel : public QObject {
    Q_OBJECT
signals:
    void temperatureChanged(double temp);
    void humidityChanged(double humidity);
public slots:
    void updateSensorData(double temp, double humidity) {
        emit temperatureChanged(temp);
        emit humidityChanged(humidity);
    }
};

// Multiple UI elements can subscribe:
connect(model, &DataModel::temperatureChanged, tempLabel, 
    [tempLabel](double t) { tempLabel->setText(QString("%1°C").arg(t)); });
connect(model, &DataModel::temperatureChanged, tempGraph, &Graph::updateData);
connect(model, &DataModel::temperatureChanged, logger, &Logger::logTemp);
```

This pattern scales well: adding new UI elements just requires new connections, without modifying the model.
