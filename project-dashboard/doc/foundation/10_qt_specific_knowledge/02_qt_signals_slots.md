# Qt Signals & Slots Deep Dive

This document provides comprehensive coverage of Qt's signals and slots mechanism, which is fundamental to event-driven programming in Qt and used throughout the Z Monitor application.

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## 1. Overview

### **What are Signals and Slots?**

Signals and slots are Qt's implementation of the **Observer Pattern**, providing a flexible mechanism for communication between objects without tight coupling.

- **Signal:** Declaration that an event has occurred (emitted by an object)
- **Slot:** Function that is called in response to a signal
- **Connection:** Link between a signal and one or more slots

### **Why Use Signals & Slots?**

✅ **Loose Coupling** - Objects don't need to know about each other  
✅ **Type Safety** - Compile-time checking of signal/slot compatibility  
✅ **Thread Safety** - Built-in support for cross-thread communication  
✅ **Flexible** - One signal can connect to multiple slots  
✅ **Callbacks** - Cleaner alternative to function pointers  

---

## 2. Basic Syntax

### **2.1 Declaring Signals**

```cpp
class PatientMonitor : public QObject {
    Q_OBJECT  // Required for signals/slots
    
signals:
    // Signals are always public
    void vitalSignUpdated(int heartRate, int spo2);
    void alarmTriggered(const QString& alarmType);
    void patientAdmitted(const QString& mrn);
    
    // Signals have no implementation
    // Return type is always void
};
```

**Rules:**
- Signals must be declared in `signals:` section
- Always `void` return type
- Never implement signal functions
- Can have parameters (passed to connected slots)
- Always public (access specifier ignored)

### **2.2 Declaring Slots**

```cpp
class AlarmManager : public QObject {
    Q_OBJECT
    
public slots:
    // Public slots can be called from anywhere
    void handleAlarm(const QString& alarmType);
    void acknowledgeAlarm(int alarmId);
    
private slots:
    // Private slots only callable as slots (not as regular methods externally)
    void checkAlarmConditions();
    void updateAlarmPriority();
    
private:
    void processAlarm();  // Regular method, not a slot
};
```

**Implementation:**

```cpp
void AlarmManager::handleAlarm(const QString& alarmType) {
    qDebug() << "Handling alarm:" << alarmType;
    // Process alarm logic
    processAlarm();
}
```

**Rules:**
- Slots are regular C++ methods
- Can be `public`, `protected`, or `private`
- Can have return values (but ignored for queued connections)
- Can be called directly like normal methods
- Can be virtual

---

## 3. Connecting Signals to Slots

### **3.1 Modern (Qt 5+) Connect Syntax**

**Recommended for new code:**

```cpp
// Function pointer syntax (compile-time checked)
QObject::connect(
    sender,              // Sender object
    &SenderClass::signalName,    // Signal (pointer to member function)
    receiver,            // Receiver object
    &ReceiverClass::slotName     // Slot (pointer to member function)
);

// Example:
QObject::connect(
    patientMonitor, &PatientMonitor::vitalSignUpdated,
    database, &DatabaseManager::saveVitals
);
```

**Benefits:**
- ✅ Compile-time type checking
- ✅ Autocomplete support
- ✅ Refactoring-friendly (rename detection)
- ✅ Works with overloaded signals/slots

### **3.2 Lambda Connections**

```cpp
// Connect to lambda function
QObject::connect(
    patientMonitor, &PatientMonitor::vitalSignUpdated,
    [this](int heartRate, int spo2) {
        // Inline logic
        qDebug() << "HR:" << heartRate << "SpO2:" << spo2;
        if (heartRate > 120) {
            triggerAlarm();
        }
    }
);
```

**Benefits:**
- ✅ No need to declare slot
- ✅ Can capture context (`[this]`, `[=]`, `[&]`)
- ✅ Inline logic for simple cases

**Caution:**
- ⚠️ Be careful with captures (can cause dangling references)
- ⚠️ Hard to disconnect (need QMetaObject::Connection handle)

### **3.3 Old (Qt 4) String-Based Syntax**

**Not recommended (but still supported):**

```cpp
// ❌ Old style - runtime checking only
connect(
    sender, SIGNAL(signalName(int, int)),
    receiver, SLOT(slotName(int, int))
);

// Example:
connect(
    patientMonitor, SIGNAL(vitalSignUpdated(int, int)),
    database, SLOT(saveVitals(int, int))
);
```

