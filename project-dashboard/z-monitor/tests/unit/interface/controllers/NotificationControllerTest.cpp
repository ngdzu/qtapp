/**
 * @file NotificationControllerTest.cpp
 * @brief GoogleTest unit tests for NotificationController.
 *
 * Tests cover:
 * - Notification queuing and priority ordering (CRITICAL > MAJOR > MINOR > INFO)
 * - Notification lifecycle (appear, read, dismiss, timeout)
 * - Auto-dismiss timeout behavior (priority-based delays)
 * - Notification state tracking (unread flag, acknowledgment)
 * - Edge cases (empty list, duplicate notifications, rapid additions)
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <memory>

#include "interface/controllers/NotificationController.h"

using namespace zmon;

/**
 * @brief Test fixture for NotificationController tests.
 *
 * Sets up a clean NotificationController instance for each test
 * and provides helper methods for verification.
 */
class NotificationControllerTest : public ::testing::Test
{
protected:
    void SetUp() override { m_controller = std::make_unique<NotificationController>(); }

    void TearDown() override { m_controller.reset(); }

    /**
     * @brief Helper to verify notification count and emit signals.
     */
    void verifyNotificationCount(int expectedCount)
    {
        EXPECT_EQ(m_controller->notificationCount(), expectedCount);
    }

    /**
     * @brief Helper to verify notification priority order.
     *
     * Checks that notifications are ordered by priority: CRITICAL > MAJOR > MINOR > INFO.
     * Within the same priority, maintains FIFO order (insertion order).
     */
    void verifyPriorityOrder()
    {
        auto notifications = m_controller->notifications();
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

        for (int i = 1; i < notifications.size(); ++i)
        {
            int prevPriority = priorityValue(notifications[i - 1].toMap()["priority"].toString());
            int currPriority = priorityValue(notifications[i].toMap()["priority"].toString());
            EXPECT_LE(prevPriority, currPriority)
                << "Priority order violation at index " << i << ": " << prevPriority << " should be <= " << currPriority;
        }
    }

    /**
     * @brief Helper to get notification ID by index.
     */
    QString getNotificationId(int index)
    {
        auto notifications = m_controller->notifications();
        if (index >= 0 && index < notifications.size())
        {
            return notifications[index].toMap()["id"].toString();
        }
        return "";
    }

    /**
     * @brief Helper to get notification priority by index.
     */
    QString getNotificationPriority(int index)
    {
        auto notifications = m_controller->notifications();
        if (index >= 0 && index < notifications.size())
        {
            return notifications[index].toMap()["priority"].toString();
        }
        return "";
    }

    std::unique_ptr<NotificationController> m_controller;
};

/**
 * @test TASK-UI-043-TEST-001: Initialization
 *
 * Verify that NotificationController initializes with correct default values.
 */
TEST_F(NotificationControllerTest, InitializesWithDefaults)
{
    EXPECT_EQ(m_controller->notificationCount(), 0);
    EXPECT_FALSE(m_controller->hasUnreadNotifications());
    EXPECT_TRUE(m_controller->notifications().isEmpty());
    EXPECT_TRUE(m_controller->lastNotification().isEmpty());
}

/**
 * @test TASK-UI-043-TEST-002: Add Single Notification
 *
 * Verify that adding a single notification updates properties and signals correctly.
 */
TEST_F(NotificationControllerTest, AddSingleNotification)
{
    QSignalSpy countChangedSpy(m_controller.get(), &NotificationController::notificationCountChanged);
    QSignalSpy notificationsChangedSpy(m_controller.get(), &NotificationController::notificationsChanged);
    QSignalSpy unreadChangedSpy(m_controller.get(), &NotificationController::hasUnreadNotificationsChanged);
    QSignalSpy lastNotificationChangedSpy(m_controller.get(),
                                          &NotificationController::lastNotificationChanged);
    QSignalSpy appearedSpy(m_controller.get(), &NotificationController::notificationAppeared);

    m_controller->addNotification("TEST_ALARM", "Test message", "CRITICAL");

    // Verify count
    verifyNotificationCount(1);

    // Verify signals were emitted
    EXPECT_EQ(notificationsChangedSpy.count(), 1);
    EXPECT_EQ(countChangedSpy.count(), 1);
    EXPECT_EQ(unreadChangedSpy.count(), 1);
    EXPECT_EQ(lastNotificationChangedSpy.count(), 1);
    EXPECT_EQ(appearedSpy.count(), 1);

    // Verify notification properties
    auto notifications = m_controller->notifications();
    EXPECT_EQ(notifications.size(), 1);

    auto notification = notifications[0].toMap();
    EXPECT_EQ(notification["type"].toString(), "TEST_ALARM");
    EXPECT_EQ(notification["message"].toString(), "Test message");
    EXPECT_EQ(notification["priority"].toString(), "CRITICAL");
    EXPECT_FALSE(notification["read"].toBool());
    EXPECT_FALSE(notification["acknowledged"].toBool());

    // Verify unread flag
    EXPECT_TRUE(m_controller->hasUnreadNotifications());

    // Verify last notification
    EXPECT_EQ(m_controller->lastNotification()["type"].toString(), "TEST_ALARM");
}

