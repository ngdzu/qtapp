/**
 * @file ITelemetryServer.h
 * @brief Interface for sending telemetry data to central monitoring server.
 *
 * This file defines the ITelemetryServer interface which provides a standardized
 * way to send telemetry data and sensor data to a central monitoring server.
 * Supports different implementations: production server (network-based), mock server
 * (testing/development), and local file-based server (offline testing).
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QByteArray>
#include <QSslConfiguration>
#include <functional>

namespace zmon {
/**
 * @struct TelemetryData
 * @brief Telemetry data structure for transmission.
 */
struct TelemetryData {
    QString deviceId;              ///< Device identifier
    QString deviceLabel;           ///< Static device identifier/asset tag (e.g., "ICU-MON-04")
    QString patientMrn;            ///< Medical Record Number - REQUIRED for patient data association
    QString patientName;           ///< Patient name (optional, for server-side validation)
    QString bedLocation;           ///< Current bed/room location (from Patient object)
    QDateTime timestamp;           ///< Timestamp
    QList<QVariant> vitals;        ///< List of vital signs (VitalRecord objects)
    QList<QVariant> alarms;        ///< List of alarms (AlarmSnapshot objects)
    QList<QVariant> infusionEvents; ///< List of infusion events
    QList<QVariant> predictiveScores; ///< List of predictive scores
    QString signature;             ///< Digital signature for data integrity
    QString nonce;                 ///< Nonce for replay attack prevention
};

/**
 * @struct SensorData
 * @brief Sensor data structure for transmission.
 */
struct SensorData {
    QString deviceId;              ///< Device identifier
    QDateTime timestamp;           ///< Timestamp
    QByteArray waveformData;      ///< Waveform data (ECG, pleth, etc.)
    QString sensorType;            ///< Sensor type
    double sampleRate;             ///< Sample rate (Hz)
};

/**
 * @struct ServerResponse
 * @brief Server response structure.
 */
struct ServerResponse {
    bool success;                  ///< true if operation succeeded
    int statusCode;                ///< HTTP status code
    QString message;               ///< Response message
    QList<int> processedIds;       ///< IDs of records successfully processed
    QDateTime serverTimestamp;     ///< Server timestamp
};

/**
 * @class ITelemetryServer
 * @brief Interface for sending telemetry data to central monitoring server.
 *
 * Provides a standardized interface for server communication, allowing for
 * different implementations (production, mock, file-based) without changing
 * application code.
 *
 * @note Thread-safe: Can be called from any thread.
 * @note Network operations should run on a dedicated worker thread.
 *
 * @ingroup Infrastructure
 */
class ITelemetryServer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ITelemetryServer() = default;

    /**
     * @brief Configure server endpoint URL.
     *
     * @param url Server URL (e.g., "https://monitoring.hospital.com:8443")
     */
    virtual void setServerUrl(const QString& url) = 0;

    /**
     * @brief Get current server URL.
     *
     * @return Server URL string
     */
    virtual QString getServerUrl() const = 0;

    /**
     * @brief Set SSL configuration for mTLS.
     *
     * @param config SSL configuration (client certificates, CA certificates, etc.)
     */
    virtual void setSslConfiguration(const QSslConfiguration& config) = 0;

    /**
     * @brief Get current SSL configuration.
     *
     * @return SSL configuration
     */
    virtual QSslConfiguration getSslConfiguration() const = 0;

    /**
     * @brief Validate certificates.
     *
     * Validates client certificate, checks expiration, and verifies CRL.
     *
     * @return true if certificates are valid, false otherwise
     */
    virtual bool validateCertificates() = 0;

    /**
     * @brief Connect to server.
     *
     * Establishes connection to the server.
     *
     * @return true if connection succeeded, false otherwise
     */
    virtual bool connect() = 0;

    /**
     * @brief Disconnect from server.
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if connected to server.
     *
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Send telemetry data asynchronously (preferred).
     *
     * Sends telemetry data to the server without blocking. The callback
     * will be invoked when the operation completes.
     *
     * @param data Telemetry data to send
     * @param callback Callback function invoked with response (may be nullptr to use signals)
     */
    virtual void sendTelemetryAsync(
        const TelemetryData& data,
        std::function<void(const ServerResponse&)> callback = nullptr) = 0;

    /**
     * @brief Send sensor data asynchronously.
     *
     * Sends sensor/waveform data to the server without blocking.
     *
     * @param data Sensor data to send
     * @param callback Callback function invoked with response (may be nullptr to use signals)
     */
    virtual void sendSensorDataAsync(
        const SensorData& data,
        std::function<void(const ServerResponse&)> callback = nullptr) = 0;

    /**
     * @brief Send telemetry data synchronously (blocking).
     *
     * Sends telemetry data to the server and blocks until response is received.
     * Use with caution as this will block the calling thread.
     *
     * @param data Telemetry data to send
     * @return Server response
     */
    virtual ServerResponse sendTelemetry(const TelemetryData& data) = 0;

    /**
     * @brief Check if server is available.
     *
     * @return true if server is available, false otherwise
     */
    virtual bool isServerAvailable() const = 0;

    /**
     * @brief Get last error message.
     *
     * Returns the error message from the most recent failed operation.
     *
     * @return Error message string, or empty if no error
     */
    virtual QString getLastError() const = 0;

signals:
    /**
     * @brief Emitted when telemetry is sent successfully.
     *
     * @param data Telemetry data that was sent
     * @param response Server response
     */
    void telemetrySent(const TelemetryData& data, const ServerResponse& response);

    /**
     * @brief Emitted when telemetry send fails.
     *
     * @param data Telemetry data that failed to send
     * @param error Error message
     */
    void telemetrySendFailed(const TelemetryData& data, const QString& error);

    /**
     * @brief Emitted when connection status changes.
     *
     * @param connected true if connected, false if disconnected
     */
    void connectionStatusChanged(bool connected);
};

} // namespace zmon
} // namespace zmon