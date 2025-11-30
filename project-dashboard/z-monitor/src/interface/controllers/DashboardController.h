/**
 * @file DashboardController.h
 * @brief QML controller for dashboard UI.
 *
 * This file contains the DashboardController class which provides QML bindings
 * for the main dashboard view including vital signs, alarms, and patient information.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>

namespace zmon
{

    // Forward declarations
    class MonitoringService;
    class VitalsCache;

    /**
     * @class DashboardController
     * @brief QML controller for dashboard UI.
     *
     * Provides QML bindings for the main dashboard view. Exposes vital signs,
     * alarm state, patient information, and dashboard state for the Dashboard View.
     *
     * @note Business logic delegated to MonitoringService
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class DashboardController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString patientName READ patientName NOTIFY patientNameChanged)
        Q_PROPERTY(QString patientMrn READ patientMrn NOTIFY patientMrnChanged)
        Q_PROPERTY(int heartRate READ heartRate NOTIFY heartRateChanged)
        Q_PROPERTY(int spo2 READ spo2 NOTIFY spo2Changed)
        Q_PROPERTY(int respiratoryRate READ respiratoryRate NOTIFY respiratoryRateChanged)
        Q_PROPERTY(QString bloodPressure READ bloodPressure NOTIFY bloodPressureChanged)
        Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
        Q_PROPERTY(bool hasActiveAlarms READ hasActiveAlarms NOTIFY hasActiveAlarmsChanged)
        Q_PROPERTY(bool isMonitoring READ isMonitoring NOTIFY isMonitoringChanged)

    public:
        /**
         * @brief Constructor.
         *
         * @param monitoringService Monitoring service for vital signs and state
         * @param vitalsCache In-memory cache for vitals (accessed for latest values)
         * @param parent Parent QObject
         */
        explicit DashboardController(MonitoringService *monitoringService = nullptr,
                                     VitalsCache *vitalsCache = nullptr,
                                     QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~DashboardController() override = default;

        /**
         * @brief Gets current patient name.
         *
         * @return Patient name, or empty string if no patient admitted
         */
        QString patientName() const { return m_patientName; }

        /**
         * @brief Gets current patient MRN.
         *
         * @return Patient MRN, or empty string if no patient admitted
         */
        QString patientMrn() const { return m_patientMrn; }

        /**
         * @brief Gets current heart rate.
         *
         * @return Heart rate in BPM, or 0 if not available
         */
        int heartRate() const { return m_heartRate; }

        /**
         * @brief Gets current SpO2.
         *
         * @return SpO2 percentage (0-100), or 0 if not available
         */
        int spo2() const { return m_spo2; }

        /**
         * @brief Gets current respiratory rate.
         *
         * @return Respiratory rate in breaths/min, or 0 if not available
         */
        int respiratoryRate() const { return m_respiratoryRate; }

        /**
         * @brief Gets current blood pressure.
         *
         * @return Blood pressure as "systolic/diastolic" (e.g., "120/80"), or empty string if not available
         */
        QString bloodPressure() const { return m_bloodPressure; }

        /**
         * @brief Gets current temperature.
         *
         * @return Temperature in degrees (C or F based on settings), or 0 if not available
         */
        double temperature() const { return m_temperature; }

        /**
         * @brief Gets whether there are active alarms.
         *
         * @return true if active alarms exist, false otherwise
         */
        bool hasActiveAlarms() const { return m_hasActiveAlarms; }

        /**
         * @brief Gets whether monitoring is active.
         *
         * @return true if monitoring is active, false if standby
         */
        bool isMonitoring() const { return m_isMonitoring; }

    public slots:
        /**
         * @brief Handle vital signs updated from MonitoringService.
         *
         * @note Called when MonitoringService emits vitalsUpdated signal
         */
        void onVitalsUpdated();

        /**
         * @brief Handle patient changed from MonitoringService.
         *
         * @note Called when MonitoringService emits patientChanged signal
         */
        void onPatientChanged();

        /**
         * @brief Handle alarm state changed from MonitoringService.
         *
         * @note Called when AlarmManager emits alarmStateChanged signal
         */
        void onAlarmStateChanged();

    signals:
        /**
         * @brief Emitted when patient name changes.
         */
        void patientNameChanged();

        /**
         * @brief Emitted when patient MRN changes.
         */
        void patientMrnChanged();

        /**
         * @brief Emitted when heart rate changes.
         */
        void heartRateChanged();

        /**
         * @brief Emitted when SpO2 changes.
         */
        void spo2Changed();

        /**
         * @brief Emitted when respiratory rate changes.
         */
        void respiratoryRateChanged();

        /**
         * @brief Emitted when blood pressure changes.
         */
        void bloodPressureChanged();

        /**
         * @brief Emitted when temperature changes.
         */
        void temperatureChanged();

        /**
         * @brief Emitted when active alarm state changes.
         */
        void hasActiveAlarmsChanged();

        /**
         * @brief Emitted when monitoring state changes.
         */
        void isMonitoringChanged();

    private:
        MonitoringService *m_monitoringService; ///< Monitoring service (not owned)
        VitalsCache *m_vitalsCache;             ///< Vitals cache (not owned)

        QString m_patientName;   ///< Current patient name
        QString m_patientMrn;    ///< Current patient MRN
        int m_heartRate;         ///< Current heart rate (BPM)
        int m_spo2;              ///< Current SpO2 (percentage)
        int m_respiratoryRate;   ///< Current respiratory rate (breaths/min)
        QString m_bloodPressure; ///< Current blood pressure (systolic/diastolic)
        double m_temperature;    ///< Current temperature
        bool m_hasActiveAlarms;  ///< Whether active alarms exist
        bool m_isMonitoring;     ///< Whether monitoring is active
        int m_activeAlarmCount;  ///< Number of active alarms
    };

} // namespace zmon
