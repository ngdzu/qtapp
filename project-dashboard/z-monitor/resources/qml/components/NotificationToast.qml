/**
 * @file NotificationToast.qml
 * @brief Toast notification component for real-time alerts and messages.
 *
 * Displays a single notification as a toast message with priority-based styling,
 * auto-dismiss timeout, and manual dismiss button. Component responds to
 * NotificationController property changes and emits signals for user interactions.
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    /**
     * @property notification
     * @brief The notification data to display.
     *
     * Expected structure (QVariantMap from NotificationController):
     * - id: Unique notification identifier
     * - type: Alarm/Alert type (e.g., "ECG_ANOMALY")
     * - message: Human-readable message
     * - priority: CRITICAL, MAJOR, MINOR, or INFO
     * - timestamp: When notification was created (milliseconds since epoch)
     * - read: Whether notification has been read
     * - expiresAt: When auto-dismiss will occur (-1 for never)
     */
    property var notification: null

    /**
     * @property controller
     * @brief Reference to NotificationController for action callbacks.
     */
    property var controller: null

    /**
     * @property hideDelay
     * @brief Delay before toast auto-hides (in milliseconds).
     *
     * Extracted from notification.expiresAt or uses default based on priority.
     */
    property int hideDelay: calculateHideDelay()

    // Styling
    implicitWidth: 360
    implicitHeight: contentLayout.implicitHeight + 16
    color: getBackgroundColor()
    border.color: getBorderColor()
    border.width: 1
    radius: 8

    // Opacity animation on appearance
    opacity: 0
    Behavior on opacity {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    // Z-stacking: keep toast above other elements
    z: 1000

    // Auto-dismiss timer
    Timer {
        id: autoDismissTimer
        interval: root.hideDelay
        onTriggered: {
            if (root.notification && !root.notification.acknowledged)
            {
                root.fadeOut()
            }
        }
    }

    // Background for interactive area
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: autoDismissTimer.stop()
        onExited: autoDismissTimer.restart()
        onClicked: {
            // Mark as read on click
            if (root.controller && root.notification)
            {
                root.controller.markAsRead(root.notification.id)
            }
        }
    }

    ColumnLayout {
        id: contentLayout
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 12
        }
        spacing: 8

        // Header: type, timestamp, close button
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Priority indicator (left edge)
            Rectangle {
                width: 4
                Layout.fillHeight: true
                color: getBorderColor()
                radius: 2
            }

            // Type label
            Text {
                text: root.notification ? root.notification.type : ""
                font.pixelSize: 13
                font.bold: true
                font.letterSpacing: 0.5
                color: getTextColor()
                Layout.fillWidth: true
            }

            // Timestamp
            Text {
                text: root.notification ? formatTime(root.notification.timestamp) : ""
                font.pixelSize: 11
                color: "#a1a1aa" // Zinc-400
            }

            // Priority badge
            Rectangle {
                color: getPriorityBadgeColor()
                radius: 3
                implicitWidth: priorityBadgeText.width + 8
                implicitHeight: 18

                Text {
                    id: priorityBadgeText
                    anchors.centerIn: parent
                    text: root.notification ? root.notification.priority : ""
                    font.pixelSize: 10
                    font.bold: true
                    color: "#ffffff"
                }
            }

            // Close button
            Button {
                implicitWidth: 32
                implicitHeight: 32
                padding: 4

                background: Rectangle {
                    color: parent.hovered ? "#52525b" : "transparent" // Zinc-600 on hover
                    radius: 4
                    border.color: parent.hovered ? "#71717a" : "transparent"
                    border.width: 1
                }

                contentItem: Text {
                    text: "✕"
                    font.pixelSize: 16
                    color: "#e4e4e7" // Zinc-200
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (root.controller && root.notification)
                    {
                        root.controller.clearNotification(root.notification.id)
                    }
                    root.fadeOut()
                }
            }
        }

        // Message text
        Text {
            text: root.notification ? root.notification.message : ""
            font.pixelSize: 12
            color: "#e4e4e7" // Zinc-200
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // Action buttons for critical notifications
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: root.notification && root.notification.priority === "CRITICAL"

            Button {
                text: "Acknowledge"
                Layout.fillWidth: true

                background: Rectangle {
                    color: "#ef4444" // Red-500
                    radius: 4
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    font.bold: true
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (root.controller && root.notification)
                    {
                        root.controller.acknowledgeNotification(root.notification.id)
                    }
                }
            }

            Button {
                text: "Silence"
                Layout.fillWidth: true

                background: Rectangle {
                    color: parent.hovered ? "#374151" : "#1f2937" // Gray-700/800
                    radius: 4
                    border.color: "#4b5563"
                    border.width: 1
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    font.bold: true
                    color: "#e4e4e7"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (root.controller && root.notification)
                    {
                        root.controller.clearNotification(root.notification.id)
                    }
                    root.fadeOut()
                }
            }
        }
    }

    onNotificationChanged: {
        if (root.notification)
        {
            // Show the toast
            opacity = 1
            // Start auto-dismiss timer for non-critical or acknowledged notifications
            if (root.notification.priority !== "CRITICAL" || root.notification.acknowledged)
            {
                autoDismissTimer.restart()
            }
            else
            {
                autoDismissTimer.stop()
            }
        }
    }

    /**
     * @brief Animate fade-out and trigger dismissal.
     */
    function fadeOut()
    {
        opacity = 0
    }

    /**
     * @brief Get background color based on notification priority.
     */
    function getBackgroundColor()
    {
        if (!root.notification)
            return "#27272a" // Zinc-800

        switch (root.notification.priority)
        {
        case "CRITICAL":
            return "#7f1d1d" // Red-900
        case "MAJOR":
            return "#7c2d12" // Orange-900
        case "MINOR":
            return "#713f12" // Yellow-900
        default:
            return "#27272a" // Zinc-800
        }
    }

    /**
     * @brief Get border color based on notification priority.
     */
    function getBorderColor()
    {
        if (!root.notification)
            return "#52525b" // Zinc-600

        switch (root.notification.priority)
        {
        case "CRITICAL":
            return "#ef4444" // Red-500
        case "MAJOR":
            return "#f97316" // Orange-500
        case "MINOR":
            return "#eab308" // Yellow-500
        default:
            return "#52525b" // Zinc-600
        }
    }

    /**
     * @brief Get text color based on notification priority.
     */
    function getTextColor()
    {
        if (!root.notification)
            return "#f4f4f5" // Zinc-50

        switch (root.notification.priority)
        {
        case "CRITICAL":
            return "#fca5a5" // Red-300
        case "MAJOR":
            return "#fdba74" // Orange-300
        case "MINOR":
            return "#fde047" // Yellow-300
        default:
            return "#e4e4e7" // Zinc-200
        }
    }

    /**
     * @brief Get priority badge background color.
     */
    function getPriorityBadgeColor()
    {
        if (!root.notification)
            return "#71717a" // Zinc-500

        switch (root.notification.priority)
        {
        case "CRITICAL":
            return "#ef4444" // Red-500
        case "MAJOR":
            return "#f97316" // Orange-500
        case "MINOR":
            return "#eab308" // Yellow-500
        default:
            return "#71717a" // Zinc-500
        }
    }

    /**
     * @brief Format timestamp for display.
     *
     * @param timestampMs: Milliseconds since epoch
     * @return Formatted time string (HH:MM:SS)
     */
    function formatTime(timestampMs)
    {
        var date = new Date(timestampMs)
        var hours = String(date.getHours()).padStart(2, '0')
        var minutes = String(date.getMinutes()).padStart(2, '0')
        var seconds = String(date.getSeconds()).padStart(2, '0')
        return hours + ":" + minutes + ":" + seconds
    }

    /**
     * @brief Calculate auto-dismiss delay.
     *
     * @return Milliseconds until auto-dismiss (0 for CRITICAL)
     */
    function calculateHideDelay()
    {
        if (!root.notification)
            return 5000

        if (root.notification.expiresAt <= 0)
            return 0 // No auto-dismiss

        var now = new Date().getTime()
        var delayMs = Math.max(0, root.notification.expiresAt - now)
        return delayMs
    }
}
