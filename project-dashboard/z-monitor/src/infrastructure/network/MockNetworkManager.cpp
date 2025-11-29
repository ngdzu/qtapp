/**
 * @file MockNetworkManager.cpp
 * @brief Mock NetworkManager implementation for telemetry server communication.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockNetworkManager.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QMutexLocker>
#include <cmath>

namespace zmon
{

    // Constants for retry and backoff
    namespace
    {
        constexpr int DEFAULT_MAX_RETRIES = 3;
        constexpr int DEFAULT_INITIAL_BACKOFF_MS = 1000;
        constexpr int DEFAULT_MAX_BACKOFF_MS = 30000;
        constexpr int DEFAULT_SIMULATED_DELAY_MS = 200;
        constexpr int DEFAULT_RESPONSE_CODE = 200;
        constexpr int TIMEOUT_DELAY_MS = 30000; // 30 seconds for timeout simulation
    }

    MockNetworkManager::MockNetworkManager(QObject *parent)
        : ITelemetryServer(parent), m_connected(false), m_simulatedResponseCode(DEFAULT_RESPONSE_CODE), m_simulatedDelayMs(DEFAULT_SIMULATED_DELAY_MS), m_simulateTimeout(false), m_maxRetries(DEFAULT_MAX_RETRIES), m_initialBackoffMs(DEFAULT_INITIAL_BACKOFF_MS), m_maxBackoffMs(DEFAULT_MAX_BACKOFF_MS), m_retryTimer(new QTimer(this))
    {
        m_retryTimer->setSingleShot(true);
        QObject::connect(m_retryTimer, &QTimer::timeout, this, &MockNetworkManager::onRetryTimerTimeout);
    }

    MockNetworkManager::~MockNetworkManager() = default;

    void MockNetworkManager::setServerUrl(const QString &url)
    {
        m_serverUrl = url;
    }

    QString MockNetworkManager::getServerUrl() const
    {
        return m_serverUrl;
    }

    void MockNetworkManager::setSslConfiguration(const QSslConfiguration &config)
    {
        m_sslConfig = config;
        // Note: SSL not used in mock implementation
    }

    QSslConfiguration MockNetworkManager::getSslConfiguration() const
    {
        return m_sslConfig;
    }

    bool MockNetworkManager::validateCertificates()
    {
        // Mock: always return true
        return true;
    }

    bool MockNetworkManager::connect()
    {
        m_connected = true;
        m_lastError.clear();
        emit connectionStatusChanged(true);
        return true;
    }

    void MockNetworkManager::disconnect()
    {
        m_connected = false;
        emit connectionStatusChanged(false);
    }

    bool MockNetworkManager::isConnected() const
    {
        return m_connected;
    }

    void MockNetworkManager::sendTelemetryAsync(
        const TelemetryData &data,
        std::function<void(const ServerResponse &)> callback)
    {
        if (!m_connected)
        {
            ServerResponse errorResponse;
            errorResponse.success = false;
            errorResponse.statusCode = 0;
            errorResponse.message = "Not connected to server";
            m_lastError = errorResponse.message;

            if (callback)
            {
                callback(errorResponse);
            }
            emit telemetrySendFailed(data, errorResponse.message);
            return;
        }

        // Record request
        {
            QMutexLocker locker(&m_requestsMutex);
            RequestRecord record;
            record.data = data;
            record.timestamp = QDateTime::currentDateTime();
            record.attemptNumber = 1;
            m_recordedRequests.append(record);
        }

        // Simulate async operation with delay
        QTimer::singleShot(m_simulatedDelayMs, this, [this, data, callback]()
                           {
        ServerResponse response = simulateSend(data, 1);

        if (callback) {
            callback(response);
        }

        if (response.success) {
            emit telemetrySent(data, response);
        } else {
            emit telemetrySendFailed(data, response.message);

            // Check if retryable and should retry
            if (isRetryable(response.statusCode) && response.statusCode != 0) {
                // Schedule retry
                QMutexLocker locker(&m_retryMutex);
                RetryItem item;
                item.data = data;
                item.attemptNumber = 2;  // Next attempt
                item.callback = callback;
                m_retryQueue.enqueue(item);

                // Update statistics
                {
                    QMutexLocker statsLocker(&m_statsMutex);
                    m_retryStatistics[1]++;
                }

                // Schedule retry with backoff
                int backoffDelay = calculateBackoffDelay(1);
                m_retryTimer->start(backoffDelay);
            }
        } });
    }

    void MockNetworkManager::sendSensorDataAsync(
        const SensorData &data,
        std::function<void(const ServerResponse &)> callback)
    {
        // Convert SensorData to TelemetryData for unified handling
        // In a real implementation, this would be handled differently
        Q_UNUSED(data);
        Q_UNUSED(callback);
        // Mock: not implemented for sensor data yet
    }

    ServerResponse MockNetworkManager::sendTelemetry(const TelemetryData &data)
    {
        if (!m_connected)
        {
            ServerResponse errorResponse;
            errorResponse.success = false;
            errorResponse.statusCode = 0;
            errorResponse.message = "Not connected to server";
            m_lastError = errorResponse.message;
            return errorResponse;
        }

        // Record request
        {
            QMutexLocker locker(&m_requestsMutex);
            RequestRecord record;
            record.data = data;
            record.timestamp = QDateTime::currentDateTime();
            record.attemptNumber = 1;
            m_recordedRequests.append(record);
        }

        // Simulate synchronous operation
        if (m_simulateTimeout)
        {
            // Simulate timeout by waiting
            QThread::msleep(TIMEOUT_DELAY_MS);
            ServerResponse timeoutResponse;
            timeoutResponse.success = false;
            timeoutResponse.statusCode = 408; // Request Timeout
            timeoutResponse.message = "Request timeout";
            m_lastError = timeoutResponse.message;
            return timeoutResponse;
        }

        // Simulate delay
        QThread::msleep(m_simulatedDelayMs);

        return simulateSend(data, 1);
    }

    bool MockNetworkManager::isServerAvailable() const
    {
        return m_connected;
    }

    QString MockNetworkManager::getLastError() const
    {
        return m_lastError;
    }

    void MockNetworkManager::setSimulatedResponseCode(int statusCode)
    {
        m_simulatedResponseCode = statusCode;
    }

    void MockNetworkManager::setSimulatedDelay(int delayMs)
    {
        m_simulatedDelayMs = delayMs;
    }

    void MockNetworkManager::setSimulateTimeout(bool simulateTimeout)
    {
        m_simulateTimeout = simulateTimeout;
    }

    void MockNetworkManager::setRetryConfig(int maxRetries, int initialBackoffMs, int maxBackoffMs)
    {
        m_maxRetries = maxRetries;
        m_initialBackoffMs = initialBackoffMs;
        m_maxBackoffMs = maxBackoffMs;
    }

    QList<RequestRecord> MockNetworkManager::getRecordedRequests() const
    {
        QMutexLocker locker(&m_requestsMutex);
        return m_recordedRequests;
    }

    void MockNetworkManager::clearRecordedRequests()
    {
        QMutexLocker locker(&m_requestsMutex);
        m_recordedRequests.clear();
    }

    QMap<int, int> MockNetworkManager::getRetryStatistics() const
    {
        QMutexLocker locker(&m_statsMutex);
        return m_retryStatistics;
    }

    void MockNetworkManager::onRetryTimerTimeout()
    {
        QMutexLocker locker(&m_retryMutex);
        if (m_retryQueue.isEmpty())
        {
            return;
        }

        RetryItem item = m_retryQueue.dequeue();
        locker.unlock();

        // Retry the request
        ServerResponse response = simulateSend(item.data, item.attemptNumber);

        if (item.callback)
        {
            item.callback(response);
        }

        if (response.success)
        {
            emit telemetrySent(item.data, response);
        }
        else
        {
            emit telemetrySendFailed(item.data, response.message);

            // Check if should retry again
            if (isRetryable(response.statusCode) && item.attemptNumber < m_maxRetries)
            {
                // Schedule another retry
                QMutexLocker retryLocker(&m_retryMutex);
                RetryItem nextItem;
                nextItem.data = item.data;
                nextItem.attemptNumber = item.attemptNumber + 1;
                nextItem.callback = item.callback;
                m_retryQueue.enqueue(nextItem);

                // Update statistics
                {
                    QMutexLocker statsLocker(&m_statsMutex);
                    m_retryStatistics[item.attemptNumber]++;
                }

                // Schedule retry with backoff
                int backoffDelay = calculateBackoffDelay(item.attemptNumber);
                m_retryTimer->start(backoffDelay);
            }
        }
    }

    ServerResponse MockNetworkManager::simulateSend(const TelemetryData &data, int attemptNumber)
    {
        Q_UNUSED(data);
        Q_UNUSED(attemptNumber);

        ServerResponse response;
        response.serverTimestamp = QDateTime::currentDateTime();

        if (m_simulateTimeout)
        {
            response.success = false;
            response.statusCode = 408; // Request Timeout
            response.message = "Request timeout";
            m_lastError = response.message;
            return response;
        }

        // Simulate delay
        QThread::msleep(m_simulatedDelayMs);

        // Simulate response based on configured code
        if (m_simulatedResponseCode == 200)
        {
            response.success = true;
            response.statusCode = 200;
            response.message = "OK";
            response.processedIds = {1, 2, 3}; // Mock processed IDs
            m_lastError.clear();
        }
        else if (m_simulatedResponseCode == 500)
        {
            response.success = false;
            response.statusCode = 500;
            response.message = "Internal Server Error";
            m_lastError = response.message;
        }
        else
        {
            response.success = false;
            response.statusCode = m_simulatedResponseCode;
            response.message = QString("HTTP %1").arg(m_simulatedResponseCode);
            m_lastError = response.message;
        }

        return response;
    }

    int MockNetworkManager::calculateBackoffDelay(int attemptNumber) const
    {
        // Exponential backoff: initialBackoff * 2^(attemptNumber - 1)
        // Capped at maxBackoffMs
        int delay = m_initialBackoffMs * static_cast<int>(std::pow(2, attemptNumber - 1));
        return std::min(delay, m_maxBackoffMs);
    }

    bool MockNetworkManager::isRetryable(int statusCode) const
    {
        // Retryable: 5xx server errors, 408 timeout, network errors (0)
        // Not retryable: 4xx client errors (except 408), 2xx success
        if (statusCode == 0)
        {
            return true; // Network error
        }
        if (statusCode == 408)
        {
            return true; // Timeout
        }
        if (statusCode >= 500 && statusCode < 600)
        {
            return true; // Server errors
        }
        return false; // Client errors (4xx except 408) and success (2xx)
    }

} // namespace zmon
