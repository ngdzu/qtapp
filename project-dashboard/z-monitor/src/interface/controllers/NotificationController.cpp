/**
 * @file NotificationController.cpp
 * @brief Implementation of NotificationController for real-time notification management.
 *
 * Handles notification lifecycle, priority queuing, auto-dismiss timeouts, and
 * state tracking for the notification system.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "NotificationController.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

namespace zmon
{
    NotificationController::NotificationController(QObject *parent) : QObject(parent), m_notificationCounter(0)
    {
        // Timer for checking expired notifications - runs every second
        // Only start timer if we're in an application context with an event loop
        connect(&m_expirationTimer, &QTimer::timeout, this, &NotificationController::onCheckExpiredNotifications);
        m_expirationTimer.setInterval(1000); // Check every second

        // Defer timer start to allow event loop to be set up
        QTimer::singleShot(100, this, [this]()
                           {
            if (QCoreApplication::instance())
            {
                m_expirationTimer.start();
            } });
    }

    NotificationController::~NotificationController()
    {
        m_expirationTimer.stop();
    }

    void NotificationController::addNotification(const QString &type, const QString &message,
                                                 const QString &priority)
    {
        Priority priorityEnum = Priority::INFO;
        if (priority == "CRITICAL")
            priorityEnum = Priority::CRITICAL;
        else if (priority == "MAJOR")
            priorityEnum = Priority::MAJOR;
        else if (priority == "MINOR")
            priorityEnum = Priority::MINOR;

        postNotification(type, message, priorityEnum);
    }

    void NotificationController::postNotification(const QString &type, const QString &message,
                                                  Priority priority, int timeoutSeconds)
    {
        // Generate unique ID
        QString notificationId = generateNotificationId();

        // Get current timestamp
        qlonglong now = QDateTime::currentMSecsSinceEpoch();

        // Calculate expiration time
        qlonglong expiresAt = -1; // -1 means no auto-dismiss (for CRITICAL)
        if (timeoutSeconds == -1)
        {
            // Use default timeout based on priority
            timeoutSeconds = getDefaultTimeout(priority);
        }
        if (timeoutSeconds > 0)
        {
            expiresAt = now + (timeoutSeconds * 1000);
        }

        // Create notification map
        QVariantMap notification;
        notification["id"] = notificationId;
        notification["type"] = type;
        notification["message"] = message;
        notification["priority"] = priority == Priority::CRITICAL ? "CRITICAL"
                                   : priority == Priority::MAJOR  ? "MAJOR"
                                   : priority == Priority::MINOR  ? "MINOR"
                                                                  : "INFO";
        notification["timestamp"] = now;
        notification["read"] = false;
        notification["acknowledged"] = false;
        notification["expiresAt"] = expiresAt;

        // Add to notification list
        m_notifications.append(notification);

        // Update last notification
        m_lastNotification = notification;

        // Update unread status
        updateUnreadStatus();

        // Sort by priority
        sortNotificationsByPriority();

        // Emit signals
        emit notificationsChanged();
        emit notificationCountChanged();
        emit lastNotificationChanged();
        emit notificationAppeared(notification);
    }

    void NotificationController::clearNotification(const QString &notificationId)
    {
        int index = findNotificationIndex(notificationId);
        if (index >= 0)
        {
            m_notifications.removeAt(index);
            emit notificationsChanged();
            emit notificationCountChanged();
            emit notificationDismissed(notificationId);
            updateUnreadStatus();
        }
    }

    void NotificationController::clearAllNotifications()
    {
        // Only clear if there are notifications
        if (m_notifications.isEmpty())
            return;

        m_notifications.clear();
        m_lastNotification = QVariantMap();
        m_hasUnreadNotifications = false;

        emit notificationsChanged();
        emit notificationCountChanged();
        emit hasUnreadNotificationsChanged();
        emit lastNotificationChanged();
    }

    void NotificationController::markAsRead(const QString &notificationId)
    {
        int index = findNotificationIndex(notificationId);
        if (index >= 0)
        {
            QVariantMap notification = m_notifications[index].toMap();
            if (!notification["read"].toBool())
            {
                notification["read"] = true;
                m_notifications[index] = notification;
                updateUnreadStatus();
                emit notificationsChanged();
                if (m_hasUnreadNotifications)
                {
                    emit hasUnreadNotificationsChanged();
                }
            }
        }
    }

    void NotificationController::acknowledgeNotification(const QString &notificationId)
    {
        int index = findNotificationIndex(notificationId);
        if (index >= 0)
        {
            QVariantMap notification = m_notifications[index].toMap();
            notification["acknowledged"] = true;
            m_notifications[index] = notification;
            emit notificationsChanged();
            emit notificationAcknowledged(notificationId);
        }
    }

    void NotificationController::onCheckExpiredNotifications()
    {
        qlonglong now = QDateTime::currentMSecsSinceEpoch();
        bool changed = false;

        // Check each notification for expiration
        for (int i = m_notifications.size() - 1; i >= 0; --i)
        {
            QVariantMap notification = m_notifications[i].toMap();
            qlonglong expiresAt = notification["expiresAt"].toLongLong();

            // If expiration time is set and has passed, remove notification
            if (expiresAt > 0 && now >= expiresAt)
            {
                QString notificationId = notification["id"].toString();
                m_notifications.removeAt(i);
                changed = true;
                emit notificationDismissed(notificationId);
            }
        }

        if (changed)
        {
            emit notificationsChanged();
            emit notificationCountChanged();
            updateUnreadStatus();
        }
    }

    void NotificationController::sortNotificationsByPriority()
    {
        // Priority order: CRITICAL (0) > MAJOR (1) > MINOR (2) > INFO (3)
        // Within same priority: FIFO (keep original insertion order)
        auto priorityValue = [](const QString &priority) -> int
        {
            if (priority == "CRITICAL")
                return 0;
            if (priority == "MAJOR")
                return 1;
            if (priority == "MINOR")
                return 2;
            return 3; // INFO
        };

        // Stable sort to maintain FIFO within priority levels
        std::stable_sort(m_notifications.begin(), m_notifications.end(),
                         [&priorityValue](const QVariant &a, const QVariant &b)
                         {
                             int priorityA = priorityValue(a.toMap()["priority"].toString());
                             int priorityB = priorityValue(b.toMap()["priority"].toString());
                             return priorityA < priorityB;
                         });
    }

    void NotificationController::updateUnreadStatus()
    {
        bool hasUnread = false;
        for (const auto &notif : m_notifications)
        {
            if (!notif.toMap()["read"].toBool())
            {
                hasUnread = true;
                break;
            }
        }

        if (hasUnread != m_hasUnreadNotifications)
        {
            m_hasUnreadNotifications = hasUnread;
            emit hasUnreadNotificationsChanged();
        }
    }

    int NotificationController::findNotificationIndex(const QString &notificationId) const
    {
        for (int i = 0; i < m_notifications.size(); ++i)
        {
            if (m_notifications[i].toMap()["id"].toString() == notificationId)
            {
                return i;
            }
        }
        return -1;
    }

    int NotificationController::getDefaultTimeout(Priority priority) const
    {
        switch (priority)
        {
        case Priority::CRITICAL:
            return 0; // No auto-dismiss for critical
        case Priority::MAJOR:
            return 10; // 10 seconds
        case Priority::MINOR:
            return 7; // 7 seconds
        case Priority::INFO:
            return 5; // 5 seconds
        default:
            return 5;
        }
    }

    QString NotificationController::generateNotificationId()
    {
        return QString("notif_%1_%2").arg(++m_notificationCounter).arg(QDateTime::currentMSecsSinceEpoch());
    }
} // namespace zmon
