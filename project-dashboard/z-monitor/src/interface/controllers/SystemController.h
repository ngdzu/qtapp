/**
 * @file SystemController.h
 * @brief QML controller for system status and device monitoring UI.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>

namespace zmon
{
    /**
     * @class SystemController
     * @brief QML controller for system status and device monitoring.
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class SystemController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
        Q_PROPERTY(double cpuTemperature READ cpuTemperature NOTIFY cpuTemperatureChanged)
        Q_PROPERTY(int memoryUsage READ memoryUsage NOTIFY memoryUsageChanged)
        Q_PROPERTY(int networkLatency READ networkLatency NOTIFY networkLatencyChanged)
        Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)
        Q_PROPERTY(QString firmwareVersion READ firmwareVersion NOTIFY firmwareVersionChanged)

    public:
        explicit SystemController(QObject *parent = nullptr);
        ~SystemController() override = default;

        int batteryLevel() const { return m_batteryLevel; }
        double cpuTemperature() const { return m_cpuTemperature; }
        int memoryUsage() const { return m_memoryUsage; }
        int networkLatency() const { return m_networkLatency; }
        QString connectionState() const { return m_connectionState; }
        QString firmwareVersion() const { return m_firmwareVersion; }

    signals:
        void batteryLevelChanged();
        void cpuTemperatureChanged();
        void memoryUsageChanged();
        void networkLatencyChanged();
        void connectionStateChanged();
        void firmwareVersionChanged();

    private:
        int m_batteryLevel{100};
        double m_cpuTemperature{0.0};
        int m_memoryUsage{0};
        int m_networkLatency{0};
        QString m_connectionState{"disconnected"};
        QString m_firmwareVersion{"1.0.0"};
    };
} // namespace zmon
