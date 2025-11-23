#include "DashboardController.h"

DashboardController::DashboardController(QObject *parent)
    : QObject(parent), m_service(nullptr)
{
    // Initialize with default values
    m_currentStats = {0, 0, 0, 0, false};

    // Fill history with zeros initially
    for (int i = 0; i < MAX_HISTORY; ++i)
    {
        m_hrHistory.append(0);
        m_o2History.append(0);
    }
}

void DashboardController::setService(IDeviceDataService *service)
{
    if (m_service)
    {
        m_service->disconnect(this);
    }
    m_service = service;
    if (m_service)
    {
        connect(m_service, &IDeviceDataService::statsUpdated,
                this, &DashboardController::onStatsUpdated);
        m_service->start();
    }
}

void DashboardController::onStatsUpdated(const DeviceStats &stats)
{
    m_currentStats = stats;

    // Update History
    m_hrHistory.append(stats.heartRate);
    if (m_hrHistory.size() > MAX_HISTORY)
        m_hrHistory.removeFirst();

    m_o2History.append(stats.oxygenLevel);
    if (m_o2History.size() > MAX_HISTORY)
        m_o2History.removeFirst();

    emit statsChanged();
    emit historyChanged();
}
