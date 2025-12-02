# Error Handling Strategy

**Document ID:** DESIGN-020  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

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

## 4. When to Return Errors vs. When to Log Errors

### 4.1. Critical Decision: Return vs. Log

**Key Principle:** Return errors when the caller needs to handle them. Log errors when they need to be recorded for debugging, auditing, or system monitoring.

### 4.2. Return Errors (Don't Log)

**Return errors when:**
- **Caller needs to handle the error** (validation failures, business rule violations)
- **Error is expected** (user input validation, business constraints)
- **Caller can recover** (retry, alternative path, user feedback)
- **Error is part of normal flow** (patient not found, already admitted)

**Examples:**

```cpp
// ✅ GOOD: Return error for validation (caller handles it)
Result<void, Error> AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    // Business validation - return error, don't log
    if (cmd.mrn.length() < 3 || cmd.mrn.length() > 20) {
        return Error("Invalid MRN format");  // ✅ Return, no logging
    }
    
    if (isPatientAlreadyAdmitted(cmd.mrn)) {
        return Error("Patient already admitted");  // ✅ Return, no logging
    }
    
    // ... proceed with admission
}

// ✅ GOOD: Return error for expected business conditions
Result<PatientIdentity, Error> PatientRepository::findByMrn(const QString& mrn) {
    auto patient = queryPatient(mrn);
    if (!patient) {
        return Error("Patient not found");  // ✅ Return, no logging (expected condition)
    }
    return patient;
}
```

**Why not log?**
- These are expected conditions, not system failures
- Caller will handle the error appropriately (show to user, retry, etc.)
- Logging would create noise in logs
- Caller may log if needed (e.g., after retry exhaustion)

### 4.3. Log Errors (Don't Return, or Log Before Returning)

**Log errors when:**
- **System-level failures** (database connection lost, file I/O failed, network unreachable)
- **Unexpected conditions** (internal state corruption, programming errors)
- **Infrastructure failures** (certificate validation failed, disk full)
- **Security events** (authentication failures, unauthorized access)
- **Errors that need auditing** (all security events, critical operations)

**Examples:**

```cpp
// ✅ GOOD: Log infrastructure failures
Result<void, Error> DatabaseManager::savePatient(const Patient& patient) {
    if (!m_database.isOpen()) {
        // ✅ Log system failure (infrastructure problem)
        m_logService->error("Database not open", {
            {"operation", "savePatient"},
            {"patientMrn", patient.mrn}
        });
        return Error("Database unavailable");
    }
    
    QSqlQuery query;
    if (!query.exec(sql)) {
        // ✅ Log database error (system failure)
        m_logService->error("Database query failed", {
            {"operation", "savePatient"},
            {"patientMrn", patient.mrn},
            {"error", query.lastError().text()},
            {"sql", sql}
        });
        return Error("Failed to save patient");
    }
    
    return Success();
}

// ✅ GOOD: Log unexpected conditions
void MonitoringService::processVitalSign(const VitalRecord& vital) {
    if (vital.timestamp.isNull()) {
        // ✅ Log unexpected condition (should never happen)
        m_logService->warning("Received vital sign with null timestamp", {
            {"patientMrn", vital.patientMrn},
            {"value", QString::number(vital.heartRate)}
        });
        return;  // Skip invalid data
    }
    
    // ... process valid vital sign
}
```

### 4.4. Both Return AND Log

**Return AND log when:**
- **System failure that caller should handle** (network timeout, database error)
- **Error needs both user feedback AND audit trail** (authentication failure, security violation)
- **Recoverable error that needs monitoring** (retry-able failures)

**Examples:**

