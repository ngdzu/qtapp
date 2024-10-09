
#include "customapplication.h"
#include "chatoverlay.h"
#include <QKeyEvent>

bool CustomApplication::notify(QObject *receiver, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space &&
            keyEvent->modifiers() == (Qt::ShiftModifier | Qt::AltModifier))
        {
            qDebug() << "Shift + Option + Space detected globally!";

            ChatOverlay *chatOverlay = new ChatOverlay();
            chatOverlay->show(); // Initially hide the chat overlay
            return true;         // Event is handled, stop propagation
        }
    }
    // Call the base class implementation for default behavior
    return QApplication::notify(receiver, event);
}