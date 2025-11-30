/**
 * @file AlarmController.h
 * @brief QML controller for alarm management UI.
 *
 * This file contains the AlarmController class which provides QML bindings
 * for alarm display, acknowledgment, silencing, and history.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QVariantList>

namespace zmon
{

    // Forward declarations
    class MonitoringService;
    struct AlarmSnapshot;

    /**
     * @class AlarmController
     * @brief QML controller for alarm management UI.
     *
     * Provides QML bindings for alarm management. Exposes alarm list,
     * acknowledgment methods, and alarm history for the Alarm View.
     *
     * @note Business logic delegated to MonitoringService
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class AlarmController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList activeAlarms READ activeAlarms NOTIFY activeAlarmsChanged)
        Q_PROPERTY(int activeAlarmCount READ activeAlarmCount NOTIFY activeAlarmCountChanged)
        Q_PROPERTY(bool hasCriticalAlarms READ hasCriticalAlarms NOTIFY hasCriticalAlarmsChanged)
        Q_PROPERTY(bool hasWarningAlarms READ hasWarningAlarms NOTIFY hasWarningAlarmsChanged)
        Q_PROPERTY(QVariantList alarmHistory READ alarmHistory NOTIFY alarmHistoryChanged)

    public:
        /**
         * @brief Constructor.
         *
         * @param monitoringService Monitoring service for alarm state
         * @param parent Parent QObject
         */
        explicit AlarmController(MonitoringService *monitoringService = nullptr, QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~AlarmController() override = default;

        /**
         * @brief Gets list of active alarms.
         *
         * @return List of alarm objects (each with id, type, priority, message, timestamp)
         */
        QVariantList activeAlarms() const { return m_activeAlarms; }

        /**
         * @brief Gets count of active alarms.
         *
         * @return Number of active alarms
         */
        int activeAlarmCount() const { return m_activeAlarms.size(); }

        /**
         * @brief Gets whether there are critical alarms.
         *
         * @return true if critical alarms exist, false otherwise
         */
        bool hasCriticalAlarms() const { return m_hasCriticalAlarms; }

        /**
         * @brief Gets whether there are warning alarms.
         *
         * @return true if warning alarms exist, false otherwise
         */
        bool hasWarningAlarms() const { return m_hasWarningAlarms; }

        /**
         * @brief Gets alarm history.
         *
         * @return List of historical alarms (last 100)
         */
        QVariantList alarmHistory() const { return m_alarmHistory; }

    public slots:
        /**
         * @brief Handle alarm triggered from AlarmManager.
         *
         * @note Called when AlarmManager emits alarmTriggered signal
         */
        void onAlarmTriggered();

        /**
         * @brief Handle alarm acknowledged from AlarmManager.
         *
         * @note Called when AlarmManager emits alarmAcknowledged signal
         */
        void onAlarmAcknowledged();

        /**
         * @brief Handle alarm cleared from AlarmManager.
         *
         * @note Called when AlarmManager emits alarmCleared signal
         */
        void onAlarmCleared();

    public:
        /**
         * @brief Acknowledge alarm by ID.
         *
         * @param alarmId Alarm identifier
         *
         * @note This method is callable from QML
         * @note Requires user login (permission check)
         */
        Q_INVOKABLE void acknowledgeAlarm(const QString &alarmId);

        /**
         * @brief Silence alarm by ID for duration.
         *
         * @param alarmId Alarm identifier
         * @param durationSeconds Duration to silence in seconds
         *
         * @note This method is callable from QML
         * @note Requires user login with appropriate permissions
         */
        Q_INVOKABLE void silenceAlarm(const QString &alarmId, int durationSeconds);

        /**
         * @brief Acknowledge all active alarms.
         *
         * @note This method is callable from QML
         * @note Requires user login (permission check)
         */
        Q_INVOKABLE void acknowledgeAllAlarms();

        /**
         * @brief Load alarm history.
         *
         * @note This method is callable from QML
         */
        Q_INVOKABLE void loadAlarmHistory();

    signals:
        /**
         * @brief Emitted when active alarms list changes.
         */
        void activeAlarmsChanged();

        /**
         * @brief Emitted when active alarm count changes.
         */
        void activeAlarmCountChanged();

        /**
         * @brief Emitted when critical alarm state changes.
         */
        void hasCriticalAlarmsChanged();

        /**
         * @brief Emitted when warning alarm state changes.
         */
        void hasWarningAlarmsChanged();

        /**
         * @brief Emitted when alarm history changes.
         */
        void alarmHistoryChanged();

    private:
        /**
         * @brief Update active alarms from MonitoringService.
         *
         * Retrieves active alarms from MonitoringService and updates properties.
         */
        void updateActiveAlarms();

        /**
         * @brief Convert AlarmSnapshot to QVariantMap for QML.
         *
         * @param alarm Alarm snapshot
         * @return QVariantMap with alarm data
         */
        QVariantMap alarmSnapshotToVariantMap(const AlarmSnapshot &alarm) const;

        MonitoringService *m_monitoringService; ///< Monitoring service (not owned)

        QVariantList m_activeAlarms; ///< List of active alarms
        bool m_hasCriticalAlarms;    ///< Whether critical alarms exist
        bool m_hasWarningAlarms;     ///< Whether warning alarms exist
        QVariantList m_alarmHistory; ///< Alarm history (last 100)
    };

} // namespace zmon
