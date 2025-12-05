/**
 * @file SystemController.cpp
 * @brief Implementation of SystemController with real system monitoring.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "SystemController.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include <QTimer>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#endif

namespace zmon
{
    SystemController::SystemController(QObject *parent) : QObject(parent)
    {
        // Create and configure timer for periodic updates (5 second interval)
        m_updateTimer = new QTimer(this);
        connect(m_updateTimer, &QTimer::timeout, this, &SystemController::updateSystemStatus);
        m_updateTimer->start(5000); // 5000 ms = 5 seconds

        // Perform initial update immediately
        updateSystemStatus();
    }

    SystemController::~SystemController()
    {
        if (m_updateTimer)
        {
            m_updateTimer->stop();
        }
    }

    void SystemController::updateSystemStatus()
    {
        updateBatteryLevel();
        updateCpuTemperature();
        updateMemoryUsage();
        updateNetworkLatency();
        updateConnectionState();
    }

    void SystemController::updateBatteryLevel()
    {
#ifdef Q_OS_LINUX
        // Platform-specific implementation for Linux
        QFile batteryFile("/sys/class/power_supply/BAT0/capacity");
        if (batteryFile.open(QIODevice::ReadOnly))
        {
            QString capacity = QString::fromUtf8(batteryFile.readAll()).trimmed();
            int level = capacity.toInt();
            if (m_batteryLevel != level && level >= 0 && level <= 100)
            {
                m_batteryLevel = level;
                emit batteryLevelChanged();
            }
            batteryFile.close();
        }
        else
        {
            // Fallback: battery info unavailable on this Linux system
            if (m_batteryLevel != -1)
            {
                m_batteryLevel = -1;
                emit batteryLevelChanged();
            }
        }
#else
        // macOS and other platforms: battery info unavailable (would require IOKit or sysctl)
        // For now, return -1 to indicate unknown
        if (m_batteryLevel != -1)
        {
            m_batteryLevel = -1;
            emit batteryLevelChanged();
        }
#endif
    }

    void SystemController::updateCpuTemperature()
    {
#ifdef Q_OS_LINUX
        // Platform-specific implementation for Linux (thermal zones)
        // Try common thermal zone paths
        for (int i = 0; i < 4; ++i)
        {
            QFile tempFile(QString("/sys/class/thermal/thermal_zone%1/temp").arg(i));
            if (tempFile.open(QIODevice::ReadOnly))
            {
                QString temp = QString::fromUtf8(tempFile.readAll()).trimmed();
                bool ok = false;
                int tempMilliC = temp.toInt(&ok);
                if (ok && tempMilliC > 0)
                {
                    double tempC = tempMilliC / 1000.0;
                    if (m_cpuTemperature != tempC)
                    {
                        m_cpuTemperature = tempC;
                        emit cpuTemperatureChanged();
                    }
                    tempFile.close();
                    return;
                }
                tempFile.close();
            }
        }
        // If no thermal zone found, return 0.0
        if (m_cpuTemperature != 0.0)
        {
            m_cpuTemperature = 0.0;
            emit cpuTemperatureChanged();
        }
#else
        // macOS and other platforms: CPU temperature unavailable without platform-specific APIs
        if (m_cpuTemperature != 0.0)
        {
            m_cpuTemperature = 0.0;
            emit cpuTemperatureChanged();
        }
#endif
    }

    void SystemController::updateMemoryUsage()
    {
#ifdef Q_OS_LINUX
        // Platform-specific implementation for Linux using /proc/meminfo
        QFile memInfo("/proc/meminfo");
        if (memInfo.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&memInfo);
            int64_t memTotal = 0;
            int64_t memAvailable = 0;

            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (line.startsWith("MemTotal:"))
                {
                    // Format: "MemTotal:        XXXXX kB"
                    QStringList parts = line.split(QRegularExpression("\\s+"));
                    if (parts.size() > 1)
                    {
                        memTotal = parts[1].toLongLong();
                    }
                }
                else if (line.startsWith("MemAvailable:"))
                {
                    QStringList parts = line.split(QRegularExpression("\\s+"));
                    if (parts.size() > 1)
                    {
                        memAvailable = parts[1].toLongLong();
                    }
                }
            }

            if (memTotal > 0)
            {
                int usage = static_cast<int>(100.0 * (memTotal - memAvailable) / memTotal);
                // Clamp to 0-100 range
                usage = qBound(0, usage, 100);
                if (m_memoryUsage != usage)
                {
                    m_memoryUsage = usage;
                    emit memoryUsageChanged();
                }
            }
            memInfo.close();
        }
#else
        // macOS and other platforms: memory usage unavailable without platform-specific APIs
        // Could use mach APIs on macOS or equivalent on other platforms
        // For now, default to 0
        if (m_memoryUsage != 0)
        {
            m_memoryUsage = 0;
            emit memoryUsageChanged();
        }
#endif
    }

    void SystemController::updateNetworkLatency()
    {
        // Network latency measurement via system ping (simplified implementation)
        // In a production system, this would integrate with actual network management
        // For now, we'll use a stub that assumes connected (0ms latency)
        // TODO: Integrate with actual TelemetryServer or NetworkManager ping method

        // Placeholder: Simulate network latency as 0 (connected)
        // Once integrated with real network layer, this will measure actual latency
        int latency = 0; // Connected, minimal latency
        if (m_networkLatency != latency)
        {
            m_networkLatency = latency;
            emit networkLatencyChanged();
        }
    }

    void SystemController::updateConnectionState()
    {
        // Connection state determination (simplified implementation)
        // In production, this would check actual server connectivity
        // For now, we'll consider "connected" if latency is >= 0
        // TODO: Integrate with actual network management layer

        QString newState = (m_networkLatency >= 0) ? "connected" : "disconnected";
        if (m_connectionState != newState)
        {
            m_connectionState = newState;
            emit connectionStateChanged();
        }
    }

} // namespace zmon
