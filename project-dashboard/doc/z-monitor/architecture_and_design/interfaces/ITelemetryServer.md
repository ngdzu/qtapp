# ITelemetryServer Interface

**Interface ID:** IFACE-002  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## 1. Overview

The `ITelemetryServer` interface defines the contract for transmitting vital signs, alarms, and telemetry data to the central server using secure mTLS connections.

**Purpose:**
- Abstract central server communication
- Enable testing with mock implementations
- Support batch transmission for efficiency
- Track transmission metrics and latency

**Related Documents:**
- **Requirements:** [REQ-FUN-DATA-001](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md), [REQ-INT-SRV-001](../../requirements/06_INTERFACE_REQUIREMENTS.md)
- **Use Cases:** [UC-DS-001](../../requirements/02_USE_CASES.md), [UC-DS-002](../../requirements/02_USE_CASES.md)
- **Security:** [06_SECURITY.md](../06_SECURITY.md)

---

## 2. Interface Definition

### 2.1 C++ Header

```cpp
/**
 * @interface ITelemetryServer
 * @brief Interface for transmitting telemetry data to central server.
 * 
 * This interface abstracts server communication, allowing multiple
 * implementations (production HTTPS/mTLS, mock, offline queue).
 * 
 * @note All transmissions use mTLS for authentication and encryption
 * @see TelemetryBatch, NetworkManager
 * @ingroup Infrastructure
 */
class ITelemetryServer : public QObject {
    Q_OBJECT

public:
    virtual ~ITelemetryServer() = default;

    /**
     * @brief Send batch of vital signs to central server.
     * 
     * Transmits vitals data in batch for efficiency. Uses mTLS
     * for secure authenticated connection.
     * 
     * @param batch Telemetry batch containing vitals and metadata
     * @return QFuture<TransmissionResult> Future resolving to transmission status
     * 
     * @note Asynchronous operation (10-second timeout)
     * @note Records timing metrics (transmitted_at, server_received_at, etc.)
     * @see TelemetryBatch, TransmissionResult
     */
    virtual QFuture<TransmissionResult> sendVitalsBatch(
        const TelemetryBatch& batch
    ) = 0;

    /**
     * @brief Send alarm event to central server (high priority).
     * 
     * Sends single alarm immediately (not batched) due to urgency.
     * 
     * @param alarm Alarm event data
     * @return QFuture<TransmissionResult> Future resolving to transmission status
     * 
     * @note Asynchronous operation (5-second timeout for alarms)
     * @note Critical alarms have highest priority
     */
    virtual QFuture<TransmissionResult> sendAlarm(
        const AlarmEvent& alarm
    ) = 0;

    /**
     * @brief Register device with central server.
     * 
     * Registers device on startup, providing device metadata and
     * establishing session.
     * 
     * @param deviceInfo Device metadata (ID, label, version, capabilities)
     * @return QFuture<RegistrationResult> Future resolving to session info
     * 
     * @note Call once on startup
     * @see DeviceInfo, RegistrationResult
     */
    virtual QFuture<RegistrationResult> registerDevice(
        const DeviceInfo& deviceInfo
    ) = 0;

    /**
     * @brief Send heartbeat to central server.
     * 
     * Periodic heartbeat to maintain connection and report device status.
     * 
     * @param status Device status (online, patient admitted, etc.)
     * @return QFuture<bool> Future resolving to true if acknowledged
     * 
     * @note Call every 5 minutes
     */
    virtual QFuture<bool> sendHeartbeat(const DeviceStatus& status) = 0;

    /**
     * @brief Check if central server is reachable.
     * 
     * Quick connectivity check (no data transmission).
     * 
     * @return QFuture<bool> Future resolving to true if server reachable
     * 
     * @note Quick check (2-second timeout)
     */
    virtual QFuture<bool> isServerAvailable() = 0;

    /**
     * @brief Get server connection metadata.
     * 
     * Returns information about server connection (URL, status, last contact).
     * 
     * @return ServerMetadata Metadata about server connection
     */
    virtual ServerMetadata getServerInfo() const = 0;

signals:
    /**
     * @brief Emitted when transmission completes successfully.
     * 
     * @param batchId Telemetry batch ID
     * @param metrics Transmission metrics (latency, size, etc.)
     */
    void transmissionSucceeded(const QString& batchId, const TransmissionMetrics& metrics);

    /**
     * @brief Emitted when transmission fails.
     * 
     * @param batchId Telemetry batch ID
     * @param error Error details
     */
    void transmissionFailed(const QString& batchId, const TransmissionError& error);

    /**
     * @brief Emitted when server connection status changes.
     * 
     * @param connected true if server now reachable
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief Emitted when server configuration received.
     * 
     * Server may push configuration updates (sync interval, etc.).
     * 
     * @param config Updated configuration from server
     */
    void serverConfigurationReceived(const ServerConfiguration& config);
};
```