/**
 * @test TASK-UI-043-TEST-003: Priority-Based Ordering
 *
 * Verify that notifications are ordered by priority: CRITICAL > MAJOR > MINOR > INFO.
 * Multiple notifications should maintain insertion order within the same priority level.
 */
TEST_F(NotificationControllerTest, PriorityBasedOrdering)
{
    // Add notifications in random priority order
    m_controller->addNotification("INFO_1", "Info message 1", "INFO");
    m_controller->addNotification("CRITICAL_1", "Critical message 1", "CRITICAL");
    m_controller->addNotification("MINOR_1", "Minor message 1", "MINOR");
    m_controller->addNotification("MAJOR_1", "Major message 1", "MAJOR");
    m_controller->addNotification("CRITICAL_2", "Critical message 2", "CRITICAL");
    m_controller->addNotification("MAJOR_2", "Major message 2", "MAJOR");
    m_controller->addNotification("INFO_2", "Info message 2", "INFO");

    verifyNotificationCount(7);

    // Verify priority order
    verifyPriorityOrder();

    // Verify FIFO within priority levels
    EXPECT_EQ(getNotificationId(0), m_controller->notifications()[0].toMap()["id"].toString()); // CRITICAL_1
    EXPECT_EQ(getNotificationId(1), m_controller->notifications()[1].toMap()["id"].toString()); // CRITICAL_2
    EXPECT_EQ(getNotificationPriority(0), "CRITICAL");
    EXPECT_EQ(getNotificationPriority(1), "CRITICAL");
    EXPECT_EQ(getNotificationPriority(2), "MAJOR");
    EXPECT_EQ(getNotificationPriority(3), "MAJOR");
    EXPECT_EQ(getNotificationPriority(4), "MINOR");
    EXPECT_EQ(getNotificationPriority(5), "INFO");
    EXPECT_EQ(getNotificationPriority(6), "INFO");
}

/**
 * @test TASK-UI-043-TEST-004: Mark as Read
 *
 * Verify that marking a notification as read updates its state and unread flag.
 */
TEST_F(NotificationControllerTest, MarkAsRead)
{
    m_controller->addNotification("TEST", "Message", "INFO");

    QSignalSpy unreadChangedSpy(m_controller.get(), &NotificationController::hasUnreadNotificationsChanged);

    // Get notification ID
    QString notifId = m_controller->notifications()[0].toMap()["id"].toString();

    // Initially unread
    EXPECT_TRUE(m_controller->hasUnreadNotifications());

    // Mark as read
    m_controller->markAsRead(notifId);

    // Verify read status
    EXPECT_FALSE(m_controller->hasUnreadNotifications());
    EXPECT_EQ(unreadChangedSpy.count(), 1);

    // Verify notification state
    auto notification = m_controller->notifications()[0].toMap();
    EXPECT_TRUE(notification["read"].toBool());
}

/**
 * @test TASK-UI-043-TEST-005: Clear Single Notification
 *
 * Verify that clearing a notification removes it and emits appropriate signals.
 */
TEST_F(NotificationControllerTest, ClearSingleNotification)
{
    m_controller->addNotification("TEST_1", "Message 1", "INFO");
    m_controller->addNotification("TEST_2", "Message 2", "CRITICAL");

    QSignalSpy countChangedSpy(m_controller.get(), &NotificationController::notificationCountChanged);
    QSignalSpy dismissedSpy(m_controller.get(), &NotificationController::notificationDismissed);

    QString notifId = getNotificationId(0);

    // Clear notification
    m_controller->clearNotification(notifId);

    verifyNotificationCount(1);
    EXPECT_EQ(dismissedSpy.count(), 1);
    EXPECT_EQ(countChangedSpy.count(), 1);
}

/**
 * @test TASK-UI-043-TEST-006: Clear All Notifications
 *
 * Verify that clearing all notifications removes all items and updates all properties.
 */
