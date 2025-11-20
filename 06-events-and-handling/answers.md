# Lesson 6 Quiz Answers: Events and Event Handling

## 1. What is the primary difference between Qt's event system and the signals/slots mechanism? When would you use events instead of signals?

**Answer:** Events handle low-level system interactions (mouse, keyboard, paint), while signals/slots handle high-level object communication.

**Explanation:** Events represent notifications from the operating system or Qt framework about user input, window changes, or system requests. Signals and slots are a higher-level abstraction for object-to-object communication. Use events when you need to respond to low-level input (like custom mouse behavior or painting) or intercept default widget behavior. Use signals/slots for application-level communication where objects need to notify each other of state changes.

## 2. You override `mousePressEvent()` in a custom widget but forget to call `event->accept()`. What happens to the event, and how does this affect event propagation?

**Answer:** The event is accepted by default in specific event handlers like `mousePressEvent()`.

**Explanation:** Unlike the general `event()` function, specific event handlers like `mousePressEvent()`, `keyPressEvent()`, etc., automatically accept events. Not calling `accept()` explicitly is fine—the event won't propagate to the parent. However, if you want the event to propagate (unusual for mouse/key events), you must explicitly call `event->ignore()`. This design makes common event handling simpler while still allowing propagation when needed.

## 3. Given this code, what will happen when the user presses the Escape key?

```cpp
void MyWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "Escape pressed";
        event->ignore();
    }
}
```

**Answer:** The message "Escape pressed" is logged, then the event propagates to the parent widget.

**Explanation:** Calling `event->ignore()` marks the event as unhandled, allowing it to propagate to the parent widget. This is useful when you want to monitor or log an event but still let parent widgets respond to it. The parent widget's `keyPressEvent()` will receive the Escape key event and can handle it (e.g., closing a dialog). If you wanted to prevent propagation, you would call `event->accept()` instead.

**Corrected code (to handle and stop propagation):**
```cpp
void MyWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "Escape pressed";
        close();  // Handle the event
        event->accept();  // Stop propagation
    }
}
```

## 4. What is the purpose of an event filter, and how does it differ from overriding event handlers directly in a widget?

**Answer:** Event filters intercept events before they reach the target widget, enabling monitoring across multiple objects.

**Explanation:** Event filters are installed on objects using `installEventFilter()` and receive events before the target widget's handlers. This allows you to:
- Monitor events across multiple widgets without modifying each one
- Implement global keyboard shortcuts or input validation
- Log or intercept events system-wide

Overriding event handlers only affects that specific widget class. Event filters are more flexible: one filter can monitor many objects, and multiple filters can be chained. Filters can also block events by returning `true`, preventing the target from ever seeing them.

## 5. In which order are these event handling mechanisms invoked when a key is pressed?

**Answer:** B → C → A → D (Event filter → Widget's `event()` → Widget's `keyPressEvent()` → Base class handler)

**Explanation:** Qt's event delivery follows this precise order:
1. **Event filters** (installed with `installEventFilter()`) get first chance to intercept
2. Widget's **`event()` function** receives the event and dispatches to specific handlers
3. Specific handler like **`keyPressEvent()`** processes the event
4. If not overridden, **base class implementation** handles it

Event filters can block events (return `true`) before widgets ever see them. The `event()` function can handle events before dispatching to specific handlers. This hierarchy provides flexibility: filters for global monitoring, `event()` for general handling, and specific handlers for targeted behavior.

## 6. You want to log all mouse clicks across your entire application without modifying each widget. Which approach would you use and why?

**Answer:** Install an event filter on the QApplication object.

**Explanation:** Installing an event filter on `QApplication` (the global application object) allows you to intercept all events for all widgets in your application:

```cpp
class GlobalClickLogger : public QObject {
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            qDebug() << "Click in" << obj->objectName() << "at" << mouseEvent->pos();
        }
        return false;  // Don't block events
    }
};

// In main():
app.installEventFilter(new GlobalClickLogger(&app));
```

This approach requires no modifications to individual widgets and can be enabled/disabled easily. It's ideal for debugging, analytics, or implementing application-wide input policies.

## 7. What does returning `true` from an `eventFilter()` function do? Provide a practical use case for this behavior.

**Answer:** Returning `true` blocks the event from reaching the target widget.

**Explanation:** When `eventFilter()` returns `true`, the event is considered fully handled and is not delivered to the target widget or its event handlers. This is powerful for implementing input validation or restrictions:

**Practical use case:** Numeric-only input field:
```cpp
class NumericFilter : public QObject {
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (!keyEvent->text().isEmpty() && !keyEvent->text()[0].isDigit()) {
                return true;  // Block non-numeric keys
            }
        }
        return false;
    }
};

lineEdit->installEventFilter(new NumericFilter(lineEdit));
```

This blocks non-numeric characters from reaching the QLineEdit, enforcing numeric-only input without modifying the QLineEdit itself.

## 8. Spot the bug in this event handler:

```cpp
void MyWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    // My custom handling
    updateColor();
}
```

**Answer:** No bug—calling the base class implementation is safe but usually unnecessary for `mousePressEvent()`.

**Explanation:** This code will work correctly. Unlike some event handlers where the base implementation performs critical work (like `paintEvent()` or `resizeEvent()`), `QWidget::mousePressEvent()` does very little—it just accepts the event. However, if the base class's implementation is important (like in QAbstractButton where it triggers signals), you should call it. The typical pattern is to handle your custom logic first, then decide whether to call the base implementation or accept/ignore the event:

**Better pattern:**
```cpp
void MyWidget::mousePressEvent(QMouseEvent *event) {
    // Custom handling first
    updateColor();
    event->accept();  // Explicit acceptance (though automatic for mouse events)
    
    // Only call base if you need its behavior
    // QWidget::mousePressEvent(event);
}
```

For most custom widgets inheriting directly from QWidget, calling the base `mousePressEvent()` is optional.

## 9. Why does this custom widget need `setFocusPolicy(Qt::StrongFocus)` to receive keyboard events?

```cpp
EventWidget::EventWidget(QWidget *parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
}
```

**Answer:** Widgets need keyboard focus to receive key events; the default focus policy for QWidget is `Qt::NoFocus`.

**Explanation:** Qt's keyboard focus system determines which widget receives key events. By default, plain `QWidget` objects have `Qt::NoFocus`, meaning they won't accept keyboard focus and thus won't receive key events. `Qt::StrongFocus` enables the widget to receive focus both by clicking (mouse) and by pressing Tab (keyboard navigation).

Focus policies:
- `Qt::NoFocus` - Never receives focus (default for QWidget)
- `Qt::TabFocus` - Focus via Tab key only
- `Qt::ClickFocus` - Focus via mouse click only
- `Qt::StrongFocus` - Focus via Tab or click (most common for interactive widgets)
- `Qt::WheelFocus` - Also includes mouse wheel

Without `setFocusPolicy(Qt::StrongFocus)`, `keyPressEvent()` would never be called because the widget can't receive keyboard focus.

## 10. Reflection: You're building a drawing application where users click and drag to draw shapes. Which event handlers would you override, and how would you use event acceptance to prevent default behavior?

**Answer:** Override `mousePressEvent()`, `mouseMoveEvent()`, `mouseReleaseEvent()`, and `paintEvent()`; accept events to prevent propagation.

**Explanation:** For a drawing application, you need these event handlers:

1. **`mousePressEvent()`**: Record starting point of drag
   ```cpp
   void DrawWidget::mousePressEvent(QMouseEvent *event) {
       m_startPoint = event->pos();
       m_drawing = true;
       event->accept();  // Handle locally, don't propagate
   }
   ```

2. **`mouseMoveEvent()`**: Track current mouse position while dragging
   ```cpp
   void DrawWidget::mouseMoveEvent(QMouseEvent *event) {
       if (m_drawing) {
           m_currentPoint = event->pos();
           update();  // Trigger repaint
           event->accept();
       }
   }
   ```

3. **`mouseReleaseEvent()`**: Finalize the shape
   ```cpp
   void DrawWidget::mouseReleaseEvent(QMouseEvent *event) {
       m_drawing = false;
       saveShape(m_startPoint, event->pos());
       event->accept();
   }
   ```

4. **`paintEvent()`**: Render the shapes
   ```cpp
   void DrawWidget::paintEvent(QPaintEvent *event) {
       QPainter painter(this);
       // Draw all saved shapes
       // Draw current shape if m_drawing is true
   }
   ```

Accepting events prevents them from propagating to parent widgets, ensuring your drawing widget fully controls mouse behavior. You might also enable mouse tracking with `setMouseTracking(true)` to receive move events even when no button is pressed, useful for hover effects or cursor changes.