---

## 3. Data Structures

### 3.1 TelemetryBatch

```cpp
/**
 * @struct TelemetryBatch
 * @brief Batch of telemetry data for transmission.
 * 
 * Contains multiple vital sign records plus metadata for efficient transmission.
 */
struct TelemetryBatch {
    QString batchId;                    ///< UUID for this batch
    QString deviceId;                   ///< Device identifier
    QString patientMrn;                 ///< Patient MRN (or empty if no patient)
    QDateTime created_at;               ///< Batch creation time
    QList<VitalRecord> vitals;          ///< Vital signs records (10-100 typical)
    QList<AlarmEvent> alarms;           ///< Alarms in this batch (if any)
    QString signature;                  ///< Digital signature (HMAC-SHA256)
    
    /**
     * @brief Validate batch data.
     * @return true if batch is valid (has device ID, records, valid signature)
     */
    bool isValid() const {
        return !batchId.isEmpty() && !deviceId.isEmpty() && 
               !vitals.isEmpty() && !signature.isEmpty();
    }
    
    /**
     * @brief Calculate batch size in bytes (for metrics).
     * @return Estimated size in bytes
     */
    int estimatedSizeBytes() const {
        return vitals.size() * 150 + alarms.size() * 500;
    }
    
    /**
     * @brief Serialize batch to JSON.
     * @return JSON representation of batch
     */
    QByteArray toJson() const;
    
    /**
     * @brief Sign batch data with HMAC-SHA256.
     * @param secretKey Secret key for signing
     */
    void sign(const QByteArray& secretKey);
    
    /**
     * @brief Verify batch signature.
     * @param secretKey Secret key for verification
     * @return true if signature valid
     */
    bool verifySignature(const QByteArray& secretKey) const;
};
```

### 3.2 VitalRecord

```cpp
/**
 * @struct VitalRecord
 * @brief Single vital sign measurement.
 */
struct VitalRecord {
    QDateTime timestamp;        ///< Measurement time (Unix milliseconds)
    int heartRate;              ///< Heart rate (bpm)
    double spo2;                ///< SpO2 (%)
    int respirationRate;        ///< Respiration rate (rpm)
    QString signalQuality;      ///< GOOD, FAIR, POOR, DISCONNECTED
    
    /**
     * @brief Serialize to JSON object.
     */
    QJsonObject toJson() const {
        return {
            {"timestamp", timestamp.toMSecsSinceEpoch()},
            {"heartRate", heartRate},
            {"spo2", spo2},
            {"respirationRate", respirationRate},
            {"signalQuality", signalQuality}
        };
    }
};
```

### 3.3 AlarmEvent

```cpp
/**
 * @struct AlarmEvent
 * @brief Alarm event data.
 */
struct AlarmEvent {
    QString alarmId;            ///< UUID for this alarm
    QString deviceId;           ///< Device identifier
    QString patientMrn;         ///< Patient MRN
    QString patientName;        ///< Patient name (for display)
    QDateTime timestamp;        ///< Alarm trigger time
    QString priority;           ///< HIGH, MEDIUM, LOW
    QString alarmType;          ///< HR_HIGH, SPO2_LOW, etc.
    double value;               ///< Vital sign value that triggered alarm
    double threshold;           ///< Threshold that was exceeded
    QString status;             ///< ACTIVE, ACKNOWLEDGED, RESOLVED
    QDateTime acknowledgedAt;   ///< Time acknowledged (if applicable)
    QString acknowledgedBy;     ///< User who acknowledged
    
    /**
     * @brief Serialize to JSON object.
     */
    QJsonObject toJson() const;
};
```

