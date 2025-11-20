# Lesson 6: Events and Event Handling

## Learning Goals

- Understand Qt's event system and how it differs from signals/slots
- Learn to override event handlers like `mousePressEvent()` and `keyPressEvent()`
- Master the `event()` function for general event handling
- Implement event filters to intercept events before they reach target widgets
- Understand event propagation and acceptance

## Introduction

While signals and slots handle high-level communication between objects, Qt's event system deals with low-level interactions from the operating system and user input devices. Events represent things like mouse clicks, key presses, window resizes, and paint requests. Understanding event handling is essential for creating responsive, interactive applications.

Qt delivers events to widgets through virtual functions that you can override. Events can be accepted, ignored, or filtered, giving you fine-grained control over application behavior.

## The Event System Architecture

Events flow through Qt in a well-defined path:

1. System generates an event (mouse click, key press, etc.)
2. Qt creates a QEvent object describing the event
3. Event is delivered to the target widget via `QApplication::notify()`
4. Widget's `event()` function dispatches to specific handlers
5. Handler processes or ignores the event

If an event is ignored, it propagates to the parent widget, allowing parent containers to respond to child events.

## Widget-Specific Event Handlers

Qt widgets have virtual event handler functions for common events:

```cpp
class MyWidget : public QWidget
{
protected:
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "Clicked at:" << event->pos();
        event->accept();  // Mark as handled
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            close();
        }
    }
};
```

Common event handlers include:
- `mousePressEvent()`, `mouseReleaseEvent()`, `mouseMoveEvent()`
- `keyPressEvent()`, `keyReleaseEvent()`
- `paintEvent()`, `resizeEvent()`, `closeEvent()`
- `wheelEvent()`, `dragEnterEvent()`, `dropEvent()`

## The General event() Function

For events without specific handlers, override the `event()` function:

```cpp
bool MyWidget::event(QEvent *event) override
{
    if (event->type() == QEvent::ToolTip) {
        // Handle tooltip events
        return true;  // Event handled
    }
    return QWidget::event(event);  // Pass to base class
}
```

Return `true` if you handle the event, `false` to pass it along. Always call the base class implementation for unhandled events.

## Event Filters

Event filters intercept events before they reach their target widget. This is useful for:
- Monitoring events across multiple widgets
- Implementing global keyboard shortcuts
- Validating input before widgets process it

Install a filter with `installEventFilter()`:

```cpp
class EventFilter : public QObject
{
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Key pressed:" << keyEvent->key();
            return false;  // Let event continue
        }
        return QObject::eventFilter(watched, event);
    }
};

// Install the filter
widget->installEventFilter(new EventFilter(widget));
```

Return `true` to block the event, `false` to let it continue to the target.

## Event Acceptance and Propagation

Events have an acceptance flag that controls propagation:

- `event->accept()`: Mark event as handled, stop propagation
- `event->ignore()`: Allow event to propagate to parent widget

For example, if a child widget doesn't handle a key press, calling `ignore()` lets the parent try to handle it. This enables flexible event handling hierarchies.

## Example Walkthrough

Our example creates a custom widget that:
1. Changes color when clicked (mouse event handling)
2. Moves on arrow keys (keyboard event handling)
3. Displays click coordinates (event data access)
4. Uses an event filter to log all key presses globally

The application demonstrates the event flow, handler overriding, and filtering in action.

## Expected Output

A window displays a colored square widget. Behaviors:
- **Mouse clicks**: Widget changes color and displays click coordinates
- **Arrow keys**: Widget moves in the pressed direction
- **Escape key**: Window closes
- **All key presses**: Logged to console via event filter

The terminal shows event filter messages for each key press, demonstrating event interception.

## Try It

1. Build and run the application
2. Click on the colored widget and observe color changes
3. Use arrow keys to move the widget around
4. Press Escape to close the window
5. Add a custom event type and emit it with `QApplication::postEvent()`
6. Implement mouse drag behavior by handling `mouseMoveEvent()`
7. Try blocking an event in the event filter by returning `true`

## Key Takeaways

- Events represent low-level system and user interactions
- Override specific handlers (`mousePressEvent()`, etc.) for targeted event handling
- Use `event()` for general event handling and custom event types
- Event filters intercept events before they reach target widgets
- Events can be accepted (stop propagation) or ignored (continue to parent)
- Event handling complements signals/slots: events for low-level, signals for high-level
- Always call base class implementations for unhandled events
