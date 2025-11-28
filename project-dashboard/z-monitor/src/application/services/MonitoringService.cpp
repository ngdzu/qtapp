/**
 * @file MonitoringService.cpp
 * @brief Implementation of MonitoringService application service.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "application/services/MonitoringService.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/monitoring/TelemetryBatch.h"
#include "domain/monitoring/AlarmAggregate.h"
#include "domain/monitoring/VitalRecord.h"

namespace ZMonitor {
namespace Application {
namespace Services {

MonitoringService::MonitoringService(
    std::shared_ptr<Domain::Repositories::IPatientRepository> patientRepo,
    std::shared_ptr<Domain::Repositories::ITelemetryRepository> telemetryRepo,
    std::shared_ptr<Domain::Repositories::IAlarmRepository> alarmRepo,
    std::shared_ptr<Domain::Repositories::IVitalsRepository> vitalsRepo,
    std::shared_ptr<ISensorDataSource> sensorDataSource,
    QObject* parent)
    : QObject(parent)
    , m_patientRepo(patientRepo)
    , m_telemetryRepo(telemetryRepo)
    , m_alarmRepo(alarmRepo)
    , m_vitalsRepo(vitalsRepo)
    , m_sensorDataSource(sensorDataSource)
    , m_currentPatient(nullptr)
    , m_alarmAggregate(std::make_shared<Domain::Monitoring::AlarmAggregate>())
    , m_currentBatch(nullptr)
{
    // Note: Connect to sensor data source signals when ISensorDataSource interface is defined
}

MonitoringService::~MonitoringService() = default;

bool MonitoringService::start() {
    // Start sensor data source
    // Note: Implementation depends on ISensorDataSource interface
    // if (m_sensorDataSource && !m_sensorDataSource->start()) {
    //     return false;
    // }
    
    // Create initial telemetry batch
    createNewBatch();
    
    return true;
}

void MonitoringService::stop() {
    // Flush pending batch
    if (m_currentBatch && !m_currentBatch->getVitals().empty()) {
        flushBatch();
    }
    
    // Stop sensor data source
    // if (m_sensorDataSource) {
    //     m_sensorDataSource->stop();
    // }
}

void MonitoringService::processVital(const Domain::Monitoring::VitalRecord& vital) {
    // Update patient aggregate if patient is admitted
    if (m_currentPatient && m_currentPatient->isAdmitted()) {
        m_currentPatient->updateVitals(vital);
    }
    
    // Evaluate alarm conditions
    evaluateAlarms(vital);
    
    // Add to current telemetry batch
    if (m_currentBatch) {
        m_currentBatch->addVital(vital);
        
        // Flush batch if it's getting large
        if (m_currentBatch->getVitals().size() >= 100) {
            flushBatch();
            createNewBatch();
        }
    }
    
    // Persist vital record
    if (m_vitalsRepo) {
        m_vitalsRepo->save(vital);
    }
    
    emit vitalProcessed(vital);
}

std::shared_ptr<Domain::Monitoring::PatientAggregate> MonitoringService::getCurrentPatient() const {
    return m_currentPatient;
}

void MonitoringService::onVitalReceived(const Domain::Monitoring::VitalRecord& vital) {
    processVital(vital);
}

void MonitoringService::createNewBatch() {
    m_currentBatch = std::make_shared<Domain::Monitoring::TelemetryBatch>();
    
    // Set patient MRN if patient is admitted
    if (m_currentPatient && m_currentPatient->isAdmitted()) {
        m_currentBatch->setPatientMrn(m_currentPatient->getPatientMrn());
    }
}

void MonitoringService::flushBatch() {
    if (!m_currentBatch || m_currentBatch->getVitals().empty()) {
        return;
    }
    
    // Sign batch (in real implementation, use SignatureService)
    // For now, use placeholder signature
    m_currentBatch->sign("placeholder_signature");
    
    // Validate batch
    if (!m_currentBatch->validate()) {
        return;
    }
    
    // Persist to repository
    if (m_telemetryRepo) {
        m_telemetryRepo->save(*m_currentBatch);
    }
    
    // Emit signal for network transmission
    emit telemetryBatchReady(QString::fromStdString(m_currentBatch->getBatchId()));
}

void MonitoringService::evaluateAlarms(const Domain::Monitoring::VitalRecord& vital) {
    // Business rule: Evaluate alarms based on vital type and thresholds
    // For now, simplified alarm evaluation
    // In real implementation, use AlarmThreshold configuration
    
    // Example: Heart rate alarm
    if (vital.vitalType == "HR") {
        if (vital.value < 60.0 || vital.value > 100.0) {
            Domain::Monitoring::AlarmPriority priority = 
                (vital.value < 50.0 || vital.value > 120.0) 
                    ? Domain::Monitoring::AlarmPriority::HIGH
                    : Domain::Monitoring::AlarmPriority::MEDIUM;
            
            std::string alarmType = (vital.value < 60.0) ? "HR_LOW" : "HR_HIGH";
            double threshold = (vital.value < 60.0) ? 60.0 : 100.0;
            
            auto alarm = m_alarmAggregate->raise(
                alarmType, priority, vital.value, threshold,
                vital.patientMrn, vital.deviceId);
            
            if (alarm.alarmId != "") {
                // Persist alarm
                if (m_alarmRepo) {
                    m_alarmRepo->save(alarm);
                }
                
                // Emit signal
                emit alarmRaised(
                    QString::fromStdString(alarm.alarmId),
                    QString::fromStdString(alarm.alarmType),
                    static_cast<int>(alarm.priority));
            }
        }
    }
}

} // namespace Services
} // namespace Application
} // namespace ZMonitor

