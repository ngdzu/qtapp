# Lesson 27 Quiz: Performance and Profiling

1. What's the difference between QTime and QElapsedTimer for performance measurement, and which should you use?

2. In the following code, what's the performance problem?
   ```cpp
   QString result;
   for (int i = 0; i < 100000; ++i) {
       result += QString::number(i) + ",";
   }
   ```

3. How does Qt's implicit sharing (copy-on-write) affect performance when passing containers to functions?

4. What's the approximate overhead of a Qt signal/slot connection compared to a direct function call, and when does it matter?

5. What happens when you call `update()` on a widget 100 times in a loop? How can you optimize this?

6. Why is this code a memory leak, and how do you fix it?
   ```cpp
   void MyWidget::addButton() {
       QPushButton *btn = new QPushButton("Click");
       layout()->addWidget(btn);
   }
   ```

7. What's the difference between Qt::DirectConnection and Qt::QueuedConnection for signal/slot performance?

8. How much faster can Release builds be compared to Debug builds in Qt, and why?

9. What profiling tool would you use to detect memory leaks on Linux, and what Qt-specific pattern helps prevent them?

10. If your Qt GUI application becomes unresponsive during a long computation, what's the likely cause and solution?
