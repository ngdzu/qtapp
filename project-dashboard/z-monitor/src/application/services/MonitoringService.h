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

namespace ZMonitor {
namespace Domain {
namespace Monitoring {
    class PatientAggregate;
    class TelemetryBatch;
    class AlarmAggregate;
    class VitalRecord;
}
namespace Repositories {
    class ITelemetryRepository;
    class IPatientRepository;
    class IAlarmRepository;
    class IVitalsRepository;
}
}

namespace Application {
namespace Services {

// Forward declaration for external service interface
namespace Infrastructure {
namespace Interfaces {
    class ISensorDataSource;
}
}

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
class MonitoringService : public QObject {
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
     * @param parent Parent QObject
     */
    explicit MonitoringService(
        std::shared_ptr<Domain::Repositories::IPatientRepository> patientRepo,
        std::shared_ptr<Domain::Repositories::ITelemetryRepository> telemetryRepo,
        std::shared_ptr<Domain::Repositories::IAlarmRepository> alarmRepo,
        std::shared_ptr<Domain::Repositories::IVitalsRepository> vitalsRepo,
        std::shared_ptr<Infrastructure::Interfaces::ISensorDataSource> sensorDataSource,
        QObject* parent = nullptr);
    
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
    void processVital(const Domain::Monitoring::VitalRecord& vital);
    
    /**
     * @brief Get current patient aggregate.
     * 
     * @return Shared pointer to patient aggregate, or nullptr if no patient admitted
     */
    std::shared_ptr<Domain::Monitoring::PatientAggregate> getCurrentPatient() const;

signals:
    /**
     * @brief Signal emitted when a vital record is processed.
     * 
     * @param vital Processed vital record
     */
    void vitalProcessed(const Domain::Monitoring::VitalRecord& vital);
    
    /**
     * @brief Signal emitted when an alarm is raised.
     * 
     * @param alarmId Alarm identifier
     * @param alarmType Alarm type
     * @param priority Alarm priority
     */
    void alarmRaised(const QString& alarmId, const QString& alarmType, int priority);
    
    /**
     * @brief Signal emitted when a telemetry batch is ready for transmission.
     * 
     * @param batchId Batch identifier
     */
    void telemetryBatchReady(const QString& batchId);

private slots:
    /**
     * @brief Slot called when sensor data source emits a vital record.
     * 
     * @param vital Vital record from sensor
     */
    void onVitalReceived(const Domain::Monitoring::VitalRecord& vital);

private:
    std::shared_ptr<Domain::Repositories::IPatientRepository> m_patientRepo;
    std::shared_ptr<Domain::Repositories::ITelemetryRepository> m_telemetryRepo;
    std::shared_ptr<Domain::Repositories::IAlarmRepository> m_alarmRepo;
    std::shared_ptr<Domain::Repositories::IVitalsRepository> m_vitalsRepo;
    std::shared_ptr<Infrastructure::Interfaces::ISensorDataSource> m_sensorDataSource;
    
    std::shared_ptr<Domain::Monitoring::PatientAggregate> m_currentPatient;
    std::shared_ptr<Domain::Monitoring::AlarmAggregate> m_alarmAggregate;
    std::shared_ptr<Domain::Monitoring::TelemetryBatch> m_currentBatch;
    
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
    void evaluateAlarms(const Domain::Monitoring::VitalRecord& vital);
};

} // namespace Services
} // namespace Application
} // namespace ZMonitor