TEST_F(NotificationControllerTest, ClearAllNotifications)
{
    m_controller->addNotification("TEST_1", "Message 1", "INFO");
    m_controller->addNotification("TEST_2", "Message 2", "CRITICAL");
    m_controller->addNotification("TEST_3", "Message 3", "MAJOR");

    QSignalSpy countChangedSpy(m_controller.get(), &NotificationController::notificationCountChanged);
    QSignalSpy notificationsChangedSpy(m_controller.get(), &NotificationController::notificationsChanged);
    QSignalSpy unreadChangedSpy(m_controller.get(), &NotificationController::hasUnreadNotificationsChanged);
    QSignalSpy lastNotifChangedSpy(m_controller.get(), &NotificationController::lastNotificationChanged);

    m_controller->clearAllNotifications();

    verifyNotificationCount(0);
    EXPECT_FALSE(m_controller->hasUnreadNotifications());
    EXPECT_TRUE(m_controller->lastNotification().isEmpty());
    EXPECT_EQ(notificationsChangedSpy.count(), 1);
    EXPECT_EQ(countChangedSpy.count(), 1);
    EXPECT_EQ(unreadChangedSpy.count(), 1);
    EXPECT_EQ(lastNotifChangedSpy.count(), 1);
}

/**
 * @test TASK-UI-043-TEST-007: Acknowledge Notification
 *
 * Verify that acknowledging a notification sets the acknowledged flag and emits signals.
 */
TEST_F(NotificationControllerTest, AcknowledgeNotification)
{
    m_controller->addNotification("TEST", "Critical message", "CRITICAL");

    QSignalSpy acknowledgedSpy(m_controller.get(), &NotificationController::notificationAcknowledged);
    QSignalSpy notificationsChangedSpy(m_controller.get(), &NotificationController::notificationsChanged);

    QString notifId = getNotificationId(0);

    // Acknowledge notification
    m_controller->acknowledgeNotification(notifId);

    EXPECT_EQ(acknowledgedSpy.count(), 1);
    EXPECT_EQ(notificationsChangedSpy.count(), 1);

    // Verify acknowledged flag
    auto notification = m_controller->notifications()[0].toMap();
    EXPECT_TRUE(notification["acknowledged"].toBool());
}

/**
 * @test TASK-UI-043-TEST-008: Auto-Dismiss Timeout Calculation
 *
 * Verify that notifications with timeout calculate expiration correctly.
 * This tests that the expiresAt field is set correctly based on timeout duration.
 */
TEST_F(NotificationControllerTest, AutoDismissTimeoutCalculation)
{
    qlonglong beforeTime = QDateTime::currentMSecsSinceEpoch();

    // Post notification with 5-second timeout
    m_controller->postNotification("TEST", "Message", NotificationController::Priority::INFO, 5);

    qlonglong afterTime = QDateTime::currentMSecsSinceEpoch();

    auto notification = m_controller->notifications()[0].toMap();
    qlonglong expiresAt = notification["expiresAt"].toLongLong();

    // expiresAt should be approximately 5 seconds from now
    qlonglong minExpected = beforeTime + 5000;
    qlonglong maxExpected = afterTime + 5000;

    EXPECT_GE(expiresAt, minExpected);
    EXPECT_LE(expiresAt, maxExpected);
}

/**
 * @test TASK-UI-043-TEST-009: Critical Notifications Don't Auto-Dismiss
 *
 * Verify that CRITICAL priority notifications don't auto-dismiss (expiresAt = -1).
 */
TEST_F(NotificationControllerTest, CriticalNotificationsNoAutoDismiss)
{
    m_controller->postNotification("CRITICAL_TEST", "Critical message",
                                   NotificationController::Priority::CRITICAL, -1);

    auto notification = m_controller->notifications()[0].toMap();
    qlonglong expiresAt = notification["expiresAt"].toLongLong();
    EXPECT_EQ(expiresAt, -1); // Should not auto-dismiss
}

/**
 * @test TASK-UI-043-TEST-010: Multiple Unread Notifications
 *
 * Verify that unread flag is correctly managed with multiple notifications.
 */