### 3.4 TransmissionResult

```cpp
/**
 * @struct TransmissionResult
 * @brief Result of telemetry transmission.
 */
struct TransmissionResult {
    bool success;                       ///< true if transmission succeeded
    QString batchId;                    ///< Batch ID
    int recordsReceived;                ///< Number of records server received
    QDateTime serverTimestamp;          ///< Server timestamp (for clock sync)
    TransmissionMetrics metrics;        ///< Transmission timing metrics
    TransmissionError error;            ///< Error details (if failed)
    
    /**
     * @brief Check if transmission was successful.
     */
    bool isSuccess() const { return success; }
    
    /**
     * @brief Get end-to-end latency in milliseconds.
     */
    int endToEndLatencyMs() const {
        return metrics.endToEndLatencyMs;
    }
};
```

### 3.5 TransmissionMetrics

```cpp
/**
 * @struct TransmissionMetrics
 * @brief Detailed timing metrics for transmission.
 * 
 * Tracks timing at each stage for performance monitoring and diagnostics.
 */
struct TransmissionMetrics {
    QDateTime created_at;               ///< Batch creation time
    QDateTime saved_at;                 ///< Saved to database
    QDateTime transmitted_at;           ///< Transmission started
    QDateTime server_received_at;       ///< Server received request
    QDateTime server_ack_at;            ///< Server acknowledgment sent
    
    int db_latency_ms;                  ///< Database write latency
    int transmission_latency_ms;        ///< Network transmission latency
    int server_latency_ms;              ///< Server processing latency
    int endToEndLatencyMs;              ///< Total latency (created → ack)
    
    int batchSizeBytes;                 ///< Batch size in bytes
    int recordCount;                    ///< Number of records in batch
    
    /**
     * @brief Calculate all latencies from timestamps.
     */
    void calculateLatencies() {
        db_latency_ms = created_at.msecsTo(saved_at);
        transmission_latency_ms = transmitted_at.msecsTo(server_received_at);
        server_latency_ms = server_received_at.msecsTo(server_ack_at);
        endToEndLatencyMs = created_at.msecsTo(server_ack_at);
    }
    
    /**
     * @brief Classify latency quality.
     * @return "EXCELLENT", "GOOD", "ACCEPTABLE", "SLOW", "CRITICAL"
     */
    QString latencyQuality() const {
        if (endToEndLatencyMs < 100) return "EXCELLENT";
        if (endToEndLatencyMs < 500) return "GOOD";
        if (endToEndLatencyMs < 2000) return "ACCEPTABLE";
        if (endToEndLatencyMs < 5000) return "SLOW";
        return "CRITICAL";
    }
};
```

### 3.6 TransmissionError

```cpp
/**
 * @struct TransmissionError
 * @brief Error information for failed transmission.
 */
struct TransmissionError {
    ErrorCode code;                     ///< Error code
    QString message;                    ///< Human-readable error message
    int httpStatus;                     ///< HTTP status code (if applicable)
    bool retryable;                     ///< true if retry recommended
    QDateTime timestamp;                ///< When error occurred
    
    bool hasError() const { return code != ErrorCode::None; }
};

/**
 * @enum ErrorCode
 * @brief Error codes for transmission failures.
 */
enum class ErrorCode {
    None,                               ///< No error
    NetworkError,                       ///< Network connection failed
    Timeout,                            ///< Request timed out
    AuthenticationError,                ///< mTLS authentication failed
    ServerError,                        ///< Server error (500)
    InvalidData,                        ///< Invalid batch data
    SignatureError,                     ///< Signature verification failed
    UnknownError                        ///< Unexpected error
};
```

---

## 4. Implementations

### 4.1 Production Implementation: NetworkTelemetryServer

**Technology:** Qt Network (HTTPS/mTLS)

