---
doc_id: DOC-API-002
title: ITelemetryServer Interface
version: 1.0
category: API
subcategory: Network Communication
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [api, interface, telemetry, server, mtls, network, transmission]
related_docs:
  - DOC-COMP-010 # MonitoringService
  - DOC-COMP-003 # TelemetryBatch
  - DOC-COMP-016 # ITelemetryRepository
  - DOC-ARCH-002 # Architecture Patterns
  - DOC-ARCH-005 # Data Flow and Caching
authors:
  - Z Monitor Team
reviewers:
  - Architecture Team
---

# ITelemetryServer Interface

## 1. Overview

The `ITelemetryServer` interface defines the contract for transmitting vital signs, alarms, and telemetry data to the central server using secure mTLS (mutual TLS) connections.

**Purpose:**
- Abstract central server communication from application logic
- Enable testing with mock implementations
- Support batch transmission for efficiency
- Track comprehensive transmission metrics and latency

**Key Characteristics:**
- **Secure:** All transmissions use mTLS for authentication and encryption
- **Asynchronous:** Returns `QFuture<TransmissionResult>` for non-blocking operations
- **Batch-Oriented:** Supports efficient batch transmission of vitals (10-100 records/batch)
- **High-Priority Alarms:** Critical alarms sent immediately (not batched)
- **Metrics Tracking:** Records timing metrics (DB latency, network latency, server latency)

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
     */
    virtual QFuture<TransmissionResult> sendVitalsBatch(const TelemetryBatch& batch) = 0;

    /**
     * @brief Send alarm event to central server (high priority).
     * 
     * Sends single alarm immediately (not batched) due to urgency.
     * 
     * @param alarm Alarm event data
     * @return QFuture<TransmissionResult> Future resolving to transmission status
     * 
     * @note Asynchronous operation (5-second timeout for alarms)
     */
    virtual QFuture<TransmissionResult> sendAlarm(const AlarmEvent& alarm) = 0;

    /**
     * @brief Register device with central server.
     * 
     * Registers device on startup, providing device metadata and establishing session.
     * 
     * @param deviceInfo Device metadata (ID, label, version, capabilities)
     * @return QFuture<RegistrationResult> Future resolving to session info
     * 
     * @note Call once on startup
     */
    virtual QFuture<RegistrationResult> registerDevice(const DeviceInfo& deviceInfo) = 0;

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
    void transmissionSucceeded(const QString& batchId, const TransmissionMetrics& metrics);
    void transmissionFailed(const QString& batchId, const TransmissionError& error);
    void connectionStatusChanged(bool connected);
    void serverConfigurationReceived(const ServerConfiguration& config);
};
```

---

## 3. Data Structures

See full interface documentation for details on:
- **TelemetryBatch** - Batch container with vitals, alarms, signature (HMAC-SHA256)
- **VitalRecord** - Single vital measurement (HR, SpO2, RR, BP, etc.)
- **AlarmEvent** - Alarm event with priority, type, thresholds, timestamps
- **TransmissionResult** - Success/failure with metrics and error details
- **TransmissionMetrics** - Detailed timing metrics (DB, network, server, end-to-end latency)
- **TransmissionError** - Error code, message, HTTP status, retryability flag

**Key Performance Metrics:**
- **End-to-End Latency:** created_at → server_ack_at
- **DB Latency:** created_at → saved_at
- **Transmission Latency:** transmitted_at → server_received_at
- **Server Latency:** server_received_at → server_ack_at

**Latency Quality Classification:**
- **EXCELLENT:** < 100ms
- **GOOD:** 100-500ms
- **ACCEPTABLE:** 500-2000ms
- **SLOW:** 2000-5000ms
- **CRITICAL:** > 5000ms

---

## 4. Implementations

### 4.1 NetworkTelemetryServer (Production)

**Technology:** Qt Network (HTTPS/mTLS)

**Features:**
- HTTPS/mTLS with client certificate authentication
- TLS 1.2+ only (secure cipher suites)
- 10-second timeout for vitals batch, 5-second for alarms
- Automatic retry with exponential backoff (1s, 2s, 4s, 8s...)
- Records all transmission metrics to telemetry_metrics table

### 4.2 MockTelemetryServer (Testing)

**Technology:** In-memory (no network)

**Features:**
- Simulated network delay (configurable 0-500ms)
- Simulated failure modes for testing error handling
- No external dependencies
- Verification of sent batches/alarms for test assertions

---

## 5. Usage Examples

### 5.1 Send Vitals Batch

```cpp
// Create telemetry batch
TelemetryBatch batch;
batch.batchId = QUuid::createUuid().toString();
batch.deviceId = m_deviceId;
batch.patientMrn = m_currentPatientMrn;
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
        m_logService->info("Transmission succeeded", {
            {"latencyMs", QString::number(result.endToEndLatencyMs())},
            {"quality", result.metrics.latencyQuality()}
        });
    } else {
        m_logService->warning("Transmission failed", {
            {"error", result.error.message},
            {"retryable", result.error.retryable ? "true" : "false"}
        });
    }
});
```

### 5.2 Send Critical Alarm (Immediate)

```cpp
// Create alarm event
AlarmEvent alarm;
alarm.alarmId = QUuid::createUuid().toString();
alarm.deviceId = m_deviceId;
alarm.patientMrn = m_currentPatientMrn;
alarm.timestamp = QDateTime::currentDateTime();
alarm.priority = "HIGH";
alarm.alarmType = "HR_HIGH";
alarm.value = 150.0;
alarm.threshold = 120.0;
alarm.status = "ACTIVE";

