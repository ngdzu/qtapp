/**
 * @file MockTelemetryServer.h
 * @brief Mock implementation of ITelemetryServer for testing.
 *
 * This file contains a simple mock implementation of ITelemetryServer that
 * swallows all data for testing purposes. Used in unit tests to verify
 * SystemController and NotificationController behavior without actual
 * network communication.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "infrastructure/interfaces/ITelemetryServer.h"
#include <QObject>
#include <QList>
#include <QMutex>

namespace zmon {

/**
 * @class MockTelemetryServer
 * @brief Simple mock implementation of ITelemetryServer for testing.
 *
 * This mock implementation:
 * - Swallows all telemetry data (no actual transmission)
 * - Records all requests for verification
 * - Can be configured to simulate success/failure responses
 * - Thread-safe for use in multi-threaded tests
 *
 * @note Used for unit testing SystemController and NotificationController
 * @ingroup Testing
 */
class MockTelemetryServer : public ITelemetryServer {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit MockTelemetryServer(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MockTelemetryServer() override;

    // ITelemetryServer interface implementation
    void setServerUrl(const QString& url) override;
    QString getServerUrl() const override;
    void setSslConfiguration(const QSslConfiguration& config) override;
    QSslConfiguration getSslConfiguration() const override;
    bool validateCertificates() override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    void sendTelemetryAsync(
        const TelemetryData& data,
        std::function<void(const ServerResponse&)> callback = nullptr) override;
    void sendSensorDataAsync(
        const SensorData& data,
        std::function<void(const ServerResponse&)> callback = nullptr) override;
    ServerResponse sendTelemetry(const TelemetryData& data) override;
    bool isServerAvailable() const override;
    QString getLastError() const override;

    // Mock-specific methods
    /**
     * @brief Set whether to simulate success or failure.
     *
     * @param shouldSucceed true to simulate success, false to simulate failure
     */
    void setShouldSucceed(bool shouldSucceed);

    /**
     * @brief Get all telemetry data that was sent (for verification).
     *
     * @return List of telemetry data records
     */
    QList<TelemetryData> getSentTelemetry() const;

    /**
     * @brief Get all sensor data that was sent (for verification).
     *
     * @return List of sensor data records
     */
    QList<SensorData> getSentSensorData() const;

    /**
     * @brief Clear all recorded data.
     */
    void clearRecordedData();

    /**
     * @brief Get count of telemetry sends.
     *
     * @return Number of telemetry sends
     */
    int getTelemetrySendCount() const;

    /**
     * @brief Get count of sensor data sends.
     *
     * @return Number of sensor data sends
     */
    int getSensorDataSendCount() const;

private:
    QString m_serverUrl;                          ///< Server URL
    QSslConfiguration m_sslConfig;               ///< SSL configuration
    bool m_connected;                            ///< Connection status
    bool m_shouldSucceed;                        ///< Whether to simulate success
    QString m_lastError;                         ///< Last error message

    mutable QMutex m_dataMutex;                  ///< Mutex for thread-safe access
    QList<TelemetryData> m_sentTelemetry;       ///< Recorded telemetry data
    QList<SensorData> m_sentSensorData;         ///< Recorded sensor data
    int m_telemetrySendCount;                    ///< Count of telemetry sends
    int m_sensorDataSendCount;                   ///< Count of sensor data sends
};

} // namespace zmon