```cpp
/**
 * @class NetworkTelemetryServer
 * @brief Production implementation for central server communication.
 * 
 * Uses Qt Network with mTLS for secure authenticated connections.
 * 
 * @note Implements retry logic with exponential backoff
 * @note Records all transmission metrics to telemetry_metrics table
 */
class NetworkTelemetryServer : public ITelemetryServer {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param baseUrl Server base URL (e.g., "https://telemetry.hospital.com")
     * @param clientCert Client certificate for mTLS
     * @param clientKey Client private key for mTLS
     * @param caCert CA certificate for server verification
     */
    NetworkTelemetryServer(
        const QString& baseUrl,
        const QSslCertificate& clientCert,
        const QSslKey& clientKey,
        const QSslCertificate& caCert,
        QObject* parent = nullptr
    );

    // ITelemetryServer interface
    QFuture<TransmissionResult> sendVitalsBatch(const TelemetryBatch& batch) override;
    QFuture<TransmissionResult> sendAlarm(const AlarmEvent& alarm) override;
    QFuture<RegistrationResult> registerDevice(const DeviceInfo& deviceInfo) override;
    QFuture<bool> sendHeartbeat(const DeviceStatus& status) override;
    QFuture<bool> isServerAvailable() override;
    ServerMetadata getServerInfo() const override;

private slots:
    void onNetworkReplyFinished();
    void onSslErrors(const QList<QSslError>& errors);
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    void configureMtls(QNetworkRequest& request);
    TransmissionResult parseResponse(const QByteArray& json);
    void recordMetrics(const TransmissionMetrics& metrics);
    void retryWithBackoff(const TelemetryBatch& batch, int retryCount);

    QNetworkAccessManager* m_network;
    QString m_baseUrl;
    QSslCertificate m_clientCert;
    QSslKey m_clientKey;
    QSslCertificate m_caCert;
    ServerMetadata m_metadata;
    ITelemetryRepository* m_metricsRepo;
};
```

### 4.2 Mock Implementation: MockTelemetryServer

**Technology:** In-memory (no network)

```cpp
/**
 * @class MockTelemetryServer
 * @brief Mock implementation for testing and development.
 * 
 * Simulates server responses without network communication.
 * 
 * @note Useful for UI development, unit testing, offline demos
 */
class MockTelemetryServer : public ITelemetryServer {
    Q_OBJECT

public:
    MockTelemetryServer(QObject* parent = nullptr);

    /**
     * @brief Simulate network delay.
     * @param delayMs Delay in milliseconds (default: 200ms)
     */
    void setSimulatedDelay(int delayMs) { m_delayMs = delayMs; }

    /**
     * @brief Simulate network failure.
     * @param shouldFail true to simulate failures
     */
    void setSimulateFailure(bool shouldFail) { m_simulateFailure = shouldFail; }

    /**
     * @brief Get all batches sent (for testing verification).
     */
    QList<TelemetryBatch> getSentBatches() const { return m_sentBatches; }

    // ITelemetryServer interface
    QFuture<TransmissionResult> sendVitalsBatch(const TelemetryBatch& batch) override;
    QFuture<TransmissionResult> sendAlarm(const AlarmEvent& alarm) override;
    QFuture<RegistrationResult> registerDevice(const DeviceInfo& deviceInfo) override;
    QFuture<bool> sendHeartbeat(const DeviceStatus& status) override;
    QFuture<bool> isServerAvailable() override;
    ServerMetadata getServerInfo() const override;

private:
    QList<TelemetryBatch> m_sentBatches;
    QList<AlarmEvent> m_sentAlarms;
    int m_delayMs = 200;
    bool m_simulateFailure = false;
};
```

---

## 5. Usage Examples

### 5.1 Send Vitals Batch

```cpp
// Create telemetry batch
TelemetryBatch batch;
batch.batchId = QUuid::createUuid().toString();
batch.deviceId = "ZM-ICU-MON-04";
batch.patientMrn = "MRN-12345";
batch.created_at = QDateTime::currentDateTime();

// Add vital signs records
for (int i = 0; i < 10; i++) {
    VitalRecord vital;
    vital.timestamp = QDateTime::currentDateTime().addSecs(i);
    vital.heartRate = 75 + qrand() % 10;
    vital.spo2 = 98.0;
    vital.respirationRate = 16;
    vital.signalQuality = "GOOD";
    batch.vitals.append(vital);
}

// Sign batch
batch.sign(deviceSecret);

// Send to server
auto future = telemetryServer->sendVitalsBatch(batch);

// Handle result
future.then([](const TransmissionResult& result) {
    if (result.isSuccess()) {
        qDebug() << "Transmission succeeded";
        qDebug() << "End-to-end latency:" << result.endToEndLatencyMs() << "ms";
        qDebug() << "Quality:" << result.metrics.latencyQuality();
    } else {
        qDebug() << "Transmission failed:" << result.error.message;
        if (result.error.retryable) {
            qDebug() << "Will retry...";
        }
    }
});
```

