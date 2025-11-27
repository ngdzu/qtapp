# Error Handling Strategy

This document defines the error handling patterns, error codes, exception policies, and recovery strategies for the Z Monitor application.

## 1. Guiding Principles

- **Fail Fast, Fail Clearly:** Errors should be detected and reported immediately with clear, actionable messages
- **No Silent Failures:** All errors must be logged and, when appropriate, surfaced to the user
- **Graceful Degradation:** System should continue operating in degraded mode when possible
- **Recovery-Oriented:** Design for automatic recovery where feasible
- **Type Safety:** Use type-safe error handling mechanisms (avoid raw error codes where possible)

## 2. Error Handling Patterns

### 2.1. Result<T, E> Pattern (Recommended)

For operations that can fail, use a `Result<T, E>` type that encapsulates either success or error:

```cpp
template<typename T, typename E = Error>
class Result {
public:
    bool isSuccess() const;
    bool isError() const;
    T value() const;  // Throws if error
    E error() const;  // Throws if success
    T valueOr(const T& defaultValue) const;
};
```

**When to Use:**
- Network operations (`NetworkManager::sendTelemetry()`)
- Database operations (`DatabaseManager::saveData()`)
- File I/O operations
- Certificate validation
- Patient lookup operations

**Example:**
```cpp
Result<QList<VitalSign>, NetworkError> result = networkManager->sendTelemetry(data);
if (result.isError()) {
    LogService::error("Telemetry send failed: " + result.error().message());
    // Handle error
} else {
    QList<VitalSign> processed = result.value();
    // Process success
}
```

### 2.2. Qt Signal/Slot Error Propagation

For asynchronous operations, use signals to propagate errors:

```cpp
class NetworkManager : public QObject {
    Q_OBJECT
signals:
    void telemetrySent(const TelemetryData& data, const ServerResponse& response);
    void telemetrySendFailed(const TelemetryData& data, const NetworkError& error);
};
```

**When to Use:**
- Asynchronous operations
- Long-running operations
- Operations that need UI feedback

### 2.3. Exception Handling (Limited Use)

Exceptions should be used sparingly and only for exceptional circumstances:

**When to Use Exceptions:**
- Programming errors (assertions, invariants violated)
- Resource exhaustion (out of memory, file handles)
- Critical system failures that cannot be recovered

**When NOT to Use Exceptions:**
- Expected error conditions (network failures, invalid input)
- Business logic errors (patient not found, invalid credentials)
- Recoverable errors (retry-able operations)

**Exception Policy:**
- Use exceptions only in C++ backend (not in QML)
- Catch exceptions at service boundaries
- Never let exceptions propagate to Qt event loop
- Log all exceptions before handling

**Example:**
```cpp
void DatabaseManager::openDatabase(const QString& path) {
    try {
        // Database operations
    } catch (const std::bad_alloc& e) {
        LogService::critical("Out of memory: " + QString::fromStdString(e.what()));
        emit databaseError(DatabaseError::OutOfMemory);
        throw;  // Re-throw critical errors
    } catch (const std::exception& e) {
        LogService::error("Database error: " + QString::fromStdString(e.what()));
        emit databaseError(DatabaseError::Unknown);
    }
}
```

### 2.4. Error Codes (For Interoperability)

For network protocols and external APIs, use standardized error codes:

```cpp
enum class NetworkErrorCode {
    Success = 0,
    ConnectionTimeout = 1001,
    ConnectionRefused = 1002,
    CertificateValidationFailed = 1003,
    ServerError = 2000,
    InvalidRequest = 4000,
    Unauthorized = 4001,
    NotFound = 4004,
    InternalServerError = 5000
};
```

## 3. Error Categories

### 3.1. Recoverable Errors

Errors that can be automatically retried or recovered:

- **Network Timeouts:** Retry with exponential backoff
- **Temporary Database Locks:** Retry after short delay
- **Certificate Expiration Warnings:** Log and continue (renewal handled separately)

**Recovery Strategy:**
- Automatic retry with exponential backoff
- Maximum retry attempts (configurable, default: 3)
- Log retry attempts
- Emit signals for UI feedback

### 3.2. Non-Recoverable Errors

Errors that require user intervention or system restart:

- **Invalid Configuration:** User must fix settings
- **Certificate Revocation:** Device must be re-provisioned
- **Database Corruption:** Requires database recovery or reset
- **Authentication Failure:** User must re-authenticate

**Recovery Strategy:**
- Stop affected operation
- Log error with full context
- Emit error signal for UI
- Provide clear error message to user
- Suggest recovery actions

### 3.3. Critical Errors

Errors that threaten system integrity:

- **Out of Memory:** System may be unstable
- **Disk Full:** Cannot persist data
- **Security Violation:** Potential breach detected

**Recovery Strategy:**
- Immediate escalation
- Log to security audit log
- Enter safe mode (degraded functionality)
- Alert user with prominent UI
- Consider automatic restart for critical subsystems

## 4. Error Logging

### 4.1. Log Levels

Use appropriate log levels for errors:

- **ERROR:** Recoverable errors, operation failures
- **WARNING:** Degraded functionality, retry attempts
- **CRITICAL:** System-threatening errors, security violations
- **FATAL:** Unrecoverable errors requiring restart

### 4.2. Error Context

Always include context in error logs:

```cpp
LogService::error(
    QString("Telemetry send failed: %1")
        .arg(error.message()),
    {
        {"deviceId", deviceId},
        {"patientMrn", patientMrn},
        {"timestamp", QDateTime::currentDateTimeUtc().toString()},
        {"errorCode", QString::number(error.code())},
        {"retryAttempt", QString::number(retryCount)}
    }
);
```

### 4.3. Security Audit Logging

Security-related errors must be logged to `security_audit_log`:

