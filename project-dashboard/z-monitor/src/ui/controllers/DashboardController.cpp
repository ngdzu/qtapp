/**
 * @file DashboardController.cpp
 * @brief Implementation of DashboardController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "DashboardController.h"
#include "application/services/MonitoringService.h"
#include "infrastructure/caching/VitalsCache.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include <memory>

namespace zmon
{

    DashboardController::DashboardController(MonitoringService *monitoringService,
                                             VitalsCache *vitalsCache,
                                             QObject *parent)
        : QObject(parent),
          m_monitoringService(monitoringService),
          m_vitalsCache(vitalsCache),
          m_patientName(""),
          m_patientMrn(""),
          m_heartRate(0),
          m_spo2(0),
          m_respiratoryRate(0),
          m_bloodPressure(""),
          m_temperature(0.0),
          m_hasActiveAlarms(false),
          m_isMonitoring(false),
          m_activeAlarmCount(0)
    {
        // Connect to MonitoringService signals
        if (m_monitoringService)
        {
            connect(m_monitoringService, &MonitoringService::vitalsUpdated,
                    this, &DashboardController::onVitalsUpdated);
            connect(m_monitoringService, &MonitoringService::alarmRaised,
                    this, &DashboardController::onAlarmStateChanged);
        }

        // Initialize monitoring state - assume started if service exists
        // (MonitoringService is started in main.cpp before controller instantiation)
        if (m_monitoringService)
        {
            m_isMonitoring = true;
            emit isMonitoringChanged();
        }

        // Load initial patient data if available
        onPatientChanged();
    }

    void DashboardController::onVitalsUpdated()
    {
        if (!m_vitalsCache)
        {
            return;
        }

        // Read latest vitals from cache and update properties
        bool found = false;
        auto hrVital = m_vitalsCache->getLatest("HR", found);
        if (found && static_cast<int>(hrVital.value) != m_heartRate)
        {
            m_heartRate = static_cast<int>(hrVital.value);
            emit heartRateChanged();
        }

        auto spo2Vital = m_vitalsCache->getLatest("SPO2", found);
        if (found && static_cast<int>(spo2Vital.value) != m_spo2)
        {
            m_spo2 = static_cast<int>(spo2Vital.value);
            emit spo2Changed();
        }

        auto rrVital = m_vitalsCache->getLatest("RR", found);
        if (found && static_cast<int>(rrVital.value) != m_respiratoryRate)
        {
            m_respiratoryRate = static_cast<int>(rrVital.value);
            emit respiratoryRateChanged();
        }

        auto tempVital = m_vitalsCache->getLatest("TEMP", found);
        if (found && tempVital.value != m_temperature)
        {
            m_temperature = tempVital.value;
            emit temperatureChanged();
        }

        // Blood pressure requires both systolic and diastolic
        bool foundSys = false, foundDia = false;
        auto nibpSysVital = m_vitalsCache->getLatest("NIBP_SYS", foundSys);
        auto nibpDiaVital = m_vitalsCache->getLatest("NIBP_DIA", foundDia);
        if (foundSys && foundDia)
        {
            QString bp = QString("%1/%2")
                             .arg(static_cast<int>(nibpSysVital.value))
                             .arg(static_cast<int>(nibpDiaVital.value));
            if (bp != m_bloodPressure)
            {
                m_bloodPressure = bp;
                emit bloodPressureChanged();
            }
        }
    }
    void DashboardController::onPatientChanged()
    {
        if (!m_monitoringService)
        {
            return;
        }

        // Read current patient from MonitoringService
        auto currentPatient = m_monitoringService->getCurrentPatient();
        if (currentPatient && currentPatient->isAdmitted())
        {
            const auto &identity = currentPatient->getPatientIdentity();
            QString newName = QString::fromStdString(identity.name);
            QString newMrn = QString::fromStdString(identity.mrn);

            if (newName != m_patientName)
            {
                m_patientName = newName;
                emit patientNameChanged();
            }

            if (newMrn != m_patientMrn)
            {
                m_patientMrn = newMrn;
                emit patientMrnChanged();
            }
        }
        else
        {
            // No patient admitted - clear patient info
            if (!m_patientName.isEmpty())
            {
                m_patientName = "";
                emit patientNameChanged();
            }
            if (!m_patientMrn.isEmpty())
            {
                m_patientMrn = "";
                emit patientMrnChanged();
            }
        }
    }

    void DashboardController::onAlarmStateChanged()
    {
        // This slot is called when alarmRaised signal is emitted
        // Increment active alarm count and update hasActiveAlarms flag
        m_activeAlarmCount++;

        bool hadAlarms = m_hasActiveAlarms;
        m_hasActiveAlarms = (m_activeAlarmCount > 0);

        if (m_hasActiveAlarms != hadAlarms)
        {
            emit hasActiveAlarmsChanged();
        }

        // Note: This is a simplified implementation that only tracks raised alarms.
        // A complete implementation would:
        // - Track alarm acknowledgments (decrement count)
        // - Query AlarmAggregate for current active alarm count
        // - Connect to additional alarm signals (alarmCleared, alarmAcknowledged)
        // This will be improved when AlarmController is implemented (task 45e-3)
    }

} // namespace zmon
