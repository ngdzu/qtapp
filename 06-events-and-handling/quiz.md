# Lesson 6 Quiz: Events and Event Handling

1. What is the primary difference between Qt's event system and the signals/slots mechanism? When would you use events instead of signals?

2. You override `mousePressEvent()` in a custom widget but forget to call `event->accept()`. What happens to the event, and how does this affect event propagation?

3. Given this code, what will happen when the user presses the Escape key?
```cpp
void MyWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "Escape pressed";
        event->ignore();
    }
}
```

4. What is the purpose of an event filter, and how does it differ from overriding event handlers directly in a widget?

5. In which order are these event handling mechanisms invoked when a key is pressed?
   - A) Widget's `keyPressEvent()`
   - B) Event filter's `eventFilter()`
   - C) Widget's `event()` function
   - D) Base class event handler

6. You want to log all mouse clicks across your entire application without modifying each widget. Which approach would you use and why?

7. What does returning `true` from an `eventFilter()` function do? Provide a practical use case for this behavior.

8. Spot the bug in this event handler:
```cpp
void MyWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    // My custom handling
    updateColor();
}
```

9. Why does this custom widget need `setFocusPolicy(Qt::StrongFocus)` to receive keyboard events?
```cpp
EventWidget::EventWidget(QWidget *parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
}
```

10. Reflection: You're building a drawing application where users click and drag to draw shapes. Which event handlers would you override, and how would you use event acceptance to prevent default behavior?
