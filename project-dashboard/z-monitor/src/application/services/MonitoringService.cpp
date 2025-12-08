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
#include "domain/monitoring/AlarmThreshold.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"
#include "domain/repositories/ITelemetryRepository.h"
#include "domain/repositories/IPatientRepository.h"
#include "domain/repositories/IAlarmRepository.h"
#include "domain/repositories/IVitalsRepository.h"
#include "domain/events/DomainEventDispatcher.h"
#include "domain/monitoring/events/AlarmRaised.h"
#include "infrastructure/interfaces/ISensorDataSource.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include "infrastructure/logging/LogService.h"
#include <QDebug>

namespace zmon
{

    MonitoringService::MonitoringService(
        std::shared_ptr<IPatientRepository> patientRepo,
        std::shared_ptr<ITelemetryRepository> telemetryRepo,
        std::shared_ptr<IAlarmRepository> alarmRepo,
        std::shared_ptr<IVitalsRepository> vitalsRepo,
        std::shared_ptr<ISensorDataSource> sensorDataSource,
        std::shared_ptr<VitalsCache> vitalsCache,
        std::shared_ptr<WaveformCache> waveformCache,
        std::shared_ptr<DomainEventDispatcher> eventDispatcher,
        QObject *parent)
        : QObject(parent),
          m_patientRepo(patientRepo),
          m_telemetryRepo(telemetryRepo),
          m_alarmRepo(alarmRepo),
          m_vitalsRepo(vitalsRepo),
          m_sensorDataSource(sensorDataSource),
          m_vitalsCache(vitalsCache),
          m_waveformCache(waveformCache),
          m_eventDispatcher(eventDispatcher),
          m_currentPatient(nullptr),
          m_alarmAggregate(std::make_shared<AlarmAggregate>()),
          m_currentBatch(nullptr),
          m_lastAlarmDetectionLatencyMs(0)
    {
        // Connect to sensor data source signals
        if (m_sensorDataSource)
        {
            connect(m_sensorDataSource.get(), &ISensorDataSource::vitalSignsReceived,
                    this, &MonitoringService::onVitalReceived);
            connect(m_sensorDataSource.get(), &ISensorDataSource::waveformSampleReceived,
                    this, &MonitoringService::onWaveformSampleReceived);
            connect(m_sensorDataSource.get(), &ISensorDataSource::sensorError,
                    this, &MonitoringService::onSensorError);
        }

        // Initialize default alarm thresholds
        // TODO: Load from configuration file/database
        m_alarmThresholds.emplace("HR", AlarmThreshold("HR", 50.0, 120.0, 5.0, AlarmPriority::HIGH, true));
        m_alarmThresholds.emplace("SPO2", AlarmThreshold("SPO2", 90.0, 100.0, 2.0, AlarmPriority::HIGH, true));
        m_alarmThresholds.emplace("RR", AlarmThreshold("RR", 8.0, 30.0, 2.0, AlarmPriority::MEDIUM, true));
    }

    MonitoringService::~MonitoringService() = default;

    bool MonitoringService::start()
    {

        // Start sensor data source
        if (m_sensorDataSource)
        {
            auto startResult = m_sensorDataSource->start();

            if (startResult.isError())
            {
                qWarning() << "Failed to start sensor data source:"
                           << QString::fromStdString(startResult.error().message);
                return false;
            }
        }
        else
        {
        }

        // Create initial telemetry batch
        createNewBatch();

        qDebug() << "MonitoringService started successfully";
        return true;
    }

    void MonitoringService::stop()
    {
        // Flush pending batch
        if (m_currentBatch && !m_currentBatch->getVitals().empty())
        {
            flushBatch();
        }

        // Stop sensor data source
        if (m_sensorDataSource)
        {
            m_sensorDataSource->stop();
        }

        qDebug() << "MonitoringService stopped";
    }