```cpp
// ✅ GOOD: Log system failure AND return error (caller handles, but we need audit trail)
Result<ServerResponse, Error> NetworkManager::sendTelemetry(const TelemetryBatch& batch) {
    auto result = m_telemetryServer->sendBatch(batch);
    
    if (result.isError()) {
        // ✅ Log for debugging/monitoring (system failure)
        m_logService->error("Telemetry send failed", {
            {"deviceId", batch.deviceId},
            {"patientMrn", batch.patientMrn},
            {"errorCode", QString::number(result.error().code())},
            {"retryAttempt", QString::number(m_retryCount)}
        });
        
        // ✅ Return error (caller can retry or show to user)
        return result;
    }
    
    return result;
}

// ✅ GOOD: Log security event AND return error
Result<UserProfile, Error> SecurityService::authenticate(const QString& userId, const QString& pin) {
    auto result = m_userMgmtService->authenticate(userId, pin);
    
    if (result.isError()) {
        // ✅ Log to security audit log (required for compliance)
        AuditEntry auditEntry;
        auditEntry.eventType = "LOGIN_FAILED";
        auditEntry.userId = userId;
        auditEntry.success = false;
        m_auditRepo->logAuditEvent(auditEntry);
        
        // ✅ Also log to application logs for troubleshooting
        m_logService->warning("Authentication failed", {
            {"userId", userId},
            {"reason", result.error().message()}
        });
        
        // ✅ Return error (caller shows to user)
        return result;
    }
    
    return result;
}
```

### 4.5. Decision Matrix

| Scenario | Return Error? | Log Error? | Rationale |
|----------|---------------|------------|-----------|
| **Input validation failure** (invalid MRN format) | ✅ Yes | ❌ No | Expected condition, caller handles |
| **Business rule violation** (patient already admitted) | ✅ Yes | ❌ No | Expected condition, caller handles |
| **Patient not found** | ✅ Yes | ❌ No | Expected condition, caller handles |
| **Database connection lost** | ✅ Yes | ✅ Yes | System failure, needs audit + caller handling |
| **Network timeout** | ✅ Yes | ✅ Yes | System failure, needs monitoring + caller retry |
| **Certificate validation failed** | ✅ Yes | ✅ Yes | Security event, needs audit + caller handling |
| **Authentication failure** | ✅ Yes | ✅ Yes | Security event, needs audit + user feedback |
| **Internal state corruption** (null timestamp) | ❌ No | ✅ Yes | Unexpected condition, log for debugging |
| **Out of memory** | ✅ Yes | ✅ Yes | Critical system failure, needs immediate attention |
| **Disk full** | ✅ Yes | ✅ Yes | Critical system failure, needs immediate attention |

### 4.6. Guidelines by Layer

#### **Domain Layer**
- **Return errors only** (no logging)
- Domain layer has no infrastructure dependencies
- Caller (application service) decides whether to log

```cpp
// ✅ GOOD: Domain layer returns errors, doesn't log
class PatientAggregate {
public:
    Result<void, Error> admit(const PatientIdentity& identity) {
        if (m_isAdmitted) {
            return Error("Patient already admitted");  // ✅ Return only
        }
        // ... domain logic
    }
};
```

