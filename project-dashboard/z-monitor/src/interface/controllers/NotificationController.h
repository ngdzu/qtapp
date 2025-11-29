/**
 * @file NotificationController.h
 * @brief QML controller for notification management UI.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace zmon
{
    /**
     * @class NotificationController
     * @brief QML controller for notification management.
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class NotificationController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList notifications READ notifications NOTIFY notificationsChanged)
        Q_PROPERTY(int notificationCount READ notificationCount NOTIFY notificationCountChanged)
        Q_PROPERTY(bool hasUnreadNotifications READ hasUnreadNotifications NOTIFY hasUnreadNotificationsChanged)

    public:
        explicit NotificationController(QObject *parent = nullptr);
        ~NotificationController() override = default;

        QVariantList notifications() const { return m_notifications; }
        int notificationCount() const { return m_notifications.size(); }
        bool hasUnreadNotifications() const { return m_hasUnreadNotifications; }

        Q_INVOKABLE void clearNotification(const QString &notificationId);
        Q_INVOKABLE void clearAllNotifications();
        Q_INVOKABLE void markAsRead(const QString &notificationId);

    signals:
        void notificationsChanged();
        void notificationCountChanged();
        void hasUnreadNotificationsChanged();

    private:
        QVariantList m_notifications;
        bool m_hasUnreadNotifications{false};
    };
} // namespace zmon
