/**
 * @file DashboardController.cpp
 * @brief Implementation of DashboardController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "DashboardController.h"
#include "application/services/MonitoringService.h"

namespace zmon
{

    DashboardController::DashboardController(MonitoringService *monitoringService, QObject *parent)
        : QObject(parent), m_monitoringService(monitoringService), m_patientName(""), m_patientMrn(""), m_heartRate(0), m_spo2(0), m_respiratoryRate(0), m_bloodPressure(""), m_temperature(0.0), m_hasActiveAlarms(false), m_isMonitoring(false)
    {
        // Connect to MonitoringService signals if provided
        if (m_monitoringService)
        {
            // TODO: Connect signals once MonitoringService is fully implemented
            // connect(m_monitoringService, &MonitoringService::vitalsUpdated, this, &DashboardController::onVitalsUpdated);
            // connect(m_monitoringService, &MonitoringService::patientChanged, this, &DashboardController::onPatientChanged);
        }
    }

    void DashboardController::onVitalsUpdated()
    {
        // TODO: Update vital signs from MonitoringService
        // For now, emit change signals to notify QML
        emit heartRateChanged();
        emit spo2Changed();
        emit respiratoryRateChanged();
        emit bloodPressureChanged();
        emit temperatureChanged();
    }

    void DashboardController::onPatientChanged()
    {
        // TODO: Update patient information from MonitoringService
        emit patientNameChanged();
        emit patientMrnChanged();
    }

    void DashboardController::onAlarmStateChanged()
    {
        // TODO: Update alarm state from AlarmManager
        emit hasActiveAlarmsChanged();
    }

} // namespace zmon
