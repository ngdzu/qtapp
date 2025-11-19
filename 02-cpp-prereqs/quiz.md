# Quiz: Modern C++ Prerequisites for Qt

## Question 1 (Conceptual)
What is RAII and how does Qt's parent-child ownership system leverage this C++ idiom?

## Question 2 (Conceptual)
When should you use `std::unique_ptr` or `std::shared_ptr` instead of relying on Qt's parent-child ownership? Give at least one specific scenario.

## Question 3 (Practical/Code)
What is wrong with the following code?

```cpp
QPushButton *button = new QPushButton("Click");
QObject::connect(button, &QPushButton::clicked, []() {
    qDebug() << "Clicked!";
});
// ... application continues
```

## Question 4 (Practical/Code)
Fill in the lambda capture to make `counter` accessible and modifiable inside the lambda:

```cpp
int counter = 0;
QPushButton button("Increment");
QObject::connect(&button, &QPushButton::clicked, /* ??? */() {
    counter++;
    qDebug() << "Counter:" << counter;
});
```

## Question 5 (Practical/Code)
What will be printed by this code? Explain the output.

```cpp
QString msg = "Hello";
QString msg2 = std::move(msg);
qDebug() << "msg:" << msg << ", msg2:" << msg2;
```

## Question 6 (Conceptual)
Explain the difference between `[=]` and `[&]` lambda captures. When would each be appropriate in Qt signal-slot connections?

## Question 7 (Practical/Code)
Is this code correct? If not, fix it:

```cpp
auto data = std::make_shared<QWidget>();
QPushButton *btn = new QPushButton("Click", data.get());
data.reset();  // Clear the shared_ptr
// btn is now used elsewhere
```

## Question 8 (Reflection)
How do move semantics improve performance when returning large `QString` or `QVector` objects from functions? Can you think of a scenario in your own projects where this would matter?

## Question 9 (Conceptual)
Why can't you copy a `std::unique_ptr`, but you can move it? What does this design enforce?

## Question 10 (Reflection)
Given that Qt containers are implicitly shared (copy-on-write), when would you still want to use `std::move` with them? Consider performance and API design.
