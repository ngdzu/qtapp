/**
 * @file MockTelemetryServer.cpp
 * @brief Implementation of MockTelemetryServer.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockTelemetryServer.h"
#include <QMutexLocker>
#include <QTimer>

namespace zmon {

MockTelemetryServer::MockTelemetryServer(QObject* parent)
    : ITelemetryServer(parent)
{
}

void MockTelemetryServer::setServerUrl(const QString& url)
{
    QMutexLocker locker(&m_mutex);
    m_serverUrl = url;
}

QString MockTelemetryServer::getServerUrl() const
{
    QMutexLocker locker(&m_mutex);
    return m_serverUrl;
}

void MockTelemetryServer::setSslConfiguration(const QSslConfiguration& config)
{
    QMutexLocker locker(&m_mutex);
    m_sslConfig = config;
}

QSslConfiguration MockTelemetryServer::getSslConfiguration() const
{
    QMutexLocker locker(&m_mutex);
    return m_sslConfig;
}

bool MockTelemetryServer::validateCertificates()
{
    QMutexLocker locker(&m_mutex);
    return !m_simulateFailures;
}

bool MockTelemetryServer::connect()
{
    QMutexLocker locker(&m_mutex);
    if (m_simulateFailures) {
        m_lastError = m_failureError;
        return false;
    }
    m_connected = true;
    emit connectionStatusChanged(true);
    return true;
}

void MockTelemetryServer::disconnect()
{
    QMutexLocker locker(&m_mutex);
    m_connected = false;
    emit connectionStatusChanged(false);
}

bool MockTelemetryServer::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_connected;
}

void MockTelemetryServer::sendTelemetryAsync(
    const TelemetryData& data,
    std::function<void(const ServerResponse&)> callback)
{
    ServerResponse response;
    
    {
        QMutexLocker locker(&m_mutex);
        m_sentTelemetry.append(data);
        
        if (m_simulateFailures) {
            m_lastError = m_failureError;
            response.success = false;
            response.statusCode = 500;
            response.message = m_failureError;
        } else {
            response.success = true;
            response.statusCode = 200;
            response.message = "OK";
            response.serverTimestamp = QDateTime::currentDateTime();
        }
    }
    
    // Emit signal
    if (response.success) {
        emit telemetrySent(data, response);
    } else {
        emit telemetrySendFailed(data, response.message);
    }
    
    // Call callback if provided
    if (callback) {
        callback(response);
    }
}

void MockTelemetryServer::sendSensorDataAsync(
    const SensorData& data,
    std::function<void(const ServerResponse&)> callback)
{
    ServerResponse response;
    
    {
        QMutexLocker locker(&m_mutex);
        m_sentSensorData.append(data);
        
        if (m_simulateFailures) {
            m_lastError = m_failureError;
            response.success = false;
            response.statusCode = 500;
            response.message = m_failureError;
        } else {
            response.success = true;
            response.statusCode = 200;
            response.message = "OK";
            response.serverTimestamp = QDateTime::currentDateTime();
        }
    }
    
    // Call callback if provided
    if (callback) {
        callback(response);
    }
}

ServerResponse MockTelemetryServer::sendTelemetry(const TelemetryData& data)
{
    ServerResponse response;
    
    {
        QMutexLocker locker(&m_mutex);
        m_sentTelemetry.append(data);
        
        if (m_simulateFailures) {
            m_lastError = m_failureError;
            response.success = false;
            response.statusCode = 500;
            response.message = m_failureError;
        } else {
            response.success = true;
            response.statusCode = 200;
            response.message = "OK";
            response.serverTimestamp = QDateTime::currentDateTime();
        }
    }
    
    // Emit signal
    if (response.success) {
        emit telemetrySent(data, response);
    } else {
        emit telemetrySendFailed(data, response.message);
    }
    
    return response;
}

bool MockTelemetryServer::isServerAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_serverAvailable;
}

QString MockTelemetryServer::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

QList<TelemetryData> MockTelemetryServer::sentTelemetry() const
{
    QMutexLocker locker(&m_mutex);
    return m_sentTelemetry;
}

QList<SensorData> MockTelemetryServer::sentSensorData() const
{
    QMutexLocker locker(&m_mutex);
    return m_sentSensorData;
}

int MockTelemetryServer::telemetrySendCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_sentTelemetry.size();
}

int MockTelemetryServer::sensorDataSendCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_sentSensorData.size();
}

void MockTelemetryServer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_sentTelemetry.clear();
    m_sentSensorData.clear();
    m_lastError.clear();
}

void MockTelemetryServer::setSimulateFailures(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_simulateFailures = enabled;
}

bool MockTelemetryServer::isSimulatingFailures() const
{
    QMutexLocker locker(&m_mutex);
    return m_simulateFailures;
}

void MockTelemetryServer::setFailureError(const QString& error)
{
    QMutexLocker locker(&m_mutex);
    m_failureError = error;
}

void MockTelemetryServer::setConnected(bool connected)
{
    QMutexLocker locker(&m_mutex);
    if (m_connected != connected) {
        m_connected = connected;
        emit connectionStatusChanged(connected);
    }
}

void MockTelemetryServer::setServerAvailable(bool available)
{
    QMutexLocker locker(&m_mutex);
    m_serverAvailable = available;
}

} // namespace zmon

