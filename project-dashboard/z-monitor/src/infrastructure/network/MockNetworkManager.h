/**
 * @file MockNetworkManager.h
 * @brief Mock NetworkManager implementation for telemetry server communication.
 *
 * This file contains the MockNetworkManager class which implements ITelemetryServer
 * interface for sending telemetry data to the central server. This is a mock
 * implementation that records requests and simulates server responses (200, 500, timeout)
 * without actual network communication. Used for testing and development before
 * adding mTLS plumbing.
 *
 * @note This is a mock implementation - no TLS/SSL initially
 * @note MockNetworkManager uses ITelemetryServer interface for server communication
 * @note Supports retry logic with exponential backoff
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "../interfaces/ITelemetryServer.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <memory>

namespace zmon {

/**
 * @struct RequestRecord
 * @brief Record of a telemetry request for testing/verification.
 */
struct RequestRecord {
    TelemetryData data;           ///< Telemetry data that was sent
    QDateTime timestamp;          ///< When request was made
    int attemptNumber;            ///< Retry attempt number (1 = first attempt)
};

/**
 * @class MockNetworkManager
 * @brief Mock NetworkManager implementation for telemetry server communication.
 *
 * Implements ITelemetryServer interface with mock behavior:
 * - Records all requests for testing/verification
 * - Simulates server responses (200, 500, timeout)
 * - Supports configurable response codes and delays
 * - Implements retry logic with exponential backoff
 *
 * @note Thread-safe: Can be called from any thread
 * @note No TLS/SSL in this mock implementation
 * @ingroup Infrastructure
 */
class MockNetworkManager : public ITelemetryServer {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit MockNetworkManager(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MockNetworkManager() override;

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

    // Mock-specific configuration methods
    /**
     * @brief Set simulated response code for next request.
     *
     * @param statusCode HTTP status code to simulate (200, 500, etc.)
     */
    void setSimulatedResponseCode(int statusCode);

    /**
     * @brief Set simulated network delay.
     *
     * @param delayMs Delay in milliseconds before responding
     */
    void setSimulatedDelay(int delayMs);

    /**
     * @brief Enable/disable timeout simulation.
     *
     * @param simulateTimeout true to simulate timeouts, false otherwise
     */
    void setSimulateTimeout(bool simulateTimeout);

    /**
     * @brief Set retry configuration.
     *
     * @param maxRetries Maximum number of retry attempts
     * @param initialBackoffMs Initial backoff delay in milliseconds
     * @param maxBackoffMs Maximum backoff delay in milliseconds
     */
    void setRetryConfig(int maxRetries, int initialBackoffMs = 1000, int maxBackoffMs = 30000);

    /**
     * @brief Get all recorded requests (for testing).
     *
     * @return List of request records
     */
    QList<RequestRecord> getRecordedRequests() const;

    /**
     * @brief Clear recorded requests.
     */
    void clearRecordedRequests();

    /**
     * @brief Get retry statistics.
     *
     * @return Map of attempt number to count of retries
     */
    QMap<int, int> getRetryStatistics() const;

private slots:
    /**
     * @brief Handle retry timer timeout.
     */
    void onRetryTimerTimeout();

private:
    /**
     * @brief Simulate sending telemetry data.
     *
     * @param data Telemetry data to send
     * @param attemptNumber Current retry attempt number
     * @return Server response
     */
    ServerResponse simulateSend(const TelemetryData& data, int attemptNumber = 1);

    /**
     * @brief Calculate exponential backoff delay.
     *
     * @param attemptNumber Retry attempt number (1 = first retry)
     * @return Delay in milliseconds
     */
    int calculateBackoffDelay(int attemptNumber) const;

    /**
     * @brief Check if error is retryable.
     *
     * @param statusCode HTTP status code
     * @return true if error is retryable, false otherwise
     */
    bool isRetryable(int statusCode) const;

    QString m_serverUrl;                          ///< Server URL
    QSslConfiguration m_sslConfig;               ///< SSL configuration (not used in mock)
    bool m_connected;                            ///< Connection status
    QString m_lastError;                         ///< Last error message

    // Mock configuration
    int m_simulatedResponseCode;                   ///< Simulated HTTP response code
    int m_simulatedDelayMs;                      ///< Simulated network delay
    bool m_simulateTimeout;                       ///< Whether to simulate timeouts

    // Retry configuration
    int m_maxRetries;                            ///< Maximum retry attempts
    int m_initialBackoffMs;                      ///< Initial backoff delay
    int m_maxBackoffMs;                          ///< Maximum backoff delay

    // Request recording
    mutable QMutex m_requestsMutex;               ///< Mutex for thread-safe access
    QList<RequestRecord> m_recordedRequests;     ///< Recorded requests

    // Retry queue
    struct RetryItem {
        TelemetryData data;
        int attemptNumber;
        std::function<void(const ServerResponse&)> callback;
    };
    QQueue<RetryItem> m_retryQueue;             ///< Queue of requests to retry
    QTimer* m_retryTimer;                       ///< Timer for retry scheduling
    QMutex m_retryMutex;                        ///< Mutex for retry queue

    // Statistics
    mutable QMutex m_statsMutex;                ///< Mutex for statistics
    QMap<int, int> m_retryStatistics;            ///< Retry attempt statistics
};

} // namespace zmon