### 5.2 Send Critical Alarm (Immediate)

```cpp
// Create alarm event
AlarmEvent alarm;
alarm.alarmId = QUuid::createUuid().toString();
alarm.deviceId = "ZM-ICU-MON-04";
alarm.patientMrn = "MRN-12345";
alarm.patientName = "John Doe";
alarm.timestamp = QDateTime::currentDateTime();
alarm.priority = "HIGH";
alarm.alarmType = "HR_HIGH";
alarm.value = 150.0;
alarm.threshold = 120.0;
alarm.status = "ACTIVE";

// Send immediately (not batched)
auto future = telemetryServer->sendAlarm(alarm);

future.then([](const TransmissionResult& result) {
    if (result.isSuccess()) {
        qDebug() << "Alarm sent to central server";
    } else {
        qCritical() << "Failed to send critical alarm:" << result.error.message;
        // Alarm escalation logic here
    }
});
```

### 5.3 Device Registration on Startup

```cpp
// Create device info
DeviceInfo deviceInfo;
deviceInfo.deviceId = "ZM-ICU-MON-04";
deviceInfo.deviceLabel = "ICU-MON-04";
deviceInfo.serialNumber = "ZM-2024-0001234";
deviceInfo.firmwareVersion = "1.0.0";
deviceInfo.capabilities = {"VITALS_MONITORING", "ALARM_SYSTEM", "ECG_WAVEFORM"};
deviceInfo.status = "ONLINE";
deviceInfo.ipAddress = "10.1.50.104";

// Register device
auto future = telemetryServer->registerDevice(deviceInfo);

future.then([](const RegistrationResult& result) {
    if (result.success) {
        qDebug() << "Device registered successfully";
        qDebug() << "Session ID:" << result.sessionId;
        qDebug() << "Server time:" << result.serverTime;
        
        // Apply server configuration
        Settings::instance()->setSyncInterval(result.configuration.syncInterval);
    } else {
        qCritical() << "Device registration failed:" << result.error;
    }
});
```

### 5.4 Batch Transmission with Retry Logic

```cpp
/**
 * @brief Send batch with automatic retry on failure.
 */
class TelemetryTransmitter {
public:
    void sendBatchWithRetry(const TelemetryBatch& batch, int maxRetries = 3) {
        sendBatchAttempt(batch, 0, maxRetries);
    }

private:
    void sendBatchAttempt(const TelemetryBatch& batch, int attempt, int maxRetries) {
        auto future = m_server->sendVitalsBatch(batch);
        
        future.then([this, batch, attempt, maxRetries](const TransmissionResult& result) {
            if (result.isSuccess()) {
                qDebug() << "Transmission succeeded on attempt" << (attempt + 1);
                recordMetrics(result.metrics);
                return;
            }
            
            // Failed
            if (!result.error.retryable) {
                qCritical() << "Non-retryable error:" << result.error.message;
                return;
            }
            
            if (attempt < maxRetries) {
                // Exponential backoff: 1s, 2s, 4s, 8s, ...
                int delayMs = qPow(2, attempt) * 1000;
                qDebug() << "Retrying in" << delayMs << "ms (attempt" << (attempt + 2) << ")";
                
                QTimer::singleShot(delayMs, [this, batch, attempt, maxRetries]() {
                    sendBatchAttempt(batch, attempt + 1, maxRetries);
                });
            } else {
                qCritical() << "Max retries exceeded. Queuing for later.";
                queueForLater(batch);
            }
        });
    }
    
    void queueForLater(const TelemetryBatch& batch) {
        // Mark as unsync in database
        // Will be retried when network restored
    }
    
    ITelemetryServer* m_server;
};
```

---

## 6. Error Handling

### 6.1 Error Codes and Recovery

