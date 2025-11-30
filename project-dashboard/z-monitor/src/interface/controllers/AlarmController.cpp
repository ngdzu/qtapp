/**
 * @file AlarmController.cpp
 * @brief Implementation of AlarmController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "AlarmController.h"
#include "application/services/MonitoringService.h"
#include "domain/monitoring/AlarmSnapshot.h"
#include <QVariantMap>
#include <QDateTime>

namespace zmon
{

    AlarmController::AlarmController(MonitoringService *monitoringService, QObject *parent)
        : QObject(parent), m_monitoringService(monitoringService), m_hasCriticalAlarms(false), m_hasWarningAlarms(false)
    {
        // Connect to MonitoringService signals if provided
        if (m_monitoringService)
        {
            connect(m_monitoringService, &MonitoringService::alarmRaised, this, &AlarmController::onAlarmTriggered);
            connect(m_monitoringService, &MonitoringService::alarmAcknowledged, this, &AlarmController::onAlarmAcknowledged);
            connect(m_monitoringService, &MonitoringService::alarmCleared, this, &AlarmController::onAlarmCleared);

            // Load initial active alarms
            updateActiveAlarms();
        }
    }

    void AlarmController::acknowledgeAlarm(const QString &alarmId)
    {
        if (!m_monitoringService)
        {
            return;
        }

        // TODO: Add permission check (requires SecurityService integration)
        // For now, use placeholder user ID
        QString userId = "system"; // Replace with actual user ID from SecurityService

        bool success = m_monitoringService->acknowledgeAlarm(alarmId, userId);

        if (success)
        {
            // Active alarms will be updated via onAlarmAcknowledged slot
        }
    }

    void AlarmController::silenceAlarm(const QString &alarmId, int durationSeconds)
    {
        if (!m_monitoringService)
        {
            return;
        }

        // TODO: Add permission check based on duration (requires SecurityService integration)
        // Long silences may require higher permissions

        int64_t durationMs = static_cast<int64_t>(durationSeconds) * 1000;
        bool success = m_monitoringService->silenceAlarm(alarmId, durationMs);

        if (success)
        {
            // Update active alarms to reflect silenced state
            updateActiveAlarms();
        }
    }

    void AlarmController::acknowledgeAllAlarms()
    {
        if (!m_monitoringService)
        {
            return;
        }

        // TODO: Add permission check (requires SecurityService integration)

        // Get all active alarms and acknowledge each one
        for (const auto &alarm : m_activeAlarms)
        {
            QVariantMap alarmMap = alarm.toMap();
            QString alarmId = alarmMap["id"].toString();
            acknowledgeAlarm(alarmId);
        }
    }

    void AlarmController::loadAlarmHistory()
    {
        if (!m_monitoringService)
        {
            return;
        }

        // Load last 24 hours of alarm history
        int64_t endTimeMs = QDateTime::currentMSecsSinceEpoch();
        int64_t startTimeMs = endTimeMs - (24 * 60 * 60 * 1000); // 24 hours ago

        // Get history for all patients (empty MRN)
        auto history = m_monitoringService->getAlarmHistory("", startTimeMs, endTimeMs);

        // Convert to QVariantList
        m_alarmHistory.clear();
        for (const auto &alarm : history)
        {
            m_alarmHistory.append(alarmSnapshotToVariantMap(alarm));
        }

        emit alarmHistoryChanged();
    }

    void AlarmController::onAlarmTriggered()
    {
        // Update active alarms from MonitoringService
        updateActiveAlarms();
    }

    void AlarmController::onAlarmAcknowledged()
    {
        // Update active alarms to reflect acknowledged state
        updateActiveAlarms();
    }

    void AlarmController::onAlarmCleared()
    {
        // Update active alarms to remove cleared alarm
        updateActiveAlarms();
    }

    void AlarmController::updateActiveAlarms()
    {
        if (!m_monitoringService)
        {
            return;
        }

        // Get active alarms from MonitoringService
        auto activeAlarms = m_monitoringService->getActiveAlarms();

        // Convert to QVariantList
        m_activeAlarms.clear();
        bool hasCritical = false;
        bool hasWarning = false;

        for (const auto &alarm : activeAlarms)
        {
            m_activeAlarms.append(alarmSnapshotToVariantMap(alarm));

            // Track critical and warning alarms
            if (alarm.priority == AlarmPriority::HIGH)
            {
                hasCritical = true;
            }
            else if (alarm.priority == AlarmPriority::MEDIUM)
            {
                hasWarning = true;
            }
        }

        // Update flags
        bool criticalChanged = (m_hasCriticalAlarms != hasCritical);
        bool warningChanged = (m_hasWarningAlarms != hasWarning);

        m_hasCriticalAlarms = hasCritical;
        m_hasWarningAlarms = hasWarning;

        // Emit signals
        emit activeAlarmsChanged();
        emit activeAlarmCountChanged();

        if (criticalChanged)
        {
            emit hasCriticalAlarmsChanged();
        }

        if (warningChanged)
        {
            emit hasWarningAlarmsChanged();
        }
    }

    QVariantMap AlarmController::alarmSnapshotToVariantMap(const AlarmSnapshot &alarm) const
    {
        QVariantMap map;
        map["id"] = QString::fromStdString(alarm.alarmId);
        map["type"] = QString::fromStdString(alarm.alarmType);
        map["priority"] = static_cast<int>(alarm.priority);
        map["status"] = static_cast<int>(alarm.status);
        map["value"] = alarm.value;
        map["threshold"] = alarm.thresholdValue;
        map["timestamp"] = alarm.timestampMs;
        map["patientMrn"] = QString::fromStdString(alarm.patientMrn);
        map["deviceId"] = QString::fromStdString(alarm.deviceId);
        map["acknowledgedBy"] = QString::fromStdString(alarm.acknowledgedBy);

        // Format message (e.g., "Heart Rate HIGH: 125 bpm (threshold: 100)")
        QString message;
        if (alarm.alarmType.find("HR") != std::string::npos)
        {
            message = QString("Heart Rate %1: %2 bpm (threshold: %3)")
                          .arg(alarm.alarmType.find("HIGH") != std::string::npos ? "HIGH" : "LOW")
                          .arg(alarm.value, 0, 'f', 0)
                          .arg(alarm.thresholdValue, 0, 'f', 0);
        }
        else if (alarm.alarmType.find("SPO2") != std::string::npos)
        {
            message = QString("SpO2 LOW: %1% (threshold: %2%)")
                          .arg(alarm.value, 0, 'f', 0)
                          .arg(alarm.thresholdValue, 0, 'f', 0);
        }
        else
        {
            message = QString::fromStdString(alarm.alarmType) + QString(" alarm");
        }

        map["message"] = message;

        return map;
    }

} // namespace zmon
