# Lesson 27 Quiz Answers: Performance and Profiling

## 1. What's the difference between QTime and QElapsedTimer for performance measurement, and which should you use?

**Answer:** QElapsedTimer is monotonic and immune to system clock changes; always use it for measurements.

**Explanation:** QTime can give incorrect results if the system clock changes (daylight saving time, NTP adjustments, user changing time). QElapsedTimer uses a monotonic clock that only moves forward:

```cpp
// Bad: Can give negative/wrong results if clock changes
QTime time = QTime::currentTime();
// ... code to measure ...
int elapsed = time.elapsed(); // May be wrong!

// Good: Always accurate
QElapsedTimer timer;
timer.start();
// ... code to measure ...
qint64 elapsed = timer.elapsed(); // Guaranteed accurate
```

QElapsedTimer also provides nanosecond precision on platforms that support it (`nsecsElapsed()`), making it ideal for micro-benchmarks.

## 2. In the following code, what's the performance problem?

```cpp
QString result;
for (int i = 0; i < 100000; ++i) {
    result += QString::number(i) + ",";
}
```

**Answer:** Causes 100,000 reallocations; use `reserve()` to pre-allocate.

**Explanation:** Each `+=` operation may reallocate the QString's internal buffer, causing quadratic time complexity O(n²). The fix:

```cpp
// Optimized version
QString result;
result.reserve(100000 * 8); // Pre-allocate (estimate ~8 chars per number)
for (int i = 0; i < 100000; ++i) {
    result += QString::number(i) + ",";
}
```

Or even better, use QStringList and join:

```cpp
QStringList parts;
parts.reserve(100000);
for (int i = 0; i < 100000; ++i) {
    parts << QString::number(i);
}
QString result = parts.join(","); // Single allocation
```

This can be 10-100x faster for large loops. The lesson: pre-allocate when you know the size.

## 3. How does Qt's implicit sharing (copy-on-write) affect performance when passing containers to functions?

**Answer:** Makes copies cheap until modification; pass by const reference to avoid detachment.

**Explanation:** Qt containers use copy-on-write (COW). Copying is cheap (just increments a reference count), but modifying triggers a deep copy (detachment):

```cpp
QVector<int> vec1;
vec1 << 1 << 2 << 3;

QVector<int> vec2 = vec1; // Cheap: O(1), shares data

vec2.append(4); // Expensive: O(n), detaches and copies all data
```

**Best practices:**

```cpp
// Good: No detachment, very fast
void processData(const QVector<int> &data) {
    for (int value : data) { ... }
}

// Bad: May cause unnecessary detachment on read
void processData(QVector<int> &data) {
    for (int value : data) { ... } // Non-const access can detach!
}

// Terrible: Always copies
void processData(QVector<int> data) { ... }
```

Pass by const reference unless you need to modify. This applies to QString, QList, QVector, QHash, etc.

## 4. What's the approximate overhead of a Qt signal/slot connection compared to a direct function call, and when does it matter?

**Answer:** ~10-50 nanoseconds overhead; matters only in tight loops (millions of calls).

**Explanation:** Signal/slot connections add minimal overhead:

- **Direct function call**: ~1-5 ns
- **Signal/slot (direct connection)**: ~10-50 ns
- **Signal/slot (queued connection)**: ~100-1000 ns (event loop overhead)

Example overhead:

```cpp
// Direct call: ~5 ns
receiver->processData(data);

// Signal/slot: ~30 ns
emit dataReady(data);

// Queued signal: ~500 ns (posted to event loop)
emit dataReady(data); // Qt::QueuedConnection
```

**When it matters:** In tight loops processing millions of items per second. For normal UI events (button clicks, network responses), the overhead is negligible.

**Rule of thumb:** If you're calling something < 1000 times per second, use signals/slots for flexibility. If calling millions of times per second in a hot loop, use direct calls.

## 5. What happens when you call `update()` on a widget 100 times in a loop? How can you optimize this?

**Answer:** Qt coalesces updates into one repaint; use `setUpdatesEnabled(false)` for explicit control.

**Explanation:** Qt automatically coalesces multiple `update()` calls into a single repaint at the next event loop iteration. However, you can optimize further:

```cpp
// Inefficient (but Qt coalesces most repaints anyway)
for (int i = 0; i < 100; ++i) {
    widget->setText(QString::number(i));
    widget->update(); // All coalesced into 1 repaint
}

// More efficient: Explicit control
widget->setUpdatesEnabled(false);
for (int i = 0; i < 100; ++i) {
    widget->setText(QString::number(i));
}
widget->setUpdatesEnabled(true); // Triggers single repaint
```

**Even better** for complex updates:

```cpp
widget->setUpdatesEnabled(false);
// ... multiple changes ...
widget->setUpdatesEnabled(true);
widget->update(); // Explicit single update
```

This is especially important when updating multiple widgets or modifying layouts, as it prevents intermediate layout calculations.

## 6. Why is this code a memory leak, and how do you fix it?

```cpp
void MyWidget::addButton() {
    QPushButton *btn = new QPushButton("Click");
    layout()->addWidget(btn);
}
```

**Answer:** No parent specified; fix by passing `this` as parent.

**Explanation:** The button is allocated but has no parent QObject. Even though it's added to the layout, layouts don't own widgets by default. The fix:

```cpp
// Correct: Widget owned by parent
void MyWidget::addButton() {
    QPushButton *btn = new QPushButton("Click", this); // Parent = this
    layout()->addWidget(btn);
    // btn automatically deleted when MyWidget destroyed
}
```

**How it works:** When `MyWidget` is destroyed, Qt's object tree automatically deletes all children (the button). This is Qt's primary memory management pattern.

