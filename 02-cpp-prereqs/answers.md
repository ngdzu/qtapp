# Answers: Modern C++ Prerequisites for Qt

## Question 1 (Conceptual)
**Q:** What is RAII and how does Qt's parent-child ownership system leverage this C++ idiom?

**A:** RAII (Resource Acquisition Is Initialization) ties resource lifetime to object lifetime—resources are acquired in constructors and released in destructors.

Qt's parent-child system uses RAII by automatically deleting child `QObject`s when the parent is destroyed. When you create a widget with a parent (e.g., `new QPushButton("Click", &window)`), the parent's destructor deletes all children, eliminating manual memory management and preventing leaks.

---

## Question 2 (Conceptual)
**Q:** When should you use `std::unique_ptr` or `std::shared_ptr` instead of relying on Qt's parent-child ownership? Give at least one specific scenario.

**A:** Use smart pointers for non-`QObject` types, when ownership is unclear, or when objects must outlive their original scope.

For example, if you're managing a plain C++ data structure (like `std::vector<CustomData>`) that isn't a `QObject`, use `std::unique_ptr` for exclusive ownership or `std::shared_ptr` if multiple components need access. Qt parent-child only works for `QObject`-derived classes.

---

## Question 3 (Practical/Code)
**Q:** What is wrong with the following code?

```cpp
QPushButton *button = new QPushButton("Click");
QObject::connect(button, &QPushButton::clicked, []() {
    qDebug() << "Clicked!";
});
// ... application continues
```

**A:** The button has no parent, so it will never be deleted, causing a memory leak.

Without a parent, the `QPushButton` allocated on the heap has no automatic cleanup. You must either give it a parent (e.g., `new QPushButton("Click", &window)`) or manage it with a smart pointer, or manually delete it when done.

**Corrected code:**
```cpp
QWidget window;
QPushButton *button = new QPushButton("Click", &window);  // window is parent
QObject::connect(button, &QPushButton::clicked, []() {
    qDebug() << "Clicked!";
});
// button automatically deleted when window is destroyed
```

**Pitfall:** Forgetting to set a parent for heap-allocated widgets is a common memory leak source.

---

## Question 4 (Practical/Code)
**Q:** Fill in the lambda capture to make `counter` accessible and modifiable inside the lambda.

**A:** `[&counter]` or `[&]`

To modify `counter` from within the lambda, you must capture it by reference. `[&counter]` captures only `counter` by reference, while `[&]` captures all used variables by reference.

**Complete code:**
```cpp
int counter = 0;
QPushButton button("Increment");
QObject::connect(&button, &QPushButton::clicked, [&counter]() {
    counter++;
    qDebug() << "Counter:" << counter;
});
```

**Pitfall:** Using `[=]` (capture by value) would create a copy of `counter`, so modifications inside the lambda wouldn't affect the original variable.

---

## Question 5 (Practical/Code)
**Q:** What will be printed by this code? Explain the output.

```cpp
QString msg = "Hello";
QString msg2 = std::move(msg);
qDebug() << "msg:" << msg << ", msg2:" << msg2;
```

**A:** Output: `msg: , msg2: Hello` (or `msg:` may show as empty string)

After `std::move(msg)`, `msg` is in a valid but unspecified state (typically empty for `QString`). The contents are transferred to `msg2`, so `msg2` contains `"Hello"` and `msg` is empty.

**Pitfall:** Using `msg` after moving from it is safe (it's valid), but its value is unpredictable. Always treat moved-from objects as if they're default-constructed.

---

## Question 6 (Conceptual)
**Q:** Explain the difference between `[=]` and `[&]` lambda captures. When would each be appropriate in Qt signal-slot connections?

**A:** `[=]` captures all used variables by value (copying them); `[&]` captures by reference (accessing the original variables).

Use `[=]` when the lambda may outlive the captured variables (e.g., in queued connections or async operations) to avoid dangling references. Use `[&]` for short-lived lambdas or when you need to modify the original variables. In Qt, prefer `[=]` for safety unless you're certain the captured objects remain valid.

---

## Question 7 (Practical/Code)
**Q:** Is this code correct? If not, fix it:

```cpp
auto data = std::make_shared<QWidget>();
QPushButton *btn = new QPushButton("Click", data.get());
data.reset();  // Clear the shared_ptr
// btn is now used elsewhere
```

**A:** Incorrect—mixing `shared_ptr` with Qt parent-child ownership causes double-delete.

When `data.reset()` is called, the `shared_ptr` deletes the `QWidget`. However, Qt's parent-child system would try to delete `btn` again when the (already-deleted) parent is destroyed, causing undefined behavior or crash.

**Corrected code:**
```cpp
// Option 1: Use Qt parent-child only (no smart pointer)
QWidget *data = new QWidget();
QPushButton *btn = new QPushButton("Click", data);
delete data;  // Automatically deletes btn

// Option 2: If using shared_ptr, don't mix with Qt parent-child
auto data = std::make_shared<QWidget>();
QPushButton *btn = new QPushButton("Click");  // No parent
// Manage btn separately or use smart pointer for it too
```

**Pitfall:** Don't mix `shared_ptr`/`unique_ptr` with Qt parent-child for `QObject`s—choose one ownership model.

---

## Question 8 (Reflection)
**Q:** How do move semantics improve performance when returning large `QString` or `QVector` objects from functions? Can you think of a scenario in your own projects where this would matter?

**A:** Move semantics avoid copying large data structures by transferring ownership instead of duplicating memory.

When returning a `QString` or `QVector` from a function, C++ can move (transfer) the internal buffer instead of copying millions of bytes. This is critical in performance-sensitive scenarios like processing large log files, returning database query results, or building large UI data models. For example, a function that reads a 10 MB file into a `QByteArray` and returns it will move the data (fast) rather than copy it (slow).

---

## Question 9 (Conceptual)
**Q:** Why can't you copy a `std::unique_ptr`, but you can move it? What does this design enforce?

**A:** `std::unique_ptr` enforces exclusive ownership—only one pointer can own the resource at a time.

Copying would create two owners, violating uniqueness and risking double-delete. Moving transfers ownership from one `unique_ptr` to another, leaving the source empty. This design prevents accidental shared ownership and makes ownership transfer explicit in code, improving safety and clarity.

---

## Question 10 (Reflection)
**Q:** Given that Qt containers are implicitly shared (copy-on-write), when would you still want to use `std::move` with them? Consider performance and API design.

**A:** Use `std::move` when you know the source container won't be used again, avoiding even the cheap reference-count increment.

Although Qt containers are implicitly shared (copying is fast), moving is still faster because it avoids atomic reference-count operations. Use `std::move` when returning containers from functions, transferring ownership in APIs, or when the source is about to be destroyed anyway. For example:

```cpp
QStringList buildLargeList() {
    QStringList list;
    // ... fill with 10,000 items
    return list;  // Automatically moved (NRVO)
}

void processData(QStringList &&data) {
    m_cache = std::move(data);  // Explicit move, data is no longer used
}
```

This makes ownership transfer explicit and avoids unnecessary reference counting.
