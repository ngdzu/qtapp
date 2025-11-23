#include "MockDeviceDataService.h"
#include <cmath>

MockDeviceDataService::MockDeviceDataService(QObject *parent)
    : IDeviceDataService(parent), m_timer(new QTimer(this)), m_time(0.0), m_battery(100)
{
    std::random_device rd;
    m_rng.seed(rd());

    connect(m_timer, &QTimer::timeout, this, &MockDeviceDataService::generateData);
}

void MockDeviceDataService::start()
{
    m_timer->start(1000); // Update every second
}

void MockDeviceDataService::stop()
{
    m_timer->stop();
}

void MockDeviceDataService::generateData()
{
    DeviceStats stats;

    // Simulate Heart Rate (Sine wave + noise)
    // Base 75, varies +/- 15, plus random noise
    double hrBase = 75 + 15 * std::sin(m_time * 0.5);
    std::uniform_int_distribution<int> noise(-5, 5);
    stats.heartRate = static_cast<int>(hrBase) + noise(m_rng);

    // Simulate SpO2 (Mostly stable 95-100)
    std::uniform_int_distribution<int> spo2Dist(95, 100);
    stats.oxygenLevel = spo2Dist(m_rng);

    // Simulate Battery (Slow decay)
    if (m_time > 0 && static_cast<int>(m_time) % 10 == 0)
    {
        m_battery = std::max(0, m_battery - 1);
    }
    stats.batteryLevel = m_battery;

    // Simulate Temperature (36-37 C)
    std::uniform_int_distribution<int> tempDist(36, 38);
    stats.temperature = tempDist(m_rng);

    stats.isConnected = true;

    emit statsUpdated(stats);

    m_time += 0.1;
}
