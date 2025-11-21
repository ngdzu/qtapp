# Lesson 17: Threading and Concurrency

## Learning Goals
- Understand Qt's threading model and event loop architecture
- Use QThread for background tasks with proper worker pattern
- Master thread-safe communication with signals/slots and QMetaObject
- Leverage QtConcurrent for parallel operations (map, filter, reduce)
- Understand QMutex, QReadWriteLock, QSemaphore, and QWaitCondition
- Use QThreadPool and QRunnable for efficient thread management
- Avoid race conditions, deadlocks, and common threading pitfalls

## Introduction

Qt provides a comprehensive threading framework that goes beyond simple thread creation. The framework includes QThread for explicit thread management, QtConcurrent for high-level parallel operations, synchronization primitives (QMutex, QSemaphore), and thread pool management. Threading allows long-running tasks to execute without freezing the UI, which is essential for responsive applications.

Qt's threading model is built around the event loop - each thread can have its own event loop, and signals/slots work across threads safely through queued connections. This makes Qt's threading model safer and more elegant than raw pthread or std::thread approaches.

## Key Concepts

### QThread and Worker Pattern

The recommended approach is to create a worker object and move it to a thread:

```cpp
class Worker : public QObject {
    Q_OBJECT
public slots:
    void process() {
        // Heavy work here
        emit finished();
    }
signals:
    void finished();
};

QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::process);
connect(worker, &Worker::finished, thread, &QThread::quit);
connect(thread, &QThread::finished, thread, &QThread::deleteLater);

thread->start();
```

**Never subclass QThread** unless you need a custom event loop. The worker pattern is safer and more flexible.

### QtConcurrent - High-Level Parallelism

QtConcurrent provides map/filter/reduce operations on containers:

```cpp
// Parallel map - transform each element
QList<int> list = {1, 2, 3, 4, 5};
QFuture<int> future = QtConcurrent::mapped(list, [](int x) {
    return x * x;
});
QList<int> results = future.results(); // [1, 4, 9, 16, 25]

// Parallel filter - select elements
QFuture<int> filtered = QtConcurrent::filtered(list, [](int x) {
    return x % 2 == 0;
}); // Returns [2, 4]

// Parallel reduce - combine elements
int sum = QtConcurrent::mappedReduced(list,
    [](int x) { return x * 2; },
    [](int &result, int x) { result += x; }
);

// Run arbitrary function
QFuture<int> future = QtConcurrent::run([]() {
    return heavyComputation();
});
```

### QThreadPool and QRunnable

For managing multiple short-lived tasks, use QThreadPool:

```cpp
class Task : public QRunnable {
    void run() override {
        // Task work here
    }
};

QThreadPool::globalInstance()->start(new Task);
```

QThreadPool automatically reuses threads and limits concurrency to the ideal thread count (usually CPU cores).

### Synchronization Primitives

**QMutex** - Mutual exclusion lock:

```cpp
QMutex mutex;

void threadSafeFunction() {
    QMutexLocker locker(&mutex);
    // Protected code - only one thread at a time
} // Automatically unlocks
```

**QReadWriteLock** - Multiple readers OR single writer:

```cpp
QReadWriteLock lock;

void readData() {
    QReadLocker locker(&lock);
    // Multiple readers can access simultaneously
}

void writeData() {
    QWriteLocker locker(&lock);
    // Exclusive write access
}
```

**QSemaphore** - Resource counting:

```cpp
QSemaphore semaphore(5); // Max 5 resources

void useResource() {
    semaphore.acquire();
    // Use resource
    semaphore.release();
}
```

**QWaitCondition** - Wait for condition:

```cpp
QMutex mutex;
QWaitCondition condition;
bool dataReady = false;

// Producer thread
void produce() {
    QMutexLocker locker(&mutex);
    dataReady = true;
    condition.wakeOne();
}

// Consumer thread
void consume() {
    QMutexLocker locker(&mutex);
    while (!dataReady)
        condition.wait(&mutex);
    // Process data
}
```

### Thread-Safe Communication

**Cross-thread signals** automatically use queued connections:

```cpp
// Signal emitted in thread B, slot runs in thread A
connect(workerObject, &Worker::dataReady,
        mainObject, &Main::processData);
```

**QMetaObject::invokeMethod** for explicit invocation:

```cpp
// Run method in target object's thread
QMetaObject::invokeMethod(object, "methodName",
    Qt::QueuedConnection,
    Q_ARG(int, value));
```

**Thread-local storage** with QThreadStorage:

```cpp
QThreadStorage<QCache<QString, QPixmap>> caches;

void useCache() {
    if (!caches.hasLocalData())
        caches.setLocalData(new QCache<QString, QPixmap>);
    QCache<QString, QPixmap> *cache = caches.localData();
}
```

## Example Walkthrough

Our demo shows both QThread worker pattern and QtConcurrent in action:

**QThread Example:** When you click "Start QThread Task", a Worker object running in a background thread performs a simulated long task. It emits `progressChanged` signals that safely update the UI progress bar. Notice the UI counter keeps updating - proof the main thread isn't blocked.

**QtConcurrent Example:** Clicking "Start QtConcurrent Task" runs a heavy computation (summing 100 million numbers) using `QtConcurrent::run()`. A QFutureWatcher monitors completion and displays the result when done. Again, the UI remains responsive.

Both approaches demonstrate the cardinal rule: **never block the main thread**. Long operations must run in background threads, with results communicated back via signals or futures.

## Common Pitfalls

**Race Conditions:** Two threads accessing shared data without synchronization:
```cpp
// BAD: Race condition
int counter = 0;
void increment() { counter++; } // Not atomic!

// GOOD: Protected with mutex
QMutex mutex;
int counter = 0;
void increment() {
    QMutexLocker locker(&mutex);
    counter++;
}
```

**Deadlocks:** Two threads waiting for each other's locks:
```cpp
// BAD: Can deadlock
Thread1: lock(A); lock(B);
Thread2: lock(B); lock(A);

// GOOD: Always lock in same order
Thread1: lock(A); lock(B);
Thread2: lock(A); lock(B);
```

**GUI Access from Worker Thread:** Always crashes or corrupts:
```cpp
// BAD: Accessing widgets from worker thread
void Worker::run() {
    label->setText("Done"); // CRASH!
}

// GOOD: Emit signal, update in main thread
void Worker::run() {
    emit finished("Done");
}
// In main thread:
connect(worker, &Worker::finished, label, &QLabel::setText);
```

**Forgetting to Start Thread:**
```cpp
thread->start(); // Don't forget this!
```

**Using moveToThread on Objects with Parent:** Objects with parents can't be moved:
```cpp
Worker *worker = new Worker(this); // Has parent
worker->moveToThread(thread); // ERROR!

Worker *worker = new Worker; // No parent - OK
worker->moveToThread(thread); // Works
```

## Try It

1. Build and run the application
2. Watch the UI counter increment continuously
3. Click "Start QThread Task" - observe:
   - Progress bar updates smoothly
   - UI counter keeps running (thread doesn't block UI)
   - Status changes from "Starting" to "Working" to "Completed"
4. Click "Start QtConcurrent Task" - observe:
   - Progress spinner appears
   - Heavy computation happens in background
   - Result appears when complete (sum of 1 to 100 million)
5. Try clicking both buttons while tasks are running
6. Notice the window remains draggable and responsive throughout

## Key Takeaways

- **QThread worker pattern** is the recommended approach - create QObject-derived workers and use `moveToThread()`
- **QtConcurrent** provides high-level parallelism (map, filter, reduce, run) - simpler for data-parallel tasks
- **QThreadPool/QRunnable** efficiently manages short-lived concurrent tasks
- **Synchronization primitives** (QMutex, QReadWriteLock, QSemaphore, QWaitCondition) prevent race conditions
- **Signals/slots work across threads** automatically via queued connections - Qt's killer feature for threading
- **Never access GUI from worker threads** - always use signals or QMetaObject::invokeMethod
- **QMutexLocker, QReadLocker, QWriteLocker** ensure locks are released (RAII pattern)
- **Thread affinity** determines which thread's event loop processes an object's events
- **QThread::idealThreadCount()** tells you optimal parallelism for the CPU
- **Race conditions and deadlocks** are the primary threading bugs - test thoroughly under load
