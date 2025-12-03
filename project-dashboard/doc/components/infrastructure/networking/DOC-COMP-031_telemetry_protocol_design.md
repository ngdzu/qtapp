---
title: "Telemetry Protocol Design"
doc_id: DOC-COMP-031
version: 2.0
category: Component
phase: 6D
status: Active
created: 2025-12-01
updated: 2025-12-02
author: migration-bot
related:
  - DOC-COMP-030_telemetry_server.md
  - DOC-ARCH-028_domain_driven_design.md
---

# Telemetry Protocol Design

This document describes the design of the telemetry protocol used in Z Monitor, including batching, compression, retry/backoff, circuit breaker, and transport security (TLS 1.3).

## Architecture Overview

The telemetry system follows a layered approach:

1. **Application Layer** (`TelemetryService`): Batches vital signs and alarm events, manages retry policy and circuit breaker.
2. **Infrastructure Layer** (`HttpTelemetryServerAdapter`): Handles HTTP(S) transport, TLS 1.3 configuration, gzip compression, and timeout enforcement.
3. **Interface** (`ITelemetryServer`): Abstract interface decoupling service from transport implementation.

## Batching Strategy

**Objective:** Reduce network overhead by batching telemetry data before upload.

- **Batch Interval:** 10 minutes (configurable via `TelemetryService::setBatchIntervalMs()`)
- **Batch Format:** Newline-delimited JSON payloads (vitals and alarms)
- **Flush Triggers:**
  - Timer-based: Every 10 minutes
  - Manual: `TelemetryService::flushNow()` for testing or forced upload
  - Service stop: Flush pending batch on shutdown

**Implementation:**
```cpp
void TelemetryService::enqueueVital(const QByteArray &payload);
void TelemetryService::enqueueAlarm(const QByteArray &payload);
```

Payloads are appended to `m_batchBuffer` with newline separators. When the timer fires or `flushNow()` is called, the batch is compressed and uploaded.

## Compression

**Objective:** Reduce bandwidth usage for telemetry uploads.

- **Algorithm:** gzip (zlib-compatible; `qCompress()` for prototyping, real gzip in production)
- **Content-Encoding Header:** `Content-Encoding: gzip` set by `HttpTelemetryServerAdapter`
- **Compression Ratio:** Typically 3:1 to 5:1 for JSON telemetry data

**Implementation:**
```cpp
QByteArray TelemetryService::compressGzip(const QByteArray &input) const {
    return qCompress(input, 6); // zlib format; production uses real gzip
}
```

The adapter receives compressed data and sets the appropriate HTTP header.

## Retry Policy with Exponential Backoff

**Objective:** Handle transient network failures without overwhelming the server.

- **Max Attempts:** Configurable (default: 3)
- **Initial Delay:** 1000ms
- **Backoff Multiplier:** 2.0x per attempt
- **Delays:** 1s, 2s, 4s (for 3 attempts)

**Implementation:**
```cpp
class RetryPolicy {
public:
    RetryPolicy(int maxAttempts = 3, std::chrono::milliseconds initialDelay = std::chrono::milliseconds(1000), double backoffMultiplier = 2.0);
    int maxAttempts() const;
    std::chrono::milliseconds delayForAttempt(int attempt) const;
};
```

`TelemetryService::flushBatch()` loops up to `maxAttempts`, sleeping between retries using `QThread::msleep()`.

## Circuit Breaker

**Objective:** Prevent cascading failures by stopping upload attempts when the server is consistently unavailable.

- **Failure Threshold:** 3 consecutive failures opens the circuit
- **Open State:** All upload attempts are rejected immediately
- **Reset:** Circuit closes after one successful upload

**Implementation:**
```cpp
class CircuitBreaker {
public:
    CircuitBreaker(int failureThreshold = 3);
    bool isOpen() const;
    void recordSuccess();
    void recordFailure();
};
```

Before uploading, `TelemetryService` checks `m_circuitBreaker.isOpen()`. If open, upload is skipped and `uploadFailed("circuit breaker open")` is emitted.