| Error Code | HTTP Status | Retryable? | Recovery Strategy |
|------------|------------|------------|-------------------|
| `NetworkError` | - | Yes | Retry with backoff, queue if persists |
| `Timeout` | 408 | Yes | Retry immediately (1-2 times), then backoff |
| `AuthenticationError` | 401 | No | Certificate issue - alert IT, check certificate expiry |
| `ServerError` | 500 | Yes | Server overloaded - retry with backoff |
| `InvalidData` | 400 | No | Bug in code - log and investigate |
| `SignatureError` | 403 | No | Secret key mismatch - check provisioning |

### 6.2 Error Handling Best Practices

```cpp
// DON'T: Ignore transmission errors
auto future = telemetryServer->sendVitalsBatch(batch);
// ... no error handling (data loss if fails!)

// DO: Always handle errors
auto future = telemetryServer->sendVitalsBatch(batch);
future.then([](const TransmissionResult& result) {
    if (result.isSuccess()) {
        handleSuccess(result);
    } else {
        handleError(result.error);
        
        // Queue for retry if retryable
        if (result.error.retryable) {
            queueForRetry(batch);
        } else {
            logCriticalError(result.error);
        }
    }
});

// DO: Monitor transmission metrics
if (result.isSuccess()) {
    if (result.metrics.endToEndLatencyMs > 5000) {
        qWarning() << "Slow transmission detected:" << result.metrics.endToEndLatencyMs << "ms";
        // Consider network diagnostics
    }
}
```

---

## 7. Security

### 7.1 mTLS Configuration

```cpp
/**
 * @brief Configure mTLS for secure communication.
 */
void NetworkTelemetryServer::configureMtls(QNetworkRequest& request) {
    // Configure SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    
    // TLS 1.2+ only
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    
    // Set client certificate and key
    sslConfig.setLocalCertificate(m_clientCert);
    sslConfig.setPrivateKey(m_clientKey);
    
    // Set CA certificate for server verification
    sslConfig.setCaCertificates({m_caCert});
    
    // Verify server certificate
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    
    // Certificate pinning (optional, recommended)
    // sslConfig.setCaCertificates({pinnedCert});
    
    // Apply SSL configuration
    request.setSslConfiguration(sslConfig);
}
```

### 7.2 Data Signing

```cpp
/**
 * @brief Sign telemetry batch with HMAC-SHA256.
 */
void TelemetryBatch::sign(const QByteArray& secretKey) {
    // Serialize batch data (excluding signature field)
    QByteArray data = toJson();
    
    // Compute HMAC-SHA256
    QMessageAuthenticationCode mac(QCryptographicHash::Sha256);
    mac.setKey(secretKey);
    mac.addData(data);
    signature = QString::fromLatin1(mac.result().toHex());
}

/**
 * @brief Verify batch signature.
 */
bool TelemetryBatch::verifySignature(const QByteArray& secretKey) const {
    // Recompute signature
    QByteArray data = toJson();
    QMessageAuthenticationCode mac(QCryptographicHash::Sha256);
    mac.setKey(secretKey);
    mac.addData(data);
    QString expected = QString::fromLatin1(mac.result().toHex());
    
    // Compare
    return signature == expected;
}
```

---

## 8. Performance Optimization

### 8.1 Batch Size Tuning

```cpp
/**
 * @brief Determine optimal batch size for transmission.
 * 
 * Balance between:
 * - Latency (smaller batches = lower latency)
 * - Overhead (larger batches = less HTTP overhead)
 * - Network quality (poor network = smaller batches)
 */
int calculateOptimalBatchSize(int networkQuality) {
    if (networkQuality > 80) {
        return 100; // Excellent network: large batches
    } else if (networkQuality > 50) {
        return 50;  // Good network: medium batches
    } else {
        return 10;  // Poor network: small batches
    }
}
```

### 8.2 Connection Pooling

```cpp
/**
 * @brief Use connection pooling for efficiency.
 * 
 * Qt Network Manager handles connection pooling automatically,
 * but configure appropriately for medical device context.
 */
void NetworkTelemetryServer::configureConnectionPooling() {
    // Persistent connections (HTTP keep-alive)
    m_network->setAutoDeleteReplies(true);
    
    // Connection timeout
    m_network->setTransferTimeout(10000); // 10 seconds
    
    // Note: Qt handles connection pooling internally
}
```

---

## 9. Testing

### 9.1 Unit Test Examples

