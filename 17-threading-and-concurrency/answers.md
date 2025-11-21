# Lesson 17 Quiz Answers

1. **Difference between QThread and QtConcurrent?**

QThread gives explicit control over threads and event loops. QtConcurrent provides high-level functions for parallel operations.

Use QtConcurrent for simple parallel tasks. Use QThread when you need event loops, continuous background work, or fine-grained control.

2. **Why must GUI updates happen on the main thread?**

Qt's GUI classes are not thread-safe by design for performance.

All QWidget operations must occur on the thread where they were created (the main thread). Cross-thread GUI updates can cause crashes or undefined behavior.

3. **How do you safely communicate between threads?**

Use Qt signals and slots with queued connections:
```cpp
connect(worker, &Worker::dataReady, 
        this, &MainWindow::handleData,
        Qt::QueuedConnection);
```
Qt handles the thread synchronization automatically.

4. **What does `moveToThread()` do?**

Moves a QObject and its children to another thread's event loop.

After moving, the object's slots will execute in the new thread when signals are emitted. The object must not have a parent.

5. **How do you wait for a QFuture to complete?**

Call `waitForFinished()` or `result()`:
```cpp
future.waitForFinished(); // Blocks until done
int value = future.result(); // Also blocks
```
Or use QFutureWatcher for non-blocking notifications.

6. **What happens if you call `label->setText()` from a worker thread?**

Undefined behavior - likely a crash or visual corruption.

Qt may print warnings like "QObject: Cannot create children for a parent in different thread." Always emit signals to update GUI from worker threads.

7. **When should you use QThread vs QtConcurrent?**

Use QtConcurrent for: CPU-intensive computations, parallel processing of collections.

Use QThread for: Event-driven work, networking, continuous background tasks, when you need signals/slots in the worker.

8. **How do you pass data from a worker thread back to the main thread?**

Emit signals with data:
```cpp
// Worker
emit resultReady(data);

// Main thread
connect(worker, &Worker::resultReady,
        this, &MainWindow::handleResult);
```

9. **What is a race condition?**

When multiple threads access shared data concurrently, and the outcome depends on timing.

Example: Thread A reads `counter=5`, Thread B reads `counter=5`, both increment to 6 and write back - lost update! Use QMutex to prevent this.

10. **How do you protect shared data between threads?**

Use QMutex for exclusive access:
```cpp
QMutex mutex;
mutex.lock();
sharedData = newValue;
mutex.unlock();
```
Or use QMutexLocker for automatic unlocking.