// Send immediately (not batched)
auto future = telemetryServer->sendAlarm(alarm);
```

---

## 6. Error Handling

### 6.1 Error Codes and Recovery

| Error Code            | HTTP Status | Retryable? | Recovery Strategy                           |
| --------------------- | ----------- | ---------- | ------------------------------------------- |
| `NetworkError`        | -           | Yes        | Retry with backoff, queue if persists       |
| `Timeout`             | 408         | Yes        | Retry immediately (1-2 times), then backoff |
| `AuthenticationError` | 401         | No         | Certificate issue - alert IT                |
| `ServerError`         | 500         | Yes        | Server overloaded - retry with backoff      |
| `InvalidData`         | 400         | No         | Bug in code - log and investigate           |
| `SignatureError`      | 403         | No         | Secret key mismatch - check provisioning    |

---

## 7. Security

### 7.1 mTLS Configuration

- **TLS 1.2+ Only:** Enforced minimum protocol version
- **Client Certificate:** Device certificate for authentication
- **Server Verification:** Verify server certificate against CA
- **Certificate Pinning:** Optional (recommended for production)

### 7.2 Data Signing

- **HMAC-SHA256:** All batches signed with device secret
- **Signature Verification:** Server verifies signature before processing
- **Prevents Tampering:** Ensures data integrity during transmission

---

## 8. Performance Optimization

### 8.1 Batch Size Tuning

```cpp
// Optimal batch size based on network quality
int calculateOptimalBatchSize(int networkQuality) {
    if (networkQuality > 80) return 100; // Excellent: large batches
    if (networkQuality > 50) return 50;  // Good: medium batches
    return 10;                           // Poor: small batches
}
```

### 8.2 Connection Pooling

Qt Network Manager handles connection pooling automatically with persistent HTTP keep-alive connections.

---

## 9. Testing

See full interface documentation for comprehensive unit test examples covering:
- Successful batch transmission
- High-priority alarm transmission
- Network error retry logic
- Timeout handling
- Authentication errors

---

## 10. Related Documents

- **DOC-COMP-010:** MonitoringService - Generates telemetry batches
- **DOC-COMP-003:** TelemetryBatch - Domain entity for batched vitals
- **DOC-COMP-016:** ITelemetryRepository - Persistence for transmission metrics
- **DOC-ARCH-002:** Architecture Patterns - Infrastructure layer interfaces
- **DOC-ARCH-005:** Data Flow and Caching - Telemetry transmission flow

---

## 11. Changelog

| Version | Date       | Author         | Changes                                        |
| ------- | ---------- | -------------- | ---------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from INTERFACE-002, added frontmatter |

---

*This interface enables secure, reliable telemetry transmission with comprehensive metrics tracking, error handling, and retry logic.*