```cpp
TEST(ITelemetryServer, SendVitalsBatchSuccess) {
    // Arrange
    MockTelemetryServer server;
    
    TelemetryBatch batch;
    batch.batchId = "batch-001";
    batch.deviceId = "ZM-TEST-01";
    batch.patientMrn = "MRN-12345";
    
    VitalRecord vital;
    vital.timestamp = QDateTime::currentDateTime();
    vital.heartRate = 75;
    vital.spo2 = 98.0;
    batch.vitals.append(vital);
    
    // Act
    auto future = server.sendVitalsBatch(batch);
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.recordsReceived, 1);
    EXPECT_LT(result.endToEndLatencyMs(), 1000);
    
    // Verify batch was received
    auto sentBatches = server.getSentBatches();
    ASSERT_EQ(sentBatches.size(), 1);
    EXPECT_EQ(sentBatches[0].batchId, "batch-001");
}

TEST(ITelemetryServer, SendAlarmHighPriority) {
    // Arrange
    MockTelemetryServer server;
    
    AlarmEvent alarm;
    alarm.alarmId = "alarm-001";
    alarm.priority = "HIGH";
    alarm.alarmType = "HR_HIGH";
    
    // Act
    auto future = server.sendAlarm(alarm);
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isSuccess());
}

TEST(ITelemetryServer, NetworkErrorRetryable) {
    // Arrange
    MockTelemetryServer server;
    server.setSimulateFailure(true);
    
    TelemetryBatch batch;
    // ... setup batch ...
    
    // Act
    auto future = server.sendVitalsBatch(batch);
    auto result = future.result();
    
    // Assert
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error.code, ErrorCode::NetworkError);
    EXPECT_TRUE(result.error.retryable);
}
```

---

## 10. Integration with MonitoringService

```cpp
/**
 * @brief MonitoringService uses ITelemetryServer for transmission.
 */
class MonitoringService {
public:
    MonitoringService(ITelemetryServer* telemetryServer)
        : m_telemetryServer(telemetryServer) {
        
        // Start periodic batch transmission
        m_batchTimer = new QTimer(this);
        connect(m_batchTimer, &QTimer::timeout, this, &MonitoringService::transmitBatch);
        m_batchTimer->start(10000); // Every 10 seconds
    }
    
    void onVitalSignReceived(const VitalRecord& vital) {
        // Add to current batch
        m_currentBatch.vitals.append(vital);
        
        // If batch full, transmit immediately
        if (m_currentBatch.vitals.size() >= 100) {
            transmitBatch();
        }
    }
    
private slots:
    void transmitBatch() {
        if (m_currentBatch.vitals.isEmpty()) {
            return; // Nothing to send
        }
        
        // Finalize batch
        m_currentBatch.batchId = QUuid::createUuid().toString();
        m_currentBatch.created_at = QDateTime::currentDateTime();
        m_currentBatch.sign(m_deviceSecret);
        
        // Send to server
        auto future = m_telemetryServer->sendVitalsBatch(m_currentBatch);
        
        future.then([this](const TransmissionResult& result) {
            if (result.isSuccess()) {
                // Success - clear batch
                m_currentBatch.vitals.clear();
                
                // Record metrics
                recordMetrics(result.metrics);
            } else {
                // Failed - queue for retry
                queueForRetry(m_currentBatch);
            }
        });
    }
    
    ITelemetryServer* m_telemetryServer;
    TelemetryBatch m_currentBatch;
    QByteArray m_deviceSecret;
    QTimer* m_batchTimer;
};
```

---

## 11. Related Documents

- **Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) (REQ-FUN-DATA-001)
- **Requirements:** [06_INTERFACE_REQUIREMENTS.md](../../requirements/06_INTERFACE_REQUIREMENTS.md) (REQ-INT-SRV-001)
- **Use Cases:** [02_USE_CASES.md](../../requirements/02_USE_CASES.md) (UC-DS-001, UC-DS-002)
- **Security:** [06_SECURITY.md](../06_SECURITY.md)
- **Database:** [10_DATABASE_DESIGN.md](../10_DATABASE_DESIGN.md) (telemetry_metrics table)

---

*This interface enables secure, reliable telemetry transmission with comprehensive metrics tracking and error handling.*

