/**
 * @file MockTelemetryServer.cpp
 * @brief Mock implementation of ITelemetryServer for testing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockTelemetryServer.h"
#include <QMutexLocker>
#include <QDateTime>

namespace zmon {

MockTelemetryServer::MockTelemetryServer(QObject* parent)
    : ITelemetryServer(parent)
    , m_connected(false)
    , m_shouldSucceed(true)
    , m_telemetrySendCount(0)
    , m_sensorDataSendCount(0)
{
}

MockTelemetryServer::~MockTelemetryServer() = default;

void MockTelemetryServer::setServerUrl(const QString& url)
{
    m_serverUrl = url;
}

QString MockTelemetryServer::getServerUrl() const
{
    return m_serverUrl;
}

void MockTelemetryServer::setSslConfiguration(const QSslConfiguration& config)
{
    m_sslConfig = config;
}

QSslConfiguration MockTelemetryServer::getSslConfiguration() const
{
    return m_sslConfig;
}

bool MockTelemetryServer::validateCertificates()
{
    return true;  // Mock: always valid
}

bool MockTelemetryServer::connect()
{
    m_connected = true;
    m_lastError.clear();
    emit connectionStatusChanged(true);
    return true;
}

void MockTelemetryServer::disconnect()
{
    m_connected = false;
    emit connectionStatusChanged(false);
}

bool MockTelemetryServer::isConnected() const
{
    return m_connected;
}

void MockTelemetryServer::sendTelemetryAsync(
    const TelemetryData& data,
    std::function<void(const ServerResponse&)> callback)
{
    // Record the data
    {
        QMutexLocker locker(&m_dataMutex);
        m_sentTelemetry.append(data);
        m_telemetrySendCount++;
    }

    // Simulate async response
    ServerResponse response;
    response.serverTimestamp = QDateTime::currentDateTime();

    if (m_shouldSucceed) {
        response.success = true;
        response.statusCode = 200;
        response.message = "OK";
        response.processedIds = {1, 2, 3};  // Mock processed IDs
        m_lastError.clear();
        emit telemetrySent(data, response);
    } else {
        response.success = false;
        response.statusCode = 500;
        response.message = "Internal Server Error";
        m_lastError = response.message;
        emit telemetrySendFailed(data, response.message);
    }

    if (callback) {
        callback(response);
    }
}

void MockTelemetryServer::sendSensorDataAsync(
    const SensorData& data,
    std::function<void(const ServerResponse&)> callback)
{
    // Record the data
    {
        QMutexLocker locker(&m_dataMutex);
        m_sentSensorData.append(data);
        m_sensorDataSendCount++;
    }

    // Simulate async response
    ServerResponse response;
    response.serverTimestamp = QDateTime::currentDateTime();

    if (m_shouldSucceed) {
        response.success = true;
        response.statusCode = 200;
        response.message = "OK";
        m_lastError.clear();
    } else {
        response.success = false;
        response.statusCode = 500;
        response.message = "Internal Server Error";
        m_lastError = response.message;
    }

    if (callback) {
        callback(response);
    }
}

ServerResponse MockTelemetryServer::sendTelemetry(const TelemetryData& data)
{
    // Record the data
    {
        QMutexLocker locker(&m_dataMutex);
        m_sentTelemetry.append(data);
        m_telemetrySendCount++;
    }

    ServerResponse response;
    response.serverTimestamp = QDateTime::currentDateTime();

    if (m_shouldSucceed) {
        response.success = true;
        response.statusCode = 200;
        response.message = "OK";
        response.processedIds = {1, 2, 3};  // Mock processed IDs
        m_lastError.clear();
        emit telemetrySent(data, response);
    } else {
        response.success = false;
        response.statusCode = 500;
        response.message = "Internal Server Error";
        m_lastError = response.message;
        emit telemetrySendFailed(data, response.message);
    }

    return response;
}

bool MockTelemetryServer::isServerAvailable() const
{
    return m_connected;
}

QString MockTelemetryServer::getLastError() const
{
    return m_lastError;
}

void MockTelemetryServer::setShouldSucceed(bool shouldSucceed)
{
    m_shouldSucceed = shouldSucceed;
}

QList<TelemetryData> MockTelemetryServer::getSentTelemetry() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_sentTelemetry;
}

QList<SensorData> MockTelemetryServer::getSentSensorData() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_sentSensorData;
}

void MockTelemetryServer::clearRecordedData()
{
    QMutexLocker locker(&m_dataMutex);
    m_sentTelemetry.clear();
    m_sentSensorData.clear();
    m_telemetrySendCount = 0;
    m_sensorDataSendCount = 0;
}

int MockTelemetryServer::getTelemetrySendCount() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_telemetrySendCount;
}

int MockTelemetryServer::getSensorDataSendCount() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_sensorDataSendCount;
}

} // namespace zmon