**Drawbacks:**
- ❌ No compile-time checking (errors at runtime)
- ❌ No autocomplete
- ❌ Typos not caught
- ❌ Refactoring breaks connections

**Only use for:**
- Legacy code maintenance
- QML connections (uses similar syntax)

---

## 4. Connection Types

### **4.1 Direct Connection (`Qt::DirectConnection`)**

**How it works:**
- Slot is called **immediately** when signal is emitted
- Slot runs in **sender's thread**
- Synchronous (sender waits for slot to finish)

```cpp
QObject::connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot,
    Qt::DirectConnection  // Explicit
);
```

**Use when:**
- ✅ Sender and receiver in same thread
- ✅ Need immediate response
- ✅ Slot execution is fast

**Caution:**
- ⚠️ Deadlock risk if slot calls back to sender
- ⚠️ Stack overflow risk with signal chains
- ⚠️ NOT thread-safe across threads

**Example in Z Monitor:**

```cpp
// Real-time alarm processing (same thread)
connect(
    vitalMonitor, &VitalMonitor::vitalsUpdated,
    alarmDetector, &AlarmDetector::checkThresholds,
    Qt::DirectConnection  // Must be immediate for <50ms latency
);
```

### **4.2 Queued Connection (`Qt::QueuedConnection`)**

**How it works:**
- Slot is queued as **event** in receiver's event loop
- Slot runs in **receiver's thread**
- Asynchronous (sender continues immediately)
- Parameters are **copied** (must be copyable and registered with Qt)

```cpp
QObject::connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot,
    Qt::QueuedConnection  // Explicit
);
```