## HTTP Transport Layer (HttpTelemetryServerAdapter)

**Objective:** Provide secure, reliable HTTP(S) uploads with TLS 1.3.

### Configuration

- **Endpoint:** Configurable URL (e.g., `https://central.example.com/telemetry`)
- **Method:** HTTP POST
- **Content-Type:** `application/octet-stream`
- **Content-Encoding:** `gzip`
- **Timeout:** 15 seconds (configurable)

### TLS 1.3

```cpp
QSslConfiguration HttpTelemetryServerAdapter::tls13Config() const {
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setProtocol(QSsl::TlsV1_3);
    return conf;
}
```

**Note:** On macOS with SecureTransport backend, TLS 1.3 may not be fully supported. Production deployments should use OpenSSL-based Qt builds or verify TLS 1.3 support on target platforms.

### Client Certificates (mTLS)

For mutual TLS authentication:
```cpp
adapter.setClientCertificates(certs, privateKey);
```

### Timeout Enforcement

Timeouts are enforced using `QEventLoop` and `QTimer`:
```cpp
QEventLoop loop;
QTimer timer;
timer.setSingleShot(true);
connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
timer.start(m_timeoutMs);
loop.exec();

if (!reply->isFinished()) {
    reply->abort(); // Timeout occurred
}
```

### Error Handling

- **Timeout:** Returns `false` with `errorOut = "timeout"`
- **HTTP Errors:** Returns `false` with `errorOut = "http 500"` (or actual status code)
- **Network Errors:** `QNetworkReply::error()` reported in `errorOut`

## Message Format

### Batch Payload Structure

Each batch is a newline-delimited stream of JSON objects:

```
{"type":"vital","mrn":"12345","hr":72,"spo2":98,"timestamp":"2025-12-02T10:15:00Z"}
{"type":"alarm","mrn":"12345","severity":"critical","message":"SpO2 < 90%","timestamp":"2025-12-02T10:16:30Z"}
{"type":"vital","mrn":"12345","hr":75,"spo2":97,"timestamp":"2025-12-02T10:17:00Z"}
```

This format is then gzip-compressed before upload.

### Response Format

Expected server response:
- **Success:** HTTP 200 OK (empty body or JSON acknowledgment)
- **Failure:** HTTP 4xx/5xx with optional error details in body

## Testing Strategy

### Unit Tests

1. **TelemetryServiceTest:**
   - Timer-based batch flush
   - Retry with exponential backoff
   - Circuit breaker blocks uploads

2. **HttpTelemetryServerAdapterTest:**
   - TLS 1.3 configuration applied
   - Content-Encoding: gzip header set
   - Timeout enforcement

### Integration Tests

1. **TelemetryWorkflowTest:**
   - End-to-end batch upload with compression
   - Mock server simulates transient failure → success

2. **HttpAdapterWorkflowTest:**
   - Local HTTP server returns 500 → adapter returns false
   - Second request returns 200 → adapter returns true

## Verification Checklist

- [x] **Functional:** Batching, compression, retry/backoff, circuit breaker, TLS 1.3 configuration verified
- [x] **Code Quality:** No hardcoded values; retry/circuit breaker configurable; DDD boundaries preserved
- [x] **Documentation:** Protocol design, batching, compression, retry/backoff, circuit breaker, TLS documented
- [x] **Integration:** Unit and integration tests passing (TelemetryService, HttpTelemetryServerAdapter)
- [x] **Tests:** Unit tests: PASS. Integration tests: PASS.

## References

- [Exponential Backoff and Jitter](https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/)
- [Circuit Breaker Pattern](https://martinfowler.com/bliki/CircuitBreaker.html)
- [Qt Network Module](https://doc.qt.io/qt-6/qtnetwork-index.html)
- [TLS 1.3 RFC 8446](https://datatracker.ietf.org/doc/html/rfc8446)

---
**Status:** ✅ Active - Implemented and verified (TASK-APP-004)