#### **Application Layer**
- **Return errors for business validation** (don't log)
- **Log errors for infrastructure failures** (before returning)
- **Log security events** (before returning)

```cpp
// ✅ GOOD: Application service - return validation errors, log infrastructure errors
Result<void, Error> AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    // Business validation - return error, don't log
    if (cmd.mrn.isEmpty()) {
        return Error("MRN is required");  // ✅ Return only
    }
    
    // Infrastructure call - log failures, return error
    auto patientResult = m_patientRepo->findByMrn(cmd.mrn);
    if (patientResult.isError()) {
        // ✅ Log infrastructure failure
        m_logService->error("Failed to lookup patient", {
            {"mrn", cmd.mrn},
            {"error", patientResult.error().message()}
        });
        return patientResult;  // ✅ Return error
    }
    
    // ... proceed with admission
}
```

#### **Infrastructure Layer**
- **Log all failures** (infrastructure problems need debugging)
- **Return errors** (caller needs to know operation failed)

```cpp
// ✅ GOOD: Infrastructure layer - log failures, return errors
Result<PatientIdentity, Error> SQLitePatientRepository::findByMrn(const QString& mrn) {
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        // ✅ Log infrastructure failure
        m_logService->error("Database query failed", {
            {"mrn", mrn},
            {"error", query.lastError().text()},
            {"queryId", QueryId::Patient::FIND_BY_MRN}
        });
        
        // ✅ Return error
        return Error("Failed to find patient");
    }
    
    // ... process result
}
```

### 4.7. Common Patterns

#### **Pattern 1: Validation Errors (Return Only)**

```cpp
// ✅ GOOD: Validation errors - return only, no logging
Result<void, Error> validateCommand(const AdmitPatientCommand& cmd) {
    if (cmd.mrn.isEmpty()) {
        return Error("MRN is required");  // ✅ Return, no log
    }
    if (cmd.mrn.length() < 3) {
        return Error("MRN too short");  // ✅ Return, no log
    }
    return Success();
}
```

#### **Pattern 2: Infrastructure Failures (Log + Return)**

```cpp
// ✅ GOOD: Infrastructure failures - log AND return
Result<void, Error> saveToDatabase(const Patient& patient) {
    if (!m_database.isOpen()) {
        m_logService->error("Database not open", {
            {"operation", "saveToDatabase"}
        });
        return Error("Database unavailable");  // ✅ Log + return
    }
    
    // ... save operation
}
```

#### **Pattern 3: Security Events (Log to Audit + Return)**

```cpp
// ✅ GOOD: Security events - log to audit log AND return
Result<UserProfile, Error> authenticate(const QString& userId, const QString& pin) {
    auto result = m_userMgmtService->authenticate(userId, pin);
    
    if (result.isError()) {
        // ✅ Log to security audit log (required)
        AuditEntry entry;
        entry.eventType = "LOGIN_FAILED";
        entry.userId = userId;
        m_auditRepo->logAuditEvent(entry);
        
        // ✅ Also log to application logs
        m_logService->warning("Authentication failed", {
            {"userId", userId}
        });
        
        // ✅ Return error
        return result;
    }
    
    return result;
}
```

#### **Pattern 4: Unexpected Conditions (Log Only, Don't Return)**

```cpp
// ✅ GOOD: Unexpected conditions - log only, skip operation
void processVitalSign(const VitalRecord& vital) {
    if (vital.timestamp.isNull()) {
        // ✅ Log unexpected condition
        m_logService->warning("Received vital sign with null timestamp", {
            {"patientMrn", vital.patientMrn}
        });
        return;  // ✅ Skip, don't return error (no caller to handle)
    }
    
    // ... process valid vital sign
}
```

### 4.8. Anti-Patterns

#### **❌ BAD: Logging validation errors**

```cpp
// ❌ BAD: Don't log validation errors
Result<void, Error> admitPatient(const AdmitPatientCommand& cmd) {
    if (cmd.mrn.isEmpty()) {
        m_logService->error("MRN is empty");  // ❌ WRONG: Validation error, don't log
        return Error("MRN is required");
    }
}
```

#### **❌ BAD: Not logging infrastructure failures**

```cpp
// ❌ BAD: Must log infrastructure failures
Result<void, Error> saveToDatabase(const Patient& patient) {
    if (!m_database.isOpen()) {
        return Error("Database unavailable");  // ❌ WRONG: Should log system failure
    }
}
```

#### **❌ BAD: Logging expected business conditions**

```cpp
// ❌ BAD: Don't log expected business conditions
Result<PatientIdentity, Error> findByMrn(const QString& mrn) {
    auto patient = queryPatient(mrn);
    if (!patient) {
        m_logService->warning("Patient not found");  // ❌ WRONG: Expected condition, don't log
        return Error("Patient not found");
    }
}
```

### 4.9. Summary Rules

**Return errors (don't log) when:**
- ✅ Input validation failures
- ✅ Business rule violations
- ✅ Expected conditions (patient not found, already admitted)
- ✅ User-correctable errors (invalid format, missing field)

**Log errors (and optionally return) when:**
- ✅ Infrastructure failures (database, network, file I/O)
- ✅ System-level errors (out of memory, disk full)
- ✅ Security events (authentication failures, unauthorized access)
- ✅ Unexpected conditions (null timestamps, corrupted state)
- ✅ Errors that need audit trail (all security events)

**Both return AND log when:**
- ✅ System failures that caller should handle (network timeout, database error)
- ✅ Security events (authentication failures, certificate validation)
- ✅ Recoverable errors that need monitoring (retry-able failures)

## 4.10. Return vs. Log vs. Emit: Complete Comparison

### 4.10.1. Three Error Handling Mechanisms

The Z Monitor application uses three distinct mechanisms for error handling:

| Mechanism | Syntax | When to Use | Caller Behavior |
|-----------|--------|-------------|-----------------|
| **Return Error** | `Result<T, E>` | Synchronous operations, immediate handling needed | Caller checks return value |
| **Log Error** | `m_logService->error(...)` | System failures, debugging, audit trail | No caller handling (fire-and-forget) |
| **Emit Error** | `emit errorOccurred(...)` | Asynchronous operations, cross-thread, UI feedback | Caller connects signal handler |

### 4.10.2. When to Return Errors

**Use `Result<T, E>` when:**
- ✅ **Synchronous operation** (caller waits for result)
- ✅ **Immediate handling needed** (caller must decide next action)
- ✅ **Same thread** (caller and callee on same thread)
- ✅ **Validation/business logic** (input validation, business rules)
- ✅ **Caller needs error details** (for retry, alternative path, user feedback)

**Example:**
```cpp
// ✅ GOOD: Return error for synchronous validation
Result<void, Error> AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    if (cmd.mrn.isEmpty()) {
        return Error("MRN is required");  // ✅ Return - caller handles immediately
    }
    
    auto patientResult = m_patientRepo->findByMrn(cmd.mrn);
    if (patientResult.isError()) {
        return patientResult;  // ✅ Return - caller can retry or show error
    }
    
    // ... proceed with admission
    return Success();
}

// Caller usage:
auto result = admissionService->admitPatient(cmd);
if (result.isError()) {
    showErrorDialog(result.error().message());  // ✅ Immediate handling
    return;
}
// Continue with success path
```

**Advantages:**
- ✅ **Type-safe:** Compiler enforces error checking
- ✅ **Immediate:** Caller handles error synchronously
- ✅ **Explicit:** Error handling is visible in code flow
- ✅ **Composable:** Can chain operations with `andThen()`, `map()`, etc.

**Disadvantages:**
- ❌ **Blocking:** Caller must wait for operation to complete
- ❌ **Same thread only:** Cannot use across threads easily
- ❌ **No UI feedback:** Cannot update UI during operation

### 4.10.3. When to Emit Errors (Signals)

**Use `emit errorOccurred(...)` when:**
- ✅ **Asynchronous operation** (operation continues in background)
- ✅ **Cross-thread communication** (worker thread → UI thread)
- ✅ **Long-running operation** (don't block caller)
- ✅ **Multiple listeners** (UI, logging, monitoring all need error)
- ✅ **Fire-and-forget** (caller doesn't need immediate response)
- ✅ **UI feedback needed** (show error dialog, update status)

**Example:**
```cpp
// ✅ GOOD: Emit error for asynchronous operation
class NetworkManager : public QObject {
    Q_OBJECT
public:
    void sendTelemetryAsync(const TelemetryBatch& batch) {
        // Start async operation (non-blocking)
        QFuture<Result<ServerResponse, Error>> future = 
            QtConcurrent::run([this, batch]() {
                return m_telemetryServer->sendBatch(batch);
            });
        
        // Monitor future completion
        auto watcher = new QFutureWatcher<Result<ServerResponse, Error>>(this);
        connect(watcher, &QFutureWatcher<Result<ServerResponse, Error>>::finished,
                this, [this, batch, watcher]() {
            auto result = watcher->result();
            if (result.isError()) {
                // ✅ Emit error signal (multiple listeners can handle)
                emit telemetrySendFailed(batch, result.error());
            } else {
                emit telemetrySent(batch, result.value());
            }
            watcher->deleteLater();
        });
        watcher->setFuture(future);
    }
    
signals:
    void telemetrySent(const TelemetryBatch& batch, const ServerResponse& response);
    void telemetrySendFailed(const TelemetryBatch& batch, const Error& error);
};

// Caller usage (non-blocking):
networkManager->sendTelemetryAsync(batch);  // ✅ Returns immediately
// ... continue with other work

// Connect signal handler (can be multiple handlers)
connect(networkManager, &NetworkManager::telemetrySendFailed,
        this, [](const TelemetryBatch& batch, const Error& error) {
    // ✅ Handle error asynchronously
    showErrorNotification(error.message());
    logErrorToDatabase(error);
});
```

**Advantages:**
- ✅ **Non-blocking:** Caller continues immediately
- ✅ **Cross-thread:** Works across thread boundaries (Qt::QueuedConnection)
- ✅ **Multiple listeners:** Multiple components can handle same error
- ✅ **UI-friendly:** Perfect for Qt/QML integration
- ✅ **Decoupled:** Caller doesn't need to know about error handling

**Disadvantages:**
- ❌ **No compile-time checking:** Easy to forget to connect signal
- ❌ **Less explicit:** Error handling is separated from call site
- ❌ **Harder to test:** Must use signal spy or mock connections
- ❌ **Potential memory leaks:** If signal handler not connected, error is lost

### 4.10.4. When to Log Errors

**Use `m_logService->error(...)` when:**
- ✅ **System failures** (infrastructure problems, unexpected conditions)
- ✅ **Debugging/auditing** (need record for troubleshooting)
- ✅ **Monitoring** (track error rates, patterns)
- ✅ **Security events** (authentication failures, unauthorized access)
- ✅ **Fire-and-forget** (no caller to handle, but need audit trail)

**Example:**
```cpp
// ✅ GOOD: Log infrastructure failures
Result<void, Error> DatabaseManager::savePatient(const Patient& patient) {
    if (!m_database.isOpen()) {
        // ✅ Log system failure (no caller to handle, but need audit trail)
        m_logService->error("Database not open", {
            {"operation", "savePatient"},
            {"patientMrn", patient.mrn}
        });
        return Error("Database unavailable");
    }
    
    // ... save operation
}

// ✅ GOOD: Log unexpected conditions (no caller to handle)
void MonitoringService::processVitalSign(const VitalRecord& vital) {
    if (vital.timestamp.isNull()) {
        // ✅ Log unexpected condition (fire-and-forget)
        m_logService->warning("Received vital sign with null timestamp", {
            {"patientMrn", vital.patientMrn}
        });
        return;  // Skip invalid data
    }
    
    // ... process valid vital sign
}
```

**Advantages:**
- ✅ **Audit trail:** Permanent record of errors
- ✅ **Debugging:** Helps troubleshoot issues
- ✅ **Monitoring:** Track error rates and patterns
- ✅ **Fire-and-forget:** No caller needed

**Disadvantages:**
- ❌ **No caller handling:** Caller cannot react to error
- ❌ **No user feedback:** User doesn't see error (unless also emitted)
- ❌ **Performance:** Logging has overhead (though async)

### 4.10.5. Decision Matrix: Return vs. Emit vs. Log

| Scenario | Return Error? | Emit Error? | Log Error? | Rationale |
|----------|---------------|-------------|------------|-----------|
| **Synchronous validation** (invalid MRN format) | ✅ Yes | ❌ No | ❌ No | Caller handles immediately |
| **Synchronous business rule** (patient already admitted) | ✅ Yes | ❌ No | ❌ No | Caller handles immediately |
| **Asynchronous network operation** (telemetry send) | ❌ No | ✅ Yes | ✅ Yes | Non-blocking, multiple listeners, needs audit |
| **Cross-thread database operation** (save from worker thread) | ❌ No | ✅ Yes | ✅ Yes | Cross-thread, UI feedback, needs audit |
| **Long-running operation** (patient lookup) | ❌ No | ✅ Yes | ✅ Yes | Non-blocking, UI feedback, needs audit |
| **Infrastructure failure** (database connection lost) | ✅ Yes | ✅ Yes | ✅ Yes | Caller handles, UI feedback, needs audit |
| **Unexpected condition** (null timestamp) | ❌ No | ❌ No | ✅ Yes | Fire-and-forget, debugging only |
| **Security event** (authentication failure) | ✅ Yes | ✅ Yes | ✅ Yes | Caller handles, UI feedback, audit required |
| **Validation error** (empty MRN) | ✅ Yes | ❌ No | ❌ No | Expected condition, caller handles |

### 4.10.6. Combined Patterns: Return + Emit + Log

**Common Pattern: All Three Together**

For critical operations that need immediate handling, UI feedback, and audit trail:

```cpp
// ✅ GOOD: Return + Emit + Log for critical operations
class NetworkManager : public QObject {
    Q_OBJECT
public:
    Result<ServerResponse, Error> sendTelemetry(const TelemetryBatch& batch) {
        auto result = m_telemetryServer->sendBatch(batch);
        
        if (result.isError()) {
            // ✅ 1. Log for audit trail
            m_logService->error("Telemetry send failed", {
                {"deviceId", batch.deviceId},
                {"patientMrn", batch.patientMrn},
                {"errorCode", QString::number(result.error().code())}
            });
            
            // ✅ 2. Emit for UI feedback (multiple listeners)
            emit telemetrySendFailed(batch, result.error());
            
            // ✅ 3. Return for immediate handling (caller can retry)
            return result;
        }
        
        // Success path
        emit telemetrySent(batch, result.value());
        return result;
    }
    
signals:
    void telemetrySent(const TelemetryBatch& batch, const ServerResponse& response);
    void telemetrySendFailed(const TelemetryBatch& batch, const Error& error);
};

// Caller can use either return value OR signal:
auto result = networkManager->sendTelemetry(batch);
if (result.isError()) {
    // ✅ Handle via return value (immediate)
    retryOrShowError(result.error());
}

// OR connect signal handler (asynchronous)
connect(networkManager, &NetworkManager::telemetrySendFailed,
        this, &MainWindow::handleTelemetryError);
```

### 4.10.7. Why Emit Instead of Return? (Programming Complexity)

**Question:** Why don't we always return errors? Wouldn't that be simpler?

**Answer:** **It depends on the operation type.** For synchronous operations, returning is simpler. For asynchronous operations, emitting is necessary.

#### **Synchronous Operations: Return is Simpler**

```cpp
// ✅ SIMPLE: Synchronous operation - return error
Result<void, Error> validateInput(const QString& input) {
    if (input.isEmpty()) {
        return Error("Input is required");
    }
    return Success();
}

// Caller usage (straightforward):
auto result = validateInput(userInput);
if (result.isError()) {
    showError(result.error().message());
    return;
}
// Continue...
```

**Why this is simple:**
- ✅ Error handling is **explicit** at call site
- ✅ Compiler enforces error checking
- ✅ Easy to test (check return value)
- ✅ Clear control flow

#### **Asynchronous Operations: Emit is Necessary**

```cpp
// ✅ NECESSARY: Asynchronous operation - emit error
void sendTelemetryAsync(const TelemetryBatch& batch) {
    // Operation runs in background (non-blocking)
    QtConcurrent::run([this, batch]() {
        auto result = m_telemetryServer->sendBatch(batch);
        if (result.isError()) {
            emit telemetrySendFailed(batch, result.error());  // ✅ Must emit
        }
    });
}

// Caller usage (non-blocking):
sendTelemetryAsync(batch);  // Returns immediately
// ... continue with other work

// Must connect signal handler (separate from call site)
connect(networkManager, &NetworkManager::telemetrySendFailed,
        this, &MainWindow::handleError);
```

**Why emit is necessary:**
- ✅ **Operation is non-blocking:** Caller doesn't wait for result
- ✅ **Result arrives later:** Cannot return immediately
- ✅ **Cross-thread:** Worker thread cannot return to UI thread directly
- ✅ **Multiple listeners:** UI, logging, monitoring all need error

**Why this seems complex:**
- ❌ Error handling is **separated** from call site
- ❌ Must remember to connect signal handler
- ❌ Harder to test (need signal spy)
- ❌ Less explicit control flow

**But it's necessary because:**
- ✅ **Non-blocking:** UI stays responsive
- ✅ **Cross-thread:** Works across thread boundaries
- ✅ **Multiple listeners:** Decoupled error handling

### 4.10.8. Best Practice: Hybrid Approach

**Recommended Pattern:** Use return for synchronous, emit for asynchronous, log for audit.

```cpp
class AdmissionService : public QObject {
    Q_OBJECT
public:
    // ✅ Synchronous validation - return error
    Result<void, Error> validateCommand(const AdmitPatientCommand& cmd) {
        if (cmd.mrn.isEmpty()) {
            return Error("MRN is required");  // ✅ Return - immediate handling
        }
        return Success();
    }
    
    // ✅ Asynchronous operation - emit error
    void admitPatientAsync(const AdmitPatientCommand& cmd) {
        // Validate first (synchronous)
        auto validationResult = validateCommand(cmd);
        if (validationResult.isError()) {
            emit admissionFailed(cmd, validationResult.error());  // ✅ Emit for async
            return;
        }
        
        // Start async operation
        QtConcurrent::run([this, cmd]() {
            auto result = performAdmission(cmd);
            if (result.isError()) {
                // ✅ Log for audit
                m_logService->error("Admission failed", {
                    {"mrn", cmd.mrn},
                    {"error", result.error().message()}
                });
                
                // ✅ Emit for UI feedback
                emit admissionFailed(cmd, result.error());
            } else {
                emit patientAdmitted(cmd, result.value());
            }
        });
    }
    
signals:
    void patientAdmitted(const AdmitPatientCommand& cmd, const Patient& patient);
    void admissionFailed(const AdmitPatientCommand& cmd, const Error& error);
};
```

### 4.10.9. Guidelines by Operation Type

#### **Synchronous Operations → Return Error**

```cpp
// ✅ GOOD: Synchronous - return error
Result<PatientIdentity, Error> findPatient(const QString& mrn) {
    // ... synchronous lookup
    if (patientNotFound) {
        return Error("Patient not found");  // ✅ Return
    }
    return patient;
}

// Usage:
auto result = findPatient(mrn);
if (result.isError()) {
    handleError(result.error());  // ✅ Immediate handling
}
```

#### **Asynchronous Operations → Emit Error**

```cpp
// ✅ GOOD: Asynchronous - emit error
void lookupPatientAsync(const QString& mrn) {
    QtConcurrent::run([this, mrn]() {
        auto result = findPatient(mrn);
        if (result.isError()) {
            emit patientLookupFailed(mrn, result.error());  // ✅ Emit
        } else {
            emit patientFound(mrn, result.value());
        }
    });
}

// Usage:
lookupPatientAsync(mrn);  // Returns immediately
connect(service, &Service::patientLookupFailed,
        this, &Handler::onLookupFailed);  // ✅ Connect handler
```

#### **Infrastructure Failures → Log + Return/Emit**

```cpp
// ✅ GOOD: Infrastructure failure - log + return/emit
Result<void, Error> saveToDatabase(const Patient& patient) {
    if (!m_database.isOpen()) {
        // ✅ Log for audit
        m_logService->error("Database not open", {...});
        
        // ✅ Return for synchronous, OR emit for asynchronous
        return Error("Database unavailable");
    }
    // ... save operation
}
```

### 4.10.10. Summary: Return vs. Emit vs. Log

**Return Error (`Result<T, E>`) when:**
- ✅ Synchronous operation
- ✅ Caller needs immediate error handling
- ✅ Same thread
- ✅ Validation/business logic

**Emit Error (`emit errorOccurred(...)`) when:**
- ✅ Asynchronous operation
- ✅ Cross-thread communication
- ✅ Long-running operation
- ✅ Multiple listeners needed
- ✅ UI feedback required

**Log Error (`m_logService->error(...)`) when:**
- ✅ System failures (infrastructure problems)
- ✅ Unexpected conditions
- ✅ Security events (audit trail)
- ✅ Debugging/monitoring needs
- ✅ Fire-and-forget (no caller to handle)

**Use All Three when:**
- ✅ Critical operations (immediate handling + UI feedback + audit trail)
- ✅ Infrastructure failures (caller handles + UI shows + audit logs)
- ✅ Security events (caller handles + UI shows + audit logs)

**Key Insight:** The choice between return and emit is determined by **operation type** (synchronous vs. asynchronous), not by preference. Synchronous operations should return errors. Asynchronous operations must emit errors (cannot return because operation hasn't completed yet).

## 5. Error Logging

### 5.1. Log Levels

Use appropriate log levels for errors:

- **ERROR:** Recoverable errors, operation failures
- **WARNING:** Degraded functionality, retry attempts
- **CRITICAL:** System-threatening errors, security violations
- **FATAL:** Unrecoverable errors requiring restart

### 5.2. Error Context

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

### 5.3. Security Audit Logging

Security-related errors must be logged to `security_audit_log`:

- Authentication failures
- Certificate validation failures
- Network connection failures
- Data integrity violations
- Unauthorized access attempts

## 6. Error Recovery Patterns

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

## 7. Error Propagation in Qt/QML

### 7.1. C++ to QML Error Propagation

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

### 7.2. QML Error Handling

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

## 8. Error Codes Reference

### 8.1. Network Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 1001 | ConnectionTimeout | Connection timed out | Retry with backoff |
| 1002 | ConnectionRefused | Server refused connection | Check server status |
| 1003 | CertificateValidationFailed | Certificate invalid | Re-provision device |
| 1004 | TLSError | TLS handshake failed | Check certificates |
| 1005 | RateLimitExceeded | Too many requests | Wait and retry |

### 8.2. Database Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 2001 | DatabaseLocked | Database is locked | Retry after delay |
| 2002 | DatabaseCorrupted | Database corruption detected | Run recovery |
| 2003 | QueryFailed | SQL query failed | Check query syntax |
| 2004 | TransactionFailed | Transaction rollback | Retry transaction |

### 8.3. Authentication Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 3001 | InvalidCredentials | Invalid PIN | User re-enters PIN |
| 3002 | AccountLocked | Account locked (brute force) | Wait for lockout period |
| 3003 | SessionExpired | Session expired | Re-authenticate |
| 3004 | Unauthorized | Insufficient permissions | Check user role |

### 8.4. Patient Management Errors

| Code | Name | Description | Recovery |
|------|------|-------------|----------|
| 4001 | PatientNotFound | Patient not found in system | Verify MRN |
| 4002 | LookupServiceUnavailable | Patient lookup service down | Use cached data |
| 4003 | InvalidMRN | Invalid MRN format | User corrects input |
| 4004 | AdmissionFailed | Patient admission failed | Check patient status |

## 9. Error Handling Best Practices

### 9.1. Do's

- ✅ Always check return values and handle errors
- ✅ Provide clear, actionable error messages
- ✅ Log errors with full context
- ✅ Use appropriate error types (recoverable vs. non-recoverable)
- ✅ Implement retry logic for transient errors
- ✅ Emit signals for async error propagation
- ✅ Validate inputs early to prevent errors

### 9.2. Don'ts

- ❌ Don't ignore errors silently
- ❌ Don't use exceptions for expected error conditions
- ❌ Don't expose implementation details in user-facing errors
- ❌ Don't retry non-retryable errors indefinitely
- ❌ Don't catch exceptions without handling them
- ❌ Don't use raw error codes without type safety

## 10. Testing Error Handling

### 10.1. Unit Tests

- Test all error paths
- Verify error messages are clear
- Test retry logic
- Test circuit breaker behavior
- Verify error logging

### 10.2. Integration Tests

- Test error propagation across components
- Test recovery from transient errors
- Test graceful degradation
- Verify error signals are emitted correctly

### 10.3. Error Injection

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

## 11. Error Monitoring and Alerting

### 11.1. Error Metrics

Track error rates and patterns:

- Error rate per operation type
- Error rate over time
- Most common errors
- Recovery success rate
- Circuit breaker state changes

### 11.2. Alerting Thresholds

Alert on:

- Error rate exceeds threshold (e.g., > 5% of operations)
- Critical errors occur
- Circuit breaker opens
- Retry exhaustion
- Security-related errors

## 12. Related Documents

- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Error handling in multi-threaded context
- [06_SECURITY.md](./06_SECURITY.md) - Security error handling
- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Error handling tests
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Error handling in class designs

