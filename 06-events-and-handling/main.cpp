#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPainter>
#include <QDebug>
#include <QRandomGenerator>

// Custom widget that handles mouse and keyboard events
class EventWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedSize(200, 200);
        setFocusPolicy(Qt::StrongFocus); // Enable keyboard focus
        generateRandomColor();
    }

protected:
    // Override mouse press event
    void mousePressEvent(QMouseEvent *event) override
    {
        m_lastClickPos = event->pos();
        generateRandomColor();
        update(); // Trigger repaint

        qDebug() << "Mouse clicked at:" << event->pos()
                 << "Button:" << event->button();

        event->accept();
    }

    // Override keyboard event for arrow key movement
    void keyPressEvent(QKeyEvent *event) override
    {
        const int moveStep = 10;
        QPoint newPos = pos();

        switch (event->key())
        {
        case Qt::Key_Left:
            newPos.setX(newPos.x() - moveStep);
            break;
        case Qt::Key_Right:
            newPos.setX(newPos.x() + moveStep);
            break;
        case Qt::Key_Up:
            newPos.setY(newPos.y() - moveStep);
            break;
        case Qt::Key_Down:
            newPos.setY(newPos.y() + moveStep);
            break;
        case Qt::Key_Escape:
            qDebug() << "Escape pressed - closing window";
            window()->close();
            event->accept();
            return;
        default:
            QWidget::keyPressEvent(event);
            return;
        }

        move(newPos);
        qDebug() << "Widget moved to:" << newPos;
        event->accept();
    }

    // Override paint event to draw custom appearance
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect(), m_color);

        // Draw click position if available
        if (m_lastClickPos.x() >= 0)
        {
            painter.setPen(Qt::white);
            painter.drawText(rect(), Qt::AlignCenter,
                             QString("Last click:\n(%1, %2)")
                                 .arg(m_lastClickPos.x())
                                 .arg(m_lastClickPos.y()));
        }
        else
        {
            painter.setPen(Qt::white);
            painter.drawText(rect(), Qt::AlignCenter,
                             "Click me!\nUse arrow keys to move");
        }
    }

    // Override general event function for additional event types
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::Enter)
        {
            qDebug() << "Mouse entered widget area";
        }
        else if (event->type() == QEvent::Leave)
        {
            qDebug() << "Mouse left widget area";
        }
        return QWidget::event(event);
    }

private:
    void generateRandomColor()
    {
        m_color = QColor::fromRgb(
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256));
    }

    QColor m_color;
    QPoint m_lastClickPos{-1, -1};
};

// Event filter class to intercept events globally
class GlobalEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit GlobalEventFilter(QObject *parent = nullptr)
        : QObject(parent) {}

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        // Log all key press events globally
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            qDebug() << "[Event Filter] Key pressed:"
                     << keyEvent->key()
                     << "Text:" << keyEvent->text()
                     << "in object:" << watched->objectName();

            // Don't block the event - let it continue to target
            return false;
        }

        // Pass all other events to base implementation
        return QObject::eventFilter(watched, event);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Main window
    QWidget window;
    window.setWindowTitle("Lesson 6: Events and Event Handling");
    window.resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(&window);

    // Instruction label
    QLabel *instructions = new QLabel(
        "Instructions:\n"
        "• Click the colored box to change its color\n"
        "• Use arrow keys to move the box\n"
        "• Press Escape to close the window\n"
        "• Watch the console for event filter messages");
    instructions->setWordWrap(true);
    layout->addWidget(instructions);

    // Custom event-handling widget
    EventWidget *eventWidget = new EventWidget(&window);
    eventWidget->setObjectName("EventWidget");
    layout->addWidget(eventWidget, 0, Qt::AlignCenter);

    // Add stretch to push content to top
    layout->addStretch();

    // Install global event filter
    GlobalEventFilter *filter = new GlobalEventFilter(&window);
    app.installEventFilter(filter);

    qDebug() << "Application started. Try clicking and using arrow keys!";

    window.show();
    return app.exec();
}

#include "main.moc"
