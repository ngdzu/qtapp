/**
 * @file NotificationController.cpp
 * @brief Implementation of NotificationController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "NotificationController.h"

namespace zmon
{
    NotificationController::NotificationController(QObject *parent) : QObject(parent)
    {
        // TODO: Connect to notification service
    }

    void NotificationController::clearNotification(const QString &notificationId)
    {
        Q_UNUSED(notificationId)
        // TODO: Remove notification
        emit notificationsChanged();
        emit notificationCountChanged();
    }

    void NotificationController::clearAllNotifications()
    {
        // TODO: Clear all notifications (requires permission check)
        m_notifications.clear();
        emit notificationsChanged();
        emit notificationCountChanged();
        m_hasUnreadNotifications = false;
        emit hasUnreadNotificationsChanged();
    }

    void NotificationController::markAsRead(const QString &notificationId)
    {
        Q_UNUSED(notificationId)
        // TODO: Mark notification as read
    }
} // namespace zmon
