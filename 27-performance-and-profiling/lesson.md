# Lesson 27: Performance and Profiling

## Learning Goals
- Measure Qt application performance using QElapsedTimer
- Profile memory usage and identify leaks
- Optimize rendering and event handling
- Use Qt's profiling tools and best practices
- Understand common performance bottlenecks

## Introduction

Performance optimization is crucial for creating responsive Qt applications. This lesson covers profiling techniques to identify bottlenecks, memory profiling to detect leaks, and Qt-specific optimizations. You'll learn to use QElapsedTimer for precise timing, understand when to use signals/slots vs direct calls, and optimize rendering performance.

## Key Concepts

**QElapsedTimer for Precision Timing**

QElapsedTimer provides high-resolution timing for performance measurements:

```cpp
QElapsedTimer timer;
timer.start();

// Code to measure
for (int i = 0; i < 1000000; ++i) {
    QString s = QString::number(i);
}

qint64 elapsed = timer.elapsed();
qDebug() << "Elapsed:" << elapsed << "ms";
```

QElapsedTimer is more accurate than QTime for performance measurements and works correctly across time zone changes and system clock adjustments.

**Memory Profiling with QObject Parent-Child**

Qt's parent-child ownership prevents memory leaks automatically:

```cpp
// Good: Child deleted automatically when parent destroyed
QWidget *parent = new QWidget();
QPushButton *btn = new QPushButton(parent); // parent owns btn
delete parent; // btn automatically deleted

// Bad: Memory leak
QPushButton *btn2 = new QPushButton(); // No parent
// btn2 never deleted = leak!
```

For non-QObjects, use smart pointers (QScopedPointer, QSharedPointer) or containers.

**Signal/Slot Performance**

Direct connections are faster than queued connections:

```cpp
// Fast: Direct call (same thread)
connect(sender, &Sender::signal, receiver, &Receiver::slot);

// Slower: Queued (cross-thread, posted to event loop)
connect(sender, &Sender::signal, receiver, &Receiver::slot, 
        Qt::QueuedConnection);

// Fastest: Direct function call (no signal overhead)
receiver->slot(); // But loses signal/slot benefits
```

Rule of thumb: Signals/slots add ~10-50ns overhead. Use direct calls only in tight loops where this matters.

**Optimize Rendering with Update Coalescing**

Avoid excessive repaints by coalescing updates:

```cpp
// Bad: Triggers 100 repaints
for (int i = 0; i < 100; ++i) {
    widget->setText(QString::number(i));
    widget->update(); // Repaint each iteration
}

// Good: Triggers 1 repaint
widget->setUpdatesEnabled(false);
for (int i = 0; i < 100; ++i) {
    widget->setText(QString::number(i));
}
widget->setUpdatesEnabled(true);
```

**QString Optimization**

Use QString::reserve() and avoid unnecessary copies:

```cpp
// Slow: Multiple allocations
QString result;
for (int i = 0; i < 10000; ++i) {
    result += QString::number(i); // Reallocates every iteration
}

// Fast: Pre-allocate, use QStringBuilder
QString result;
result.reserve(50000); // Allocate once
for (int i = 0; i < 10000; ++i) {
    result += QString::number(i);
}

// Faster: Use QStringBuilder with %
result = QString::number(1) % ", " % QString::number(2);
```

**Container Detachment and Implicit Sharing**

Qt containers use copy-on-write (COW):

```cpp
QList<int> list1;
list1 << 1 << 2 << 3;

QList<int> list2 = list1; // Cheap: No copy, shares data

list2.append(4); // Detaches: Now list2 has its own copy
```

Pass containers by const reference to avoid detachment:

```cpp
// Good
void processData(const QVector<int> &data) { ... }

// Bad: May cause unnecessary detachment
void processData(QVector<int> data) { ... }
```

## Example Walkthrough

The demo application measures performance of common Qt operations:

1. **String operations** - QString construction, concatenation, reserve()
2. **Container operations** - QVector append, insert, lookup
3. **Rendering performance** - Widget updates with/without coalescing
4. **Memory profiling** - QObject allocation with proper cleanup

Each benchmark runs multiple iterations and reports average time in microseconds or milliseconds. This demonstrates real-world performance differences between approaches.

## Expected Output

A Qt application that displays:
- Benchmark results for string operations (construction, concatenation, reserve)
- Container performance (append, prepend, insert, find)
- Rendering benchmarks (update coalescing effects)
- Memory allocation patterns with QObject parent-child
- Comparison of optimized vs unoptimized code paths

You'll see concrete numbers showing optimized code running 2-10x faster than naive implementations.

## Try It

1. **Run the benchmarks** - Observe timing differences between operations
2. **Modify iteration counts** - See how performance scales
3. **Add your own benchmarks** - Measure your algorithms
4. **Compare Debug vs Release** - Rebuild in Release mode to see optimizer impact
5. **Profile with external tools** - Try Valgrind (Linux) or Instruments (macOS)
6. **Monitor memory** - Use Task Manager/Activity Monitor while running benchmarks

## Key Takeaways

- **QElapsedTimer** provides microsecond-precision timing for performance measurements
- **Memory management** in Qt leverages parent-child relationships to prevent leaks
- **Signal/slot overhead** is minimal (~10-50ns) but matters in tight loops
- **QString::reserve()** dramatically improves performance when building large strings
- **Update coalescing** prevents excessive repaints by batching widget updates
- **Implicit sharing** (COW) makes Qt container copies cheap until modification
- **Always profile before optimizing** - measure to find real bottlenecks
- **Release builds** can be 5-20x faster than Debug builds due to optimization