**Alternative** using layout ownership (also works):

```cpp
QPushButton *btn = new QPushButton("Click");
layout()->addWidget(btn);
// Layout takes ownership when added to a widget with a parent
```

But explicit parenting is clearer and works even before adding to layout.

## 7. What's the difference between Qt::DirectConnection and Qt::QueuedConnection for signal/slot performance?

**Answer:** DirectConnection calls immediately (~30ns), QueuedConnection posts to event loop (~500ns).

**Explanation:**

**Qt::DirectConnection** (default for same-thread):
- Slot called immediately, like a function call
- Overhead: ~10-50 ns
- Blocking: sender waits for slot to complete
- Use when: Same thread, need immediate execution

**Qt::QueuedConnection** (default for cross-thread):
- Slot posted to receiver's event loop
- Overhead: ~100-1000 ns
- Non-blocking: sender continues immediately
- Use when: Cross-thread communication, need async execution

```cpp
// Direct: Slot runs immediately
connect(sender, &Sender::signal, receiver, &Receiver::slot,
        Qt::DirectConnection);
emit mySignal(); // Blocks until slot completes

// Queued: Slot runs later in event loop
connect(sender, &Sender::signal, receiver, &Receiver::slot,
        Qt::QueuedConnection);
emit mySignal(); // Returns immediately, slot runs later
```

**Auto connection** (default) chooses based on thread:
- Same thread → DirectConnection
- Different threads → QueuedConnection

For performance-critical same-thread code, DirectConnection is 10-20x faster than QueuedConnection.

## 8. How much faster can Release builds be compared to Debug builds in Qt, and why?

**Answer:** 5-20x faster due to compiler optimizations and disabled debug code.

**Explanation:**

**Debug builds** include:
- No compiler optimization (-O0)
- Debug symbols (increases binary size)
- Q_ASSERT macros enabled
- Verbose logging overhead
- Bounds checking in some containers
- No function inlining

**Release builds** include:
- Full optimization (-O2 or -O3)
- Debug symbols stripped
- Q_ASSERT compiled out
- Minimal logging
- Aggressive inlining

**Typical speedup examples:**
- String operations: 5-10x faster
- Math-heavy code: 10-20x faster
- Container operations: 3-7x faster
- Overall application: 5-15x faster

```cpp
// In Debug: Runs every iteration
Q_ASSERT(index < size());
value = data[index];

// In Release: Q_ASSERT removed by preprocessor
value = data[index];
```

**Always profile in Release mode** for realistic performance measurements. Debug mode performance is not representative of production.

## 9. What profiling tool would you use to detect memory leaks on Linux, and what Qt-specific pattern helps prevent them?

**Answer:** Valgrind (memcheck tool); Qt's parent-child QObject ownership prevents leaks.

**Explanation:**

**Valgrind for leak detection:**

```bash
# Run your Qt app through Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./myapp

# Common Valgrind output:
# definitely lost: 1,024 bytes (real leaks)
# indirectly lost: 512 bytes (leaked via other leaks)
# possibly lost: 256 bytes (may be false positives)
# still reachable: 2,048 bytes (not freed at exit, but tracked)
```

**Qt's prevention pattern:**

```cpp
// Correct: Parent-child ownership
QWidget *parent = new QWidget();
QPushButton *btn = new QPushButton(parent); // btn owned by parent
QLabel *label = new QLabel(parent); // label owned by parent

delete parent; // Automatically deletes btn and label
```

**Other tools:**
- **Heaptrack** (Linux): Heap memory profiler
- **Instruments** (macOS): Leaks and allocations
- **Dr. Memory** (Windows/Linux): Similar to Valgrind
- **AddressSanitizer** (Clang/GCC): Compile-time leak detection

**Best practice:** Combine Valgrind with Qt's parent-child pattern. Let Qt manage QObject lifetimes, use smart pointers (QScopedPointer, QSharedPointer) for non-QObjects.

## 10. If your Qt GUI application becomes unresponsive during a long computation, what's the likely cause and solution?

**Answer:** Blocking the event loop; move computation to QThread or use QTimer for chunking.

**Explanation:** Qt's event loop must run continuously to process UI events. Long computations block it, freezing the GUI.

**Problem:**

```cpp
void MyWidget::onButtonClicked() {
    // BAD: Blocks event loop for 10 seconds
    for (int i = 0; i < 10000000; ++i) {
        heavyComputation();
    }
    updateUI(); // UI frozen until this completes
}
```

**Solution 1: Move to QThread**

```cpp
void MyWidget::onButtonClicked() {
    QThread *thread = QThread::create([] {
        for (int i = 0; i < 10000000; ++i) {
            heavyComputation();
        }
    });
    connect(thread, &QThread::finished, this, &MyWidget::updateUI);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start(); // UI stays responsive
}
```

**Solution 2: Chunk with QTimer**

```cpp
void MyWidget::onButtonClicked() {
    int i = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() mutable {
        // Process chunk
        for (int j = 0; j < 1000 && i < 10000000; ++j, ++i) {
            heavyComputation();
        }
        
        if (i >= 10000000) {
            timer->stop();
            timer->deleteLater();
            updateUI();
        }
    });
    timer->start(0); // Process chunks between event loop iterations
}
```

**Solution 3: QtConcurrent**

```cpp
QFuture<void> future = QtConcurrent::run([] {
    for (int i = 0; i < 10000000; ++i) {
        heavyComputation();
    }
});
// Connect QFutureWatcher for completion notification
```

**Rule:** Never block the event loop for > 100ms. For longer operations, use threads or chunking.