    void MonitoringService::processVital(const VitalRecord &vital)
    {
        // Update patient aggregate if patient is admitted
        if (m_currentPatient && m_currentPatient->isAdmitted())
        {
            auto updateResult = m_currentPatient->updateVitals(vital);
            if (updateResult.isError())
            {
                // Domain layer returns errors only (no logging per guidelines)
                // Log infrastructure failure if needed, but domain errors are expected conditions
                // For now, silently continue - vitals update failure is not critical for processing
                // In production, might want to emit a signal for UI feedback
            }
        }

        // Evaluate alarm conditions
        evaluateAlarms(vital);

        // Add to current telemetry batch
        if (m_currentBatch)
        {
            m_currentBatch->addVital(vital);

            // Flush batch if it's getting large
            if (m_currentBatch->getVitals().size() >= 100)
            {
                flushBatch();
                createNewBatch();
            }
        }

        // Persist vital record (infrastructure call - log failures per guidelines)
        if (m_vitalsRepo)
        {
            auto saveResult = m_vitalsRepo->save(vital);
            if (saveResult.isError())
            {
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

    std::shared_ptr<PatientAggregate> MonitoringService::getCurrentPatient() const
    {
        return m_currentPatient;
    }

    void MonitoringService::onVitalReceived(const VitalRecord &vital)
    {
        // Cache vital for UI display
        if (m_vitalsCache)
        {
            m_vitalsCache->append(vital);
        }

        // Process vital through normal flow (domain logic, alarms, persistence)
        processVital(vital);

        // Emit signal for UI controllers
        emit vitalsUpdated();
    }

    void MonitoringService::onWaveformSampleReceived(const WaveformSample &sample)
    {
        // Cache waveform sample for UI display
        if (m_waveformCache)
        {
            m_waveformCache->append(sample);
        }

        // Note: Waveforms are display-only, not persisted or processed
    }

    void MonitoringService::onSensorError(const SensorError &error)
    {
        qWarning() << "Sensor error occurred:"
                   << error.message
                   << "Type:" << error.sensorType
                   << "Code:" << static_cast<int>(error.code)
                   << "Recoverable:" << error.recoverable;

        // TODO: Emit signal for UI notification
        // TODO: Handle non-recoverable errors (attempt reconnection, etc.)
    }

    void MonitoringService::createNewBatch()
    {
        m_currentBatch = std::make_shared<TelemetryBatch>();

        // Set patient MRN if patient is admitted
        if (m_currentPatient && m_currentPatient->isAdmitted())
        {
            m_currentBatch->setPatientMrn(m_currentPatient->getPatientMrn());
        }
    }

    void MonitoringService::flushBatch()
    {
        if (!m_currentBatch || m_currentBatch->getVitals().empty())
        {
            return;
        }

        // Sign batch (in real implementation, use SignatureService)
        // For now, use placeholder signature
        m_currentBatch->sign("placeholder_signature");

        // Validate batch
        if (!m_currentBatch->validate())
        {
            return;
        }

        // Persist to repository (infrastructure call - log failures per guidelines)
        if (m_telemetryRepo)
        {
            auto saveResult = m_telemetryRepo->save(*m_currentBatch);
            if (saveResult.isError())
            {
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

    void MonitoringService::evaluateAlarms(const VitalRecord &vital)
    {
        // Start performance measurement
        m_alarmDetectionTimer.start();

        // Get threshold configuration for this vital type
        auto it = m_alarmThresholds.find(vital.vitalType);
        if (it == m_alarmThresholds.end())
        {
            // No threshold configured for this vital type - skip evaluation
            m_lastAlarmDetectionLatencyMs = m_alarmDetectionTimer.elapsed();
            return;
        }

        const AlarmThreshold &threshold = it->second;

        // Skip if threshold is disabled
        if (!threshold.enabled)
        {
            m_lastAlarmDetectionLatencyMs = m_alarmDetectionTimer.elapsed();
            return;
        }

        // Check if vital violates threshold (low or high)
        bool violated = false;
        std::string alarmType;
        double thresholdValue = 0.0;

        if (vital.value < threshold.lowLimit)
        {
            violated = true;
            alarmType = vital.vitalType + "_LOW";
            thresholdValue = threshold.lowLimit;
        }
        else if (vital.value > threshold.highLimit)
        {
            violated = true;
            alarmType = vital.vitalType + "_HIGH";
            thresholdValue = threshold.highLimit;
        }

        if (!violated)
        {
            m_lastAlarmDetectionLatencyMs = m_alarmDetectionTimer.elapsed();
            return;
        }

        // Raise alarm using AlarmAggregate
        auto alarm = m_alarmAggregate->raise(
            alarmType, threshold.priority, vital.value, thresholdValue,
            vital.patientMrn, vital.deviceId);

        if (alarm.alarmId != "")
        {
            // Persist alarm (infrastructure call - log failures per guidelines)
            if (m_alarmRepo)
            {
                auto saveResult = m_alarmRepo->save(alarm);
                if (saveResult.isError())
                {
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

            // Dispatch domain event
            if (m_eventDispatcher)
            {
                m_eventDispatcher->dispatch(zmon::Events::AlarmRaised(alarm, alarm.timestampMs));
            }
        }

        // Record latency measurement
        m_lastAlarmDetectionLatencyMs = m_alarmDetectionTimer.elapsed();
    }

    bool MonitoringService::acknowledgeAlarm(const QString &alarmId, const QString &userId)
    {
        if (!m_alarmAggregate)
        {
            return false;
        }

        // Acknowledge in domain aggregate
        bool success = m_alarmAggregate->acknowledge(alarmId.toStdString(), userId.toStdString());

        if (success)
        {
            // Update status in repository
            if (m_alarmRepo)
            {
                auto updateResult = m_alarmRepo->updateStatus(
                    alarmId.toStdString(),
                    AlarmStatus::Acknowledged,
                    userId.toStdString());

                if (updateResult.isError())
                {
                    qWarning() << "Failed to update alarm status in repository:"
                               << QString::fromStdString(updateResult.error().message);
                    // Continue - alarm is acknowledged in memory, repository update failed
                }
            }

            // Emit signal
            emit alarmAcknowledged(alarmId);
        }

        return success;
    }

    bool MonitoringService::silenceAlarm(const QString &alarmId, int64_t durationMs)
    {
        if (!m_alarmAggregate)
        {
            return false;
        }

        // Silence in domain aggregate
        bool success = m_alarmAggregate->silence(alarmId.toStdString(), durationMs);

        if (success)
        {
            // Update status in repository
            if (m_alarmRepo)
            {
                auto updateResult = m_alarmRepo->updateStatus(
                    alarmId.toStdString(),
                    AlarmStatus::Silenced,
                    ""); // No user ID for silence

                if (updateResult.isError())
                {
                    qWarning() << "Failed to update alarm status in repository:"
                               << QString::fromStdString(updateResult.error().message);
                    // Continue - alarm is silenced in memory, repository update failed
                }
            }

            // Note: No signal emitted for silence (not needed by UI currently)
        }

        return success;
    }

    std::vector<AlarmSnapshot> MonitoringService::getActiveAlarms() const
    {
        if (!m_alarmAggregate)
        {
            return {};
        }

        return m_alarmAggregate->getActiveAlarms();
    }

    std::vector<AlarmSnapshot> MonitoringService::getAlarmHistory(const QString &patientMrn, int64_t startTimeMs, int64_t endTimeMs) const
    {
        if (!m_alarmRepo)
        {
            return {};
        }

        return m_alarmRepo->getHistory(patientMrn.toStdString(), startTimeMs, endTimeMs);
    }

    void MonitoringService::setAlarmThreshold(const AlarmThreshold &threshold)
    {
        // Erase existing threshold for this vital type (if any)
        m_alarmThresholds.erase(threshold.vitalType);
        // Insert new threshold
        m_alarmThresholds.emplace(threshold.vitalType, threshold);
    }

    const AlarmThreshold *MonitoringService::getAlarmThreshold(const std::string &vitalType) const
    {
        auto it = m_alarmThresholds.find(vitalType);
        if (it == m_alarmThresholds.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    int64_t MonitoringService::getLastAlarmDetectionLatencyMs() const
    {
        return m_lastAlarmDetectionLatencyMs;
    }

} // namespace zmon