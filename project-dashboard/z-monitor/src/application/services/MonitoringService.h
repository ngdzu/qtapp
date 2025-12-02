/**
 * @file MonitoringService.h
 * @brief Application service coordinating vitals ingestion, telemetry batching, and transmission.
 *
 * This file contains the MonitoringService class which orchestrates the monitoring
 * use case. It coordinates between domain aggregates (PatientAggregate, TelemetryBatch,
 * AlarmAggregate), repositories, and external services (ISensorDataSource).
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace zmon
{

    // Forward declarations
    class PatientAggregate;
    class TelemetryBatch;
    class AlarmAggregate;
    struct AlarmSnapshot;
    class VitalRecord;
    class WaveformSample;
    struct SensorError;
    class ITelemetryRepository;
    class IPatientRepository;
    class IAlarmRepository;
    class IVitalsRepository;
    class ISensorDataSource;
    class VitalsCache;
    class WaveformCache;

    /**
     * @class MonitoringService
     * @brief Application service coordinating vitals ingestion and telemetry transmission.
     *
     * This service orchestrates the monitoring use case:
     * - Receives vitals from sensor data source
     * - Updates patient aggregate with vitals
     * - Evaluates alarm conditions
     * - Batches telemetry data for transmission
     * - Persists data via repositories
     *
     * The service depends on:
     * - Domain aggregates (PatientAggregate, TelemetryBatch, AlarmAggregate)
     * - Repository interfaces (ITelemetryRepository, IPatientRepository, etc.)
     * - External service interfaces (ISensorDataSource)
     *
     * @note Application layer uses Qt Core (QObject, signals/slots) for event coordination.
     * @note Service does not contain infrastructure details - uses interfaces only.
     */
    class MonitoringService : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * @param patientRepo Patient repository
         * @param telemetryRepo Telemetry repository
         * @param alarmRepo Alarm repository
         * @param vitalsRepo Vitals repository
         * @param sensorDataSource Sensor data source
         * @param vitalsCache In-memory cache for vitals (3-day capacity)
         * @param waveformCache In-memory cache for waveforms (30-second capacity)
         * @param parent Parent QObject
         */
        explicit MonitoringService(
            std::shared_ptr<IPatientRepository> patientRepo,
            std::shared_ptr<ITelemetryRepository> telemetryRepo,
            std::shared_ptr<IAlarmRepository> alarmRepo,
            std::shared_ptr<IVitalsRepository> vitalsRepo,
            std::shared_ptr<ISensorDataSource> sensorDataSource,
            std::shared_ptr<VitalsCache> vitalsCache,
            std::shared_ptr<WaveformCache> waveformCache,
            QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~MonitoringService();

        /**
         * @brief Start monitoring.
         *
         * Starts the sensor data source and begins processing vitals.
         *
         * @return true if start succeeded, false otherwise
         */
        bool start();

        /**
         * @brief Stop monitoring.
         *
         * Stops the sensor data source and flushes pending telemetry batches.
         */
        void stop();

        /**
         * @brief Process a vital record.
         *
         * Processes a vital record: updates patient aggregate, evaluates alarms,
         * and adds to telemetry batch.
         *
         * @param vital Vital record to process
         */
        void processVital(const VitalRecord &vital);

        /**
         * @brief Get current patient aggregate.
         *
         * @return Shared pointer to patient aggregate, or nullptr if no patient admitted
         */
        virtual std::shared_ptr<PatientAggregate> getCurrentPatient() const;

        /**
         * @brief Acknowledge an alarm.
         *
         * Marks an alarm as acknowledged in the AlarmAggregate and persists to repository.
         *
         * @param alarmId Alarm identifier
         * @param userId User ID who acknowledged the alarm
         * @return true if acknowledgment succeeded, false otherwise
         */
        bool acknowledgeAlarm(const QString &alarmId, const QString &userId);

        /**
         * @brief Silence an alarm temporarily.
         *
         * Temporarily silences an alarm in the AlarmAggregate.
         *
         * @param alarmId Alarm identifier
         * @param durationMs Silence duration in milliseconds
         * @return true if silence succeeded, false otherwise
         */
        bool silenceAlarm(const QString &alarmId, int64_t durationMs);

        /**
         * @brief Get active alarms.
         *
         * Retrieves all currently active alarms from AlarmAggregate.
         *
         * @return Vector of active alarm snapshots
         */
        std::vector<AlarmSnapshot> getActiveAlarms() const;

        /**
         * @brief Get alarm history.
         *
         * Retrieves alarm history from repository.
         *
         * @param patientMrn Patient MRN (empty for all patients)
         * @param startTimeMs Start time in milliseconds (epoch)
         * @param endTimeMs End time in milliseconds (epoch)
         * @return Vector of alarm snapshots (most recent first)
         */
        std::vector<AlarmSnapshot> getAlarmHistory(const QString &patientMrn, int64_t startTimeMs, int64_t endTimeMs) const;

    signals:
        /**
         * @brief Signal emitted when a vital record is processed.
         *
         * @param vital Processed vital record
         */
        void vitalProcessed(const VitalRecord &vital);

        /**
         * @brief Signal emitted when vitals are updated (for UI controllers).
         *
         * Emitted after vital is processed and cached. Controllers should connect
         * to this signal to update Q_PROPERTY values.
         */
        void vitalsUpdated();

        /**
         * @brief Signal emitted when an alarm is raised.
         *
         * @param alarmId Alarm identifier
         * @param alarmType Alarm type
         * @param priority Alarm priority
         */
        void alarmRaised(const QString &alarmId, const QString &alarmType, int priority);

        /**
         * @brief Signal emitted when an alarm is acknowledged.
         *
         * @param alarmId Alarm identifier
         */
        void alarmAcknowledged(const QString &alarmId);

        /**
         * @brief Signal emitted when an alarm is cleared/resolved.
         *
         * @param alarmId Alarm identifier
         */
        void alarmCleared(const QString &alarmId);

        /**
         * @brief Signal emitted when a telemetry batch is ready for transmission.
         *
         * @param batchId Batch identifier
         */
        void telemetryBatchReady(const QString &batchId);

    private slots:
        /**
         * @brief Slot called when sensor data source emits a vital record.
         *
         * @param vital Vital record from sensor
         */
        void onVitalReceived(const VitalRecord &vital);

        /**
         * @brief Slot called when sensor data source emits a waveform sample.
         *
         * @param sample Waveform sample from sensor
         */
        void onWaveformSampleReceived(const WaveformSample &sample);

        /**
         * @brief Slot called when sensor data source emits an error.
         *
         * @param error Sensor error
         */
        void onSensorError(const SensorError &error);

    private:
        std::shared_ptr<IPatientRepository> m_patientRepo;
        std::shared_ptr<ITelemetryRepository> m_telemetryRepo;
        std::shared_ptr<IAlarmRepository> m_alarmRepo;
        std::shared_ptr<IVitalsRepository> m_vitalsRepo;
        std::shared_ptr<ISensorDataSource> m_sensorDataSource;
        std::shared_ptr<VitalsCache> m_vitalsCache;
        std::shared_ptr<WaveformCache> m_waveformCache;

        std::shared_ptr<PatientAggregate> m_currentPatient;
        std::shared_ptr<AlarmAggregate> m_alarmAggregate;
        std::shared_ptr<TelemetryBatch> m_currentBatch;

        /**
         * @brief Create a new telemetry batch.
         */
        void createNewBatch();

        /**
         * @brief Flush current batch to repository.
         */
        void flushBatch();

        /**
         * @brief Evaluate alarm conditions for a vital record.
         *
         * @param vital Vital record to evaluate
         */
        void evaluateAlarms(const VitalRecord &vital);
    };

} // namespace zmon