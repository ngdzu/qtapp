/**
 * @file SystemController.h
 * @brief QML controller for system status and device monitoring UI.
 *
 * Provides real-time system metrics including battery level, CPU temperature, memory usage,
 * and network latency. Updates are performed asynchronously on the UI thread with a 5-second
 * update interval. Platform-specific implementations handle Linux, macOS, and Windows differences.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <cstdint>

namespace zmon
{
    /**
     * @class SystemController
     * @brief QML controller for system status and device monitoring.
     *
     * Integrates with platform-specific APIs to monitor:
     * - **Battery Level**: Percentage from 0-100 (platform-dependent, -1 if unavailable)
     * - **CPU Temperature**: Degrees Celsius (platform-specific, 0.0 if unavailable)
     * - **Memory Usage**: Percentage from 0-100 (from /proc/meminfo on Linux)
     * - **Network Latency**: Milliseconds for round-trip to server (-1 if disconnected)
     * - **Connection State**: "connected" or "disconnected"
     * - **Firmware Version**: Device firmware version string
     *
     * Updates are performed every 5 seconds via QTimer. All monitoring is non-blocking
     * and runs on the UI thread. Platform-specific code is isolated to private methods.
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
        /**
         * @brief Constructs SystemController.
         * @param parent Optional parent QObject for memory management
         */
        explicit SystemController(QObject *parent = nullptr);

        /**
         * @brief Destructor. Stops monitoring timer.
         */
        ~SystemController() override;

        /**
         * @brief Returns current battery level percentage (0-100), or -1 if unavailable.
         * @return Battery level percentage or -1 for unknown
         */
        int batteryLevel() const { return m_batteryLevel; }

        /**
         * @brief Returns current CPU temperature in degrees Celsius, or 0.0 if unavailable.
         * @return Temperature in Celsius
         */
        double cpuTemperature() const { return m_cpuTemperature; }

        /**
         * @brief Returns current memory usage percentage (0-100).
         * @return Memory usage percentage
         */
        int memoryUsage() const { return m_memoryUsage; }

        /**
         * @brief Returns current network latency to server in milliseconds, or -1 if disconnected.
         * @return Latency in milliseconds or -1 for disconnected
         */
        int networkLatency() const { return m_networkLatency; }

        /**
         * @brief Returns connection state ("connected" or "disconnected").
         * @return Connection state string
         */
        QString connectionState() const { return m_connectionState; }

        /**
         * @brief Returns firmware version string.
         * @return Firmware version (e.g., "1.0.0")
         */
        QString firmwareVersion() const { return m_firmwareVersion; }

    signals:
        /**
         * @brief Emitted when battery level changes.
         */
        void batteryLevelChanged();

        /**
         * @brief Emitted when CPU temperature changes.
         */
        void cpuTemperatureChanged();

        /**
         * @brief Emitted when memory usage percentage changes.
         */
        void memoryUsageChanged();

        /**
         * @brief Emitted when network latency changes.
         */
        void networkLatencyChanged();

        /**
         * @brief Emitted when connection state changes.
         */
        void connectionStateChanged();

        /**
         * @brief Emitted when firmware version changes.
         */
        void firmwareVersionChanged();

    private slots:
        /**
         * @brief Performs periodic system monitoring update.
         * Called every 5 seconds by m_updateTimer.
         */
        void updateSystemStatus();

    private:
        /**
         * @brief Updates battery level from platform-specific sources.
         * Platform-specific implementation for Linux (/sys/class/power_supply/BAT0/capacity),
         * macOS (IOKit or sysctl), Windows (WMI or API calls).
         */
        void updateBatteryLevel();

        /**
         * @brief Updates CPU temperature from platform-specific sources.
         * Platform-specific implementation may require thermal zone access.
         */
        void updateCpuTemperature();

        /**
         * @brief Updates memory usage percentage from system info.
         * Uses /proc/meminfo on Linux, sysctl/mach APIs on macOS, Windows API on Windows.
         */
        void updateMemoryUsage();

        /**
         * @brief Updates network latency via ping to server.
         * Measures round-trip time to server. Sets to -1 if connection fails.
         */
        void updateNetworkLatency();

        /**
         * @brief Updates connection state based on network availability.
         * Emits connectionStateChanged() when state transitions between "connected"/"disconnected".
         */
        void updateConnectionState();

        // Member variables
        QTimer *m_updateTimer{nullptr};         ///< Timer for periodic updates (5 second interval)
        int m_batteryLevel{100};                ///< Battery percentage (0-100) or -1 if unknown
        double m_cpuTemperature{0.0};           ///< CPU temperature in Celsius
        int m_memoryUsage{0};                   ///< Memory usage percentage (0-100)
        int m_networkLatency{0};                ///< Network latency in milliseconds, -1 if disconnected
        QString m_connectionState{"connected"}; ///< Connection state ("connected" or "disconnected")
        QString m_firmwareVersion{"1.0.0"};     ///< Firmware version string
    };
} // namespace zmon