**Use when:**
- ✅ **Cross-thread communication** (REQUIRED for thread safety)
- ✅ Decoupling (sender doesn't wait)
- ✅ Parameters are lightweight or copyable

**Caution:**
- ⚠️ Delay between emit and slot execution
- ⚠️ Parameters must be copy-constructible
- ⚠️ Custom types must be registered: `qRegisterMetaType<CustomType>()`

**Example in Z Monitor:**

```cpp
// Cross-thread: Real-time thread → Database thread
connect(
    vitalGenerator, &VitalGenerator::vitalReady,  // Real-time thread
    database, &DatabaseManager::saveVital,        // Database thread
    Qt::QueuedConnection  // MUST use queued for thread safety
);

// Register custom types for queued connections
qRegisterMetaType<VitalRecord>("VitalRecord");
```

### **4.3 Automatic Connection (`Qt::AutoConnection`)** ⭐ DEFAULT

**How it works:**
- Qt **automatically chooses** based on sender/receiver thread
- If **same thread** → `DirectConnection`
- If **different threads** → `QueuedConnection`

```cpp
QObject::connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot
    // Qt::AutoConnection is default (not specified)
);
```

**Use when:**
- ✅ **Most cases** (recommended default)
- ✅ Flexible code (works regardless of thread)
- ✅ Don't know thread at compile time

**Caution:**
- ⚠️ Be aware which mode is active (affects timing)
- ⚠️ For real-time code, prefer explicit `Qt::DirectConnection`

### **4.4 Blocking Queued Connection (`Qt::BlockingQueuedConnection`)**

**How it works:**
- Queued like `QueuedConnection`
- But sender **blocks** until slot finishes
- Slot runs in **receiver's thread**

```cpp
QObject::connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot,
    Qt::BlockingQueuedConnection
);
```

**Use when:**
- ✅ Need cross-thread call with return value
- ✅ Rare synchronization needs

**Caution:**
- ⚠️ **NEVER** use from main thread (will freeze UI)
- ⚠️ Deadlock risk (sender waits for receiver's event loop)
- ⚠️ Only use from worker threads

**Example:**

```cpp
// Worker thread needs synchronous result from main thread
// (Rare - usually bad design)
connect(
    worker, &Worker::requestData,
    mainThread, &MainThread::provideData,
    Qt::BlockingQueuedConnection
);
```

### **4.5 Unique Connection (`Qt::UniqueConnection`)**

**How it works:**
- Connection is made **only if it doesn't already exist**
- Prevents duplicate connections

```cpp
QObject::connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot,
    Qt::AutoConnection | Qt::UniqueConnection  // Combine with other types
);
```

**Use when:**
- ✅ Dynamic connections that might be called multiple times
- ✅ Plugin systems
- ✅ Preventing duplicate signal processing

---

## 5. Advanced Topics

### **5.1 Connection Lifetime Management**

**Automatic Disconnection:**

```cpp
// Connections automatically broken when sender or receiver destroyed
PatientMonitor* monitor = new PatientMonitor();
AlarmManager* alarmManager = new AlarmManager();

connect(monitor, &PatientMonitor::alarmTriggered,
        alarmManager, &AlarmManager::handleAlarm);

delete monitor;  // Connection automatically broken
// alarmManager->handleAlarm() will NEVER be called after this
```

**Manual Disconnection:**

```cpp
// Save connection handle
QMetaObject::Connection connection = connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot
);

// Later: disconnect
QObject::disconnect(connection);
```

**Disconnect all signals from object:**

```cpp
// Disconnect ALL signals from sender
sender->disconnect();

// Disconnect specific signal from sender
sender->disconnect(SIGNAL(signalName()));

// Disconnect all connections to receiver
disconnect(sender, nullptr, receiver, nullptr);
```

**Lambda Disconnection:**

```cpp
// Must save handle to disconnect lambda
auto connection = connect(sender, &Sender::signal, [this]() {
    // Do something
});

// Disconnect lambda
disconnect(connection);
```

### **5.2 Overloaded Signals and Slots**

**Problem:** Ambiguity with overloads

```cpp
class DataSource : public QObject {
    Q_OBJECT
signals:
    void dataReady(int value);
    void dataReady(const QString& value);  // Overload
};
```

**Solution:** Use `qOverload` or function pointer cast

```cpp
// Qt 5.7+: Use qOverload
connect(
    dataSource, qOverload<int>(&DataSource::dataReady),
    receiver, &Receiver::handleInt
);

connect(
    dataSource, qOverload<const QString&>(&DataSource::dataReady),
    receiver, &Receiver::handleString
);

// Pre-Qt 5.7: Use static_cast
connect(
    dataSource, static_cast<void (DataSource::*)(int)>(&DataSource::dataReady),
    receiver, &Receiver::handleInt
);
```

### **5.3 Signals Connected to Signals**

**Forwarding signals:**

```cpp
class DeviceManager : public QObject {
    Q_OBJECT
signals:
    void deviceConnected();
    void statusChanged();
    
public:
    DeviceManager() {
        // Forward signal from one object to another
        connect(device, &Device::connected, 
                this, &DeviceManager::deviceConnected);
    }
};
```

**Use for:**
- Signal forwarding/aggregation
- Facade pattern
- Event bubbling

### **5.4 Parameters and Type Compatibility**

**Rules:**
- Slot can have **fewer** parameters than signal (extras ignored)
- Slot parameters must be **same or more general** types
- Signal parameters are **copied** for queued connections

```cpp
signals:
    void dataReady(int value, const QString& message);
    
slots:
    void handleData(int value, const QString& message);  // ✅ Exact match
    void handleDataSimple(int value);                    // ✅ Fewer params
    void handleDataGeneral(QVariant value);              // ✅ More general type
    void handleDataWrong(int value, int other);          // ❌ Incompatible
```

### **5.5 Return Values**

**Direct connections:**
- Last connected slot's return value is returned
- Usually ignored

```cpp
signals:
    void requestValue();
    
public slots:
    int provideValue() { return 42; }  // Return value available

// Usage:
int result = emit requestValue();  // Gets return value from last slot
```

**Queued connections:**
- Return values are **always ignored** (async)
- Use signals to return results instead

```cpp
// ❌ BAD: Won't work with queued
int result = emit asyncRequest();  // Returns void for queued

// ✅ GOOD: Use signal for response
signals:
    void asyncRequest();
    void asyncResponse(int result);
    
// Receiver emits response signal with result
```

---

## 6. Performance Considerations

### **6.1 Signal/Slot Overhead**

**Direct Connection:**
- Similar to virtual function call
- Very low overhead (~5-10 nanoseconds)
- Suitable for high-frequency signals

**Queued Connection:**
- Event queue overhead
- Parameter copying
- Higher latency (~microseconds to milliseconds)
- Not suitable for >10kHz signals

### **6.2 Optimization Tips**

```cpp
// ✅ GOOD: Pre-connect in constructor
MyClass::MyClass() {
    connect(timer, &QTimer::timeout, this, &MyClass::update);
}

// ❌ BAD: Connecting inside frequently-called method
void MyClass::processData() {
    connect(source, &Source::data, this, &MyClass::handle);  // Leaks!
    // Creates new connection every call
}

// ✅ GOOD: Disconnect when not needed
void MyClass::pauseProcessing() {
    disconnect(source, &Source::data, this, &MyClass::handle);
}

// ✅ GOOD: Use Qt::UniqueConnection for dynamic connections
void MyClass::ensureConnected() {
    connect(source, &Source::data, this, &MyClass::handle,
            Qt::UniqueConnection);  // Safe to call multiple times
}
```

### **6.3 Connection Benchmarks (Z Monitor)**

| Connection Type | Latency | Use Case |
|-----------------|---------|----------|
| Direct (same thread) | < 10 ns | Real-time alarm detection |
| Queued (cross-thread) | 10-100 μs | Vitals → Database |
| Blocking Queued | 100 μs - 10 ms | Worker ↔ Main (rare) |

---

## 7. Thread Safety Rules

### **7.1 Golden Rules**

1. **QObjects with signals/slots must live in ONE thread**
2. **Use Qt::QueuedConnection for cross-thread communication**
3. **Never call slots directly from another thread** (use signals)
4. **moveToThread() moves object to new thread**

### **7.2 Cross-Thread Communication Pattern**

```cpp
// Worker thread object
class DataProcessor : public QObject {
    Q_OBJECT
signals:
    void resultReady(const QByteArray& data);  // Emitted from worker thread
    
public slots:
    void processData(const QByteArray& input) {  // Called in worker thread
        // Heavy processing
        QByteArray result = process(input);
        emit resultReady(result);  // Safe: automatically queued to receiver's thread
    }
};

// Main thread setup
DataProcessor* processor = new DataProcessor();
QThread* thread = new QThread();
processor->moveToThread(thread);  // Move to worker thread

// Cross-thread connection (automatically queued)
connect(
    this, &MainWindow::startProcessing,     // Main thread signal
    processor, &DataProcessor::processData  // Worker thread slot (queued)
);

connect(
    processor, &DataProcessor::resultReady,  // Worker thread signal
    this, &MainWindow::handleResult          // Main thread slot (queued)
);

thread->start();
```

### **7.3 Common Thread Safety Mistakes**

```cpp
// ❌ BAD: Direct call from wrong thread
void MainThread::badExample() {
    workerObject->someSlot();  // CRASH if workerObject in different thread
}

// ✅ GOOD: Use signal (automatically queued)
void MainThread::goodExample() {
    emit triggerWorker();  // Signal queued to worker thread
}

// ❌ BAD: Accessing QObject from wrong thread
void WorkerThread::run() {
    mainWindowObject->setWindowTitle("Test");  // CRASH!
}

// ✅ GOOD: Use signal
signals:
    void updateTitle(const QString& title);
    
void WorkerThread::run() {
    emit updateTitle("Test");  // Queued to main thread
}
```

---

## 8. Memory Management with Signals/Slots

### **8.1 Parent-Child Ownership**

```cpp
// Parent deletes children automatically
QObject* parent = new QObject();
QObject* child = new QObject(parent);  // Child's parent is set

delete parent;  // child is automatically deleted
// Connections involving child are automatically broken
```

### **8.2 Dangling Connections**

```cpp
// ❌ BAD: Lambda captures pointer that might be deleted
QObject* temp = new QObject();
connect(sender, &Sender::signal, [temp]() {
    temp->doSomething();  // CRASH if temp deleted!
});

// ✅ GOOD: Use QPointer for safe pointer
QPointer<QObject> safeTemp = new QObject();
connect(sender, &Sender::signal, [safeTemp]() {
    if (safeTemp) {  // Check if still valid
        safeTemp->doSomething();
    }
});

// ✅ BETTER: Use receiver parameter
connect(sender, &Sender::signal, receiver, [receiver]() {
    receiver->doSomething();  // Connection auto-broken if receiver deleted
});
```

### **8.3 Cleanup Pattern**

```cpp
class MyClass : public QObject {
    Q_OBJECT
public:
    MyClass() {
        // Connections to 'this' automatically broken when destroyed
        connect(timer, &QTimer::timeout, this, &MyClass::update);
        connect(source, &Source::data, this, &MyClass::handleData);
    }
    
    ~MyClass() {
        // No need to manually disconnect if 'this' is receiver
        // Qt handles it automatically
    }
};
```

---

## 9. Z Monitor Usage Examples

### **9.1 Vital Signs Data Flow**

```cpp
// Real-time thread generates vitals
class VitalGenerator : public QObject {
    Q_OBJECT
signals:
    void vitalGenerated(const VitalRecord& vital);
};

// Database thread saves vitals
class DatabaseManager : public QObject {
    Q_OBJECT
public slots:
    void saveVital(const VitalRecord& vital);
};

// Setup (in main or service initialization)
connect(
    vitalGenerator, &VitalGenerator::vitalGenerated,  // Real-time thread
    databaseManager, &DatabaseManager::saveVital,     // Database thread
    Qt::QueuedConnection  // Explicit queued for cross-thread
);

// VitalRecord must be registered for queued connections
qRegisterMetaType<VitalRecord>();
```

### **9.2 Alarm System**

```cpp
// Direct connection for immediate alarm response
connect(
    vitalMonitor, &VitalMonitor::thresholdExceeded,
    alarmManager, &AlarmManager::triggerAlarm,
    Qt::DirectConnection  // Must be immediate (<50ms requirement)
);

// Queued connection for UI update (different thread)
connect(
    alarmManager, &AlarmManager::alarmTriggered,
    alarmView, &AlarmView::displayAlarm,
    Qt::QueuedConnection  // UI in main thread
);
```

### **9.3 Patient Admission Workflow**

```cpp
// Admission controller (UI thread)
connect(
    admissionController, &AdmissionController::admitPatientRequested,
    admissionService, &AdmissionService::admitPatient
);

// Service emits result back to controller
connect(
    admissionService, &AdmissionService::patientAdmitted,
    admissionController, &AdmissionController::onPatientAdmitted
);

// Update UI
connect(
    admissionController, &AdmissionController::patientAdmittedChanged,
    patientBanner, &PatientBanner::updatePatientInfo
);
```

---

## 10. Debugging Signals/Slots

### **10.1 Enable Connection Warnings**

```cpp
// In main.cpp
qputenv("QT_LOGGING_RULES", "qt.qml.connections=true");
```

### **10.2 Check Connection Success**

```cpp
bool success = connect(
    sender, &Sender::signal,
    receiver, &Receiver::slot
);

if (!success) {
    qWarning() << "Connection failed!";
}
```

### **10.3 List Active Connections**

```cpp
// Dump all connections for debugging
sender->dumpObjectInfo();  // Prints connections to qDebug()
```

### **10.4 Common Errors**

| Error | Cause | Solution |
|-------|-------|----------|
| "No such signal" | Typo in signal name | Use function pointer syntax |
| "No such slot" | Slot not declared in Q_OBJECT | Add `public slots:` |
| "Object deleted while signal emitted" | Dangling pointer | Use QPointer or check sender() |
| "Cannot queue arguments of type X" | Type not registered | `qRegisterMetaType<X>()` |

---

## 11. Best Practices for Z Monitor

### **DO:**

✅ **Use modern connect syntax** (function pointers)  
✅ **Explicit connection types** for cross-thread (Qt::QueuedConnection)  
✅ **Register custom types** before queued connections  
✅ **Use parent-child ownership** for automatic cleanup  
✅ **Direct connections** for real-time paths (<50ms)  
✅ **Queued connections** for cross-thread communication  
✅ **Disconnect** when no longer needed (optimization)

### **DON'T:**

❌ **Don't use string-based syntax** (SIGNAL/SLOT macros)  
❌ **Don't call slots directly from different threads**  
❌ **Don't forget qRegisterMetaType** for custom types  
❌ **Don't create connections in loops** without disconnecting  
❌ **Don't use BlockingQueuedConnection** from main thread  
❌ **Don't capture raw pointers in lambdas** (use QPointer)  
❌ **Don't emit signals with large data** (use references or pointers)

---

## 12. References

- Qt Documentation: https://doc.qt.io/qt-6/signalsandslots.html
- Qt Thread Basics: https://doc.qt.io/qt-6/thread-basics.html
- Z Monitor Thread Model: `../12_THREAD_MODEL.md`
- Z Monitor Architecture: `../02_ARCHITECTURE.md`

---

*This document covers the signals/slots mechanism used throughout Z Monitor. All inter-component communication should follow these patterns.*

