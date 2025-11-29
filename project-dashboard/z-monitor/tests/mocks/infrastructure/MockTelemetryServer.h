/**
 * @file MockTelemetryServer.h
 * @brief Mock implementation of ITelemetryServer for testing.
 *
 * This mock implementation swallows all telemetry data without sending to a real server,
 * returns immediate success responses, and supports simulated failures for testing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "infrastructure/interfaces/ITelemetryServer.h"
#include <QMutex>
#include <QList>
#include <QString>

namespace zmon {

/**
 * @class MockTelemetryServer
 * @brief Mock implementation of ITelemetryServer for testing.
 *
 * This mock implementation:
 * - Swallows all data without sending to real server
 * - Returns immediate success responses
 * - Supports simulated failures for testing
 * - Tracks all sent telemetry data for verification
 *
 * @note Thread-safe: All methods are protected by mutex.
 */
class MockTelemetryServer : public ITelemetryServer {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     */
    explicit MockTelemetryServer(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MockTelemetryServer() override = default;

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

    // Test helper methods

    /**
     * @brief Get all telemetry data sent to this mock.
     *
     * @return List of all telemetry data sent
     */
    QList<TelemetryData> sentTelemetry() const;

    /**
     * @brief Get all sensor data sent to this mock.
     *
     * @return List of all sensor data sent
     */
    QList<SensorData> sentSensorData() const;

    /**
     * @brief Get the number of telemetry sends.
     *
     * @return Number of telemetry sends
     */
    int telemetrySendCount() const;

    /**
     * @brief Get the number of sensor data sends.
     *
     * @return Number of sensor data sends
     */
    int sensorDataSendCount() const;

    /**
     * @brief Clear all stored data.
     */
    void clear();

    /**
     * @brief Enable simulated failures.
     *
     * When enabled, all send operations will fail.
     *
     * @param enabled true to enable failures, false to disable
     */
    void setSimulateFailures(bool enabled);

    /**
     * @brief Check if failures are being simulated.
     *
     * @return true if failures are enabled, false otherwise
     */
    bool isSimulatingFailures() const;

    /**
     * @brief Set the error message for simulated failures.
     *
     * @param error Error message to return on failures
     */
    void setFailureError(const QString& error);

    /**
     * @brief Set connection state.
     *
     * @param connected true to simulate connected state, false for disconnected
     */
    void setConnected(bool connected);

    /**
     * @brief Set server availability.
     *
     * @param available true to simulate available server, false for unavailable
     */
    void setServerAvailable(bool available);

private:
    mutable QMutex m_mutex;
    QString m_serverUrl;
    QSslConfiguration m_sslConfig;
    bool m_connected{false};
    bool m_serverAvailable{true};
    bool m_simulateFailures{false};
    QString m_failureError{"Simulated failure"};
    QList<TelemetryData> m_sentTelemetry;
    QList<SensorData> m_sentSensorData;
    QString m_lastError;
};

} // namespace zmon