- Authentication failures
- Certificate validation failures
- Network connection failures
- Data integrity violations
- Unauthorized access attempts

## 5. Error Recovery Patterns

### 5.1. Retry with Exponential Backoff

```cpp
class RetryPolicy {
public:
    Result<T, E> executeWithRetry(std::function<Result<T, E>()> operation) {
        int attempt = 0;
        while (attempt < maxRetries) {
            auto result = operation();
            if (result.isSuccess()) {
                return result;
            }
            if (!isRetryable(result.error())) {
                return result;  // Don't retry non-retryable errors
            }
            int delay = baseDelay * (1 << attempt);  // Exponential backoff
            QThread::msleep(delay);
            attempt++;
        }
        return Result<T, E>::error(RetryExhaustedError());
    }
};
```

### 5.2. Circuit Breaker Pattern

For external services (network, patient lookup):

```cpp
class CircuitBreaker {
    enum State { Closed, Open, HalfOpen };
    
    Result<T, E> execute(std::function<Result<T, E>()> operation) {
        if (state == Open) {
            if (shouldAttemptReset()) {
                state = HalfOpen;
            } else {
                return Result<T, E>::error(CircuitOpenError());
            }
        }
        
        auto result = operation();
        if (result.isSuccess()) {
            onSuccess();
        } else {
            onFailure();
        }
        return result;
    }
};
```

### 5.3. Graceful Degradation

When non-critical operations fail, continue with reduced functionality:

- **Network Offline:** Continue local operation, queue data for sync
- **Patient Lookup Failed:** Use cached patient data if available
- **Analytics Unavailable:** Continue monitoring without predictive scores
- **Trends View Unavailable:** Show current vitals only

## 6. Error Propagation in Qt/QML

### 6.1. C++ to QML Error Propagation

Use properties and signals:

```cpp
class PatientController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    
signals:
    void errorMessageChanged(const QString& message);
    void patientLookupFailed(const QString& patientId, const QString& error);
};
```

### 6.2. QML Error Handling

```qml
PatientController {
    id: patientController
    
    onPatientLookupFailed: (patientId, error) => {
        errorDialog.show("Failed to lookup patient: " + error);
    }
    
    Connections {
        target: patientController
        function onError() {
            // Handle error
        }
    }
}
```

## 7. Error Codes Reference

### 7.1. Network Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 1001 | ConnectionTimeout | Connection timed out | Retry with backoff |
| 1002 | ConnectionRefused | Server refused connection | Check server status |
| 1003 | CertificateValidationFailed | Certificate invalid | Re-provision device |
| 1004 | TLSError | TLS handshake failed | Check certificates |
| 1005 | RateLimitExceeded | Too many requests | Wait and retry |

### 7.2. Database Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 2001 | DatabaseLocked | Database is locked | Retry after delay |
| 2002 | DatabaseCorrupted | Database corruption detected | Run recovery |
| 2003 | QueryFailed | SQL query failed | Check query syntax |
| 2004 | TransactionFailed | Transaction rollback | Retry transaction |

### 7.3. Authentication Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 3001 | InvalidCredentials | Invalid PIN | User re-enters PIN |
| 3002 | AccountLocked | Account locked (brute force) | Wait for lockout period |
| 3003 | SessionExpired | Session expired | Re-authenticate |
| 3004 | Unauthorized | Insufficient permissions | Check user role |

### 7.4. Patient Management Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 4001 | PatientNotFound | Patient not found in system | Verify MRN |
| 4002 | LookupServiceUnavailable | Patient lookup service down | Use cached data |
| 4003 | InvalidMRN | Invalid MRN format | User corrects input |
| 4004 | AdmissionFailed | Patient admission failed | Check patient status |

## 8. Error Handling Best Practices

### 8.1. Do's

- ✅ Always check return values and handle errors
- ✅ Provide clear, actionable error messages
- ✅ Log errors with full context
- ✅ Use appropriate error types (recoverable vs. non-recoverable)
- ✅ Implement retry logic for transient errors
- ✅ Emit signals for async error propagation
- ✅ Validate inputs early to prevent errors

### 8.2. Don'ts

- ❌ Don't ignore errors silently
- ❌ Don't use exceptions for expected error conditions
- ❌ Don't expose implementation details in user-facing errors
- ❌ Don't retry non-retryable errors indefinitely
- ❌ Don't catch exceptions without handling them
- ❌ Don't use raw error codes without type safety

## 9. Testing Error Handling

### 9.1. Unit Tests

- Test all error paths
- Verify error messages are clear
- Test retry logic
- Test circuit breaker behavior
- Verify error logging

### 9.2. Integration Tests

- Test error propagation across components
- Test recovery from transient errors
- Test graceful degradation
- Verify error signals are emitted correctly

### 9.3. Error Injection

Use mock objects to inject errors:

```cpp
class MockTelemetryServer : public ITelemetryServer {
    void injectError(NetworkError error) {
        nextError = error;
    }
    
    Result<ServerResponse, NetworkError> SendTelemetryAsync(...) override {
        if (nextError) {
            return Result<ServerResponse, NetworkError>::error(nextError);
        }
        // Normal operation
    }
};
```

## 10. Error Monitoring and Alerting

### 10.1. Error Metrics

Track error rates and patterns:

- Error rate per operation type
- Error rate over time
- Most common errors
- Recovery success rate
- Circuit breaker state changes

### 10.2. Alerting Thresholds

Alert on:

- Error rate exceeds threshold (e.g., > 5% of operations)
- Critical errors occur
- Circuit breaker opens
- Retry exhaustion
- Security-related errors

## 11. Related Documents

- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Error handling in multi-threaded context
- [06_SECURITY.md](./06_SECURITY.md) - Security error handling
- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Error handling tests
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Error handling in class designs

