/**
 * @file NotificationController.h
 * @brief QML controller for notification management and real-time alerts.
 *
 * Manages notification display lifecycle, priority queuing, auto-dismiss timeout,
 * and notification persistence. Notifications are queued by priority and displayed
 * in the order they were received (FIFO within same priority level).
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QQueue>
#include <memory>

namespace zmon
{
    /**
     * @class NotificationController
     * @brief QML controller for notification management and real-time alerts.
     *
     * Handles:
     * - Notification display and lifecycle (appear/dismiss/auto-timeout)
     * - Priority-based queuing (CRITICAL > MAJOR > MINOR > INFO)
     * - Notification state tracking (unread, acknowledged, dismissed)
     * - Auto-dismiss timeout (default 5 seconds for non-critical)
     * - Manual notification dismissal and clearing
     *
     * Notifications are stored as QVariantMap with keys:
     *   - id (QString): Unique notification ID
     *   - type (QString): Alarm/Alert type (e.g., "ECG_ANOMALY", "SENSOR_DISCONNECTED")
     *   - message (QString): Human-readable message
     *   - priority (QString): CRITICAL, MAJOR, MINOR, or INFO
     *   - timestamp (qlonglong): When notification was created (milliseconds since epoch)
     *   - read (bool): Whether notification has been read
     *   - expiresAt (qlonglong): When notification auto-dismisses (milliseconds since epoch)
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     * @ingroup NotificationSystem
     */
    class NotificationController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList notifications READ notifications NOTIFY notificationsChanged)
        Q_PROPERTY(int notificationCount READ notificationCount NOTIFY notificationCountChanged)
        Q_PROPERTY(bool hasUnreadNotifications READ hasUnreadNotifications NOTIFY hasUnreadNotificationsChanged)
        Q_PROPERTY(QVariantMap lastNotification READ lastNotification NOTIFY lastNotificationChanged)

    public:
        /**
         * @brief Notification priority levels.
         *
         * Controls display order and default timeout behavior:
         * - CRITICAL: Highest priority, no auto-dismiss (require manual acknowledgment)
         * - MAJOR: High priority, 10-second timeout
         * - MINOR: Medium priority, 7-second timeout
         * - INFO: Low priority, 5-second timeout
         */
        enum class Priority
        {
            CRITICAL = 0, /**< Critical alarm - user action required */
            MAJOR = 1,    /**< Major alarm - user attention recommended */
            MINOR = 2,    /**< Minor alarm - informational */
            INFO = 3      /**< Informational message */
        };

        explicit NotificationController(QObject *parent = nullptr);
        ~NotificationController() override;

        // Properties
        QVariantList notifications() const { return m_notifications; }
        int notificationCount() const { return m_notifications.size(); }
        bool hasUnreadNotifications() const { return m_hasUnreadNotifications; }
        QVariantMap lastNotification() const { return m_lastNotification; }

        // QML-invokable methods
        Q_INVOKABLE void addNotification(const QString &type, const QString &message,
                                         const QString &priority = "INFO");
        Q_INVOKABLE void clearNotification(const QString &notificationId);
        Q_INVOKABLE void clearAllNotifications();
        Q_INVOKABLE void markAsRead(const QString &notificationId);
        Q_INVOKABLE void acknowledgeNotification(const QString &notificationId);

        // C++ API for internal use
        void postNotification(const QString &type, const QString &message,
                              Priority priority, int timeoutSeconds = -1);

    signals:
        /// Emitted when notification list changes (add/remove/update)
        void notificationsChanged();

        /// Emitted when notification count changes
        void notificationCountChanged();

        /// Emitted when unread notification state changes
        void hasUnreadNotificationsChanged();

        /// Emitted when most recent notification changes
        void lastNotificationChanged();

        /// Emitted when a notification appears on screen
        void notificationAppeared(const QVariantMap &notification);

        /// Emitted when a notification is dismissed
        void notificationDismissed(const QString &notificationId);

        /// Emitted when user acknowledges a critical notification
        void notificationAcknowledged(const QString &notificationId);

    private slots:
        /// Periodically checks for expired notifications
        void onCheckExpiredNotifications();

    private:
        /// Reorders notifications by priority (CRITICAL → MAJOR → MINOR → INFO, FIFO within priority)
        void sortNotificationsByPriority();

        /// Updates unread notification flag
        void updateUnreadStatus();

        /// Finds notification index by ID (-1 if not found)
        int findNotificationIndex(const QString &notificationId) const;

        /// Gets default timeout for priority level
        int getDefaultTimeout(Priority priority) const;

        /// Generates unique notification ID
        QString generateNotificationId();

        // Member variables
        QVariantList m_notifications;         /**< Current notifications */
        QVariantMap m_lastNotification;       /**< Most recent notification */
        bool m_hasUnreadNotifications{false}; /**< Whether any unread notifications exist */
        QTimer m_expirationTimer;             /**< Timer for checking expired notifications */
        int m_notificationCounter{0};         /**< Counter for generating unique IDs */
    };
} // namespace zmon