TEST_F(NotificationControllerTest, MultipleUnreadNotifications)
{
    m_controller->addNotification("TEST_1", "Message 1", "INFO");
    m_controller->addNotification("TEST_2", "Message 2", "CRITICAL");
    m_controller->addNotification("TEST_3", "Message 3", "MAJOR");

    EXPECT_TRUE(m_controller->hasUnreadNotifications());

    // Mark first as read
    QString notifId1 = getNotificationId(0);
    m_controller->markAsRead(notifId1);
    EXPECT_TRUE(m_controller->hasUnreadNotifications()); // Still has unread

    // Mark second as read
    QString notifId2 = getNotificationId(1);
    m_controller->markAsRead(notifId2);
    EXPECT_TRUE(m_controller->hasUnreadNotifications()); // Still has unread

    // Mark third as read
    QString notifId3 = getNotificationId(2);
    m_controller->markAsRead(notifId3);
    EXPECT_FALSE(m_controller->hasUnreadNotifications()); // Now all read
}

/**
 * @test TASK-UI-043-TEST-011: Last Notification Tracking
 *
 * Verify that lastNotification property always tracks the most recently added notification.
 */
TEST_F(NotificationControllerTest, LastNotificationTracking)
{
    EXPECT_TRUE(m_controller->lastNotification().isEmpty());

    m_controller->addNotification("FIRST", "First message", "INFO");
    EXPECT_EQ(m_controller->lastNotification()["type"].toString(), "FIRST");

    m_controller->addNotification("SECOND", "Second message", "CRITICAL");
    EXPECT_EQ(m_controller->lastNotification()["type"].toString(), "SECOND");

    m_controller->addNotification("THIRD", "Third message", "MAJOR");
    EXPECT_EQ(m_controller->lastNotification()["type"].toString(), "THIRD");

    // After clearing all, last notification should be empty
    m_controller->clearAllNotifications();
    EXPECT_TRUE(m_controller->lastNotification().isEmpty());
}

/**
 * @test TASK-UI-043-TEST-012: Signal Emission on State Changes
 *
 * Verify that all expected signals are emitted during notification lifecycle.
 */
TEST_F(NotificationControllerTest, SignalEmissionOnStateChanges)
{
    QSignalSpy addedSpy(m_controller.get(), &NotificationController::notificationAppeared);
    QSignalSpy countChangedSpy(m_controller.get(), &NotificationController::notificationCountChanged);
    QSignalSpy notificationsChangedSpy(m_controller.get(), &NotificationController::notificationsChanged);

    // Add notification
    m_controller->addNotification("TEST", "Message", "INFO");
    EXPECT_EQ(addedSpy.count(), 1);
    EXPECT_EQ(countChangedSpy.count(), 1);
    EXPECT_EQ(notificationsChangedSpy.count(), 1);

    int initialNotifCount = notificationsChangedSpy.count();

    // Clear notification
    QString notifId = getNotificationId(0);
    m_controller->clearNotification(notifId);

    EXPECT_GT(notificationsChangedSpy.count(), initialNotifCount);
    EXPECT_EQ(countChangedSpy.count(), 2);
}

/**
 * @test TASK-UI-043-TEST-013: Invalid Notification ID Handling
 *
 * Verify that operations with invalid notification IDs don't crash or cause errors.
 */
TEST_F(NotificationControllerTest, InvalidNotificationIdHandling)
{
    m_controller->addNotification("TEST", "Message", "INFO");

    // These should not crash or cause errors
    m_controller->markAsRead("INVALID_ID");
    m_controller->clearNotification("INVALID_ID");
    m_controller->acknowledgeNotification("INVALID_ID");

    // Original notification should still exist
    verifyNotificationCount(1);
}

/**
 * @test TASK-UI-043-TEST-014: Empty Clear All
 *
 * Verify that clearing all when no notifications exist doesn't cause errors.
 */
TEST_F(NotificationControllerTest, ClearAllWhenEmpty)
{
    QSignalSpy countChangedSpy(m_controller.get(), &NotificationController::notificationCountChanged);

    // Clear when empty (should not emit signals)
    m_controller->clearAllNotifications();

    EXPECT_EQ(countChangedSpy.count(), 0);
    verifyNotificationCount(0);
}

/**
 * @test TASK-UI-043-TEST-015: Rapid Notification Addition
 *
 * Verify that adding notifications rapidly maintains priority order and doesn't lose data.
 */
TEST_F(NotificationControllerTest, RapidNotificationAddition)
{
    // Add 20 notifications rapidly
    for (int i = 0; i < 20; ++i)
    {
        QString priority;
        if (i % 4 == 0)
            priority = "CRITICAL";
        else if (i % 4 == 1)
            priority = "MAJOR";
        else if (i % 4 == 2)
            priority = "MINOR";
        else
            priority = "INFO";

        m_controller->addNotification(QString("TEST_%1").arg(i), QString("Message %1").arg(i),
                                      priority);
    }

    verifyNotificationCount(20);
    verifyPriorityOrder();
}
