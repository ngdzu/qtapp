/**
 * @file MonitoringService.cpp
 * @brief Implementation of MonitoringService application service.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "application/services/MonitoringService.h"
#include "domain/common/Result.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/monitoring/TelemetryBatch.h"
#include "domain/monitoring/AlarmAggregate.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/repositories/ITelemetryRepository.h"
#include "domain/repositories/IPatientRepository.h"
#include "domain/repositories/IAlarmRepository.h"
#include "domain/repositories/IVitalsRepository.h"
#include "infrastructure/interfaces/ISensorDataSource.h"
#include "infrastructure/logging/LogService.h"
#include <QDebug>

namespace zmon {

MonitoringService::MonitoringService(
    std::shared_ptr<IPatientRepository> patientRepo,
    std::shared_ptr<ITelemetryRepository> telemetryRepo,
    std::shared_ptr<IAlarmRepository> alarmRepo,
    std::shared_ptr<IVitalsRepository> vitalsRepo,
    std::shared_ptr<ISensorDataSource> sensorDataSource,
    QObject* parent)
    : QObject(parent)
    , m_patientRepo(patientRepo)
    , m_telemetryRepo(telemetryRepo)
    , m_alarmRepo(alarmRepo)
    , m_vitalsRepo(vitalsRepo)
    , m_sensorDataSource(sensorDataSource)
    , m_currentPatient(nullptr)
    , m_alarmAggregate(std::make_shared<AlarmAggregate>())
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

void MonitoringService::processVital(const VitalRecord& vital) {
    // Update patient aggregate if patient is admitted
    if (m_currentPatient && m_currentPatient->isAdmitted()) {
        auto updateResult = m_currentPatient->updateVitals(vital);
        if (updateResult.isError()) {
            // Domain layer returns errors only (no logging per guidelines)
            // Log infrastructure failure if needed, but domain errors are expected conditions
            // For now, silently continue - vitals update failure is not critical for processing
            // In production, might want to emit a signal for UI feedback
        }
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
    
    // Persist vital record (infrastructure call - log failures per guidelines)
    if (m_vitalsRepo) {
        auto saveResult = m_vitalsRepo->save(vital);
        if (saveResult.isError()) {
            // Application layer: Log infrastructure failures before continuing
            // Note: LogService would be injected via DI - for now use qWarning as fallback
            // TODO: Inject LogService and use proper logging
            qWarning() << "Failed to save vital record:" 
                       << QString::fromStdString(saveResult.error().message)
                       << "MRN:" << QString::fromStdString(vital.patientMrn);
            // Continue processing - vital was processed, just not persisted
        }
    }
    
    emit vitalProcessed(vital);
}

std::shared_ptr<PatientAggregate> MonitoringService::getCurrentPatient() const {
    return m_currentPatient;
}

void MonitoringService::onVitalReceived(const VitalRecord& vital) {
    processVital(vital);
}

void MonitoringService::createNewBatch() {
    m_currentBatch = std::make_shared<TelemetryBatch>();
    
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
    
    // Persist to repository (infrastructure call - log failures per guidelines)
    if (m_telemetryRepo) {
        auto saveResult = m_telemetryRepo->save(*m_currentBatch);
        if (saveResult.isError()) {
            // Application layer: Log infrastructure failures before continuing
            // TODO: Inject LogService and use proper logging
            qWarning() << "Failed to save telemetry batch:" 
                       << QString::fromStdString(saveResult.error().message)
                       << "Batch ID:" << QString::fromStdString(m_currentBatch->getBatchId());
            // Continue - emit signal anyway for retry mechanism
        }
    }
    
    // Emit signal for network transmission
    emit telemetryBatchReady(QString::fromStdString(m_currentBatch->getBatchId()));
}

void MonitoringService::evaluateAlarms(const VitalRecord& vital) {
    // Business rule: Evaluate alarms based on vital type and thresholds
    // For now, simplified alarm evaluation
    // In real implementation, use AlarmThreshold configuration
    
    // Example: Heart rate alarm
    if (vital.vitalType == "HR") {
        if (vital.value < 60.0 || vital.value > 100.0) {
            AlarmPriority priority = 
                (vital.value < 50.0 || vital.value > 120.0) 
                    ? AlarmPriority::HIGH
                    : AlarmPriority::MEDIUM;
            
            std::string alarmType = (vital.value < 60.0) ? "HR_LOW" : "HR_HIGH";
            double threshold = (vital.value < 60.0) ? 60.0 : 100.0;
            
            auto alarm = m_alarmAggregate->raise(
                alarmType, priority, vital.value, threshold,
                vital.patientMrn, vital.deviceId);
            
            if (alarm.alarmId != "") {
                // Persist alarm (infrastructure call - log failures per guidelines)
                if (m_alarmRepo) {
                    auto saveResult = m_alarmRepo->save(alarm);
                    if (saveResult.isError()) {
                        // Application layer: Log infrastructure failures before continuing
                        // TODO: Inject LogService and use proper logging
                        qWarning() << "Failed to save alarm:" 
                                   << QString::fromStdString(saveResult.error().message)
                                   << "Alarm ID:" << QString::fromStdString(alarm.alarmId);
                        // Continue - emit signal anyway (alarm was raised, just not persisted)
                    }
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

} // namespace zmon