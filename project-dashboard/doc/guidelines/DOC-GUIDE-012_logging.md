---
doc_id: DOC-GUIDE-012
title: Logging Strategy and Best Practices
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Guideline
tags: [logging, audit, action-log, structured-logging, async, thread-safety]
related:
  - DOC-GUIDE-011
  - DOC-ARCH-009
  - DOC-PROC-003
source: 21_LOGGING_STRATEGY.md
---

# Logging Strategy and Best Practices

## Overview

This guideline defines the logging strategy, logging types, and best practices for the Z Monitor application. The system uses **three distinct logging mechanisms** for different purposes: Application Logs (file-based), Action Logs (database), and Security Audit Logs (database).

## Core Principles

1. **Separation of Concerns:** Different logging types for different purposes
2. **Asynchronous Logging:** All logging is non-blocking (< 1μs call time)
3. **Structured Data:** Use key-value context for machine-readable logs
4. **Dependency Injection:** **NO global log objects** - always inject dependencies
5. **Thread-Safe:** Can be called from any thread safely

## Three Logging Types

### 1. Application Logs (LogService) - File-Based

**Purpose:** Debug, info, warning, error messages for troubleshooting and development.

**Storage:** Rotated text/JSON files in `logs/z-monitor.log.*` (7-day retention).

**Thread:** Dedicated background thread with lock-free queue.

**When to Use:**
- System status/health monitoring
- Error messages (network failures, database errors)
- Debug/trace messages for development
- Performance metrics

**Log Levels:**
- `TRACE` (0): Detailed execution traces (disabled in production)
- `DEBUG` (1): Debug information (disabled in production)
- `INFO` (2): Informational messages
- `WARN` (3): Warning messages (potential issues)
- `ERROR` (4): Error messages (operation failed, but app continues)
- `CRITICAL` (5): Critical errors (major failure, but not fatal)
- `FATAL` (6): Fatal errors (application cannot continue)

**Example:**
```cpp
// Dependency injection (NEVER use global singleton)
class NetworkManager {
public:
    NetworkManager(LogService* logService, QObject* parent = nullptr)
        : QObject(parent), m_logService(logService) {}
    
    void sendTelemetry(const TelemetryBatch& batch) {
        auto result = transmit(batch);
        if (result.isError()) {
            m_logService->error("Telemetry send failed", {
                {"patientMrn", batch.patientMrn},
                {"errorCode", QString::number(result.error().code())},
                {"retryAttempt", QString::number(m_retryCount)}
            });
        } else {
            m_logService->debug("Telemetry sent successfully", {
                {"patientMrn", batch.patientMrn},
                {"sampleCount", QString::number(batch.samples.size())}
            });
        }
    }
    
private:
    LogService* m_logService;  // ← Injected dependency
};
```

**Format (JSON):**
```json
{
  "timestamp": "2025-11-27T14:30:25.123Z",
  "level": "ERROR",
  "category": "NetworkManager",
  "message": "Telemetry send failed",
  "context": {
    "patientMrn": "MRN-12345",
    "errorCode": "1001",
    "retryAttempt": "2"
  },
  "threadId": "NetworkThread",
  "file": "NetworkManager.cpp",
  "line": 245
}
```

### 2. Action Logs (IActionLogRepository) - Database-Based

**Purpose:** User actions for audit trail, compliance, and accountability.

**Storage:** `action_log` table in SQLite database (encrypted with SQLCipher).

**Thread:** Database I/O Thread (asynchronous).

**When to Use:**
- Login, logout, auto-logout events
- Patient management (admit, discharge, transfer)
- Configuration changes (settings, thresholds)
- Notification clearing
- Any user-initiated action requiring audit trail

**Note:** Action logs do **NOT** use log levels. They use `result` field (SUCCESS, FAILURE, PARTIAL).

**Example:**
```cpp
// Dependency injection (NEVER use global singleton)
class AdmissionService : public QObject {
public:
    AdmissionService(
        IPatientRepository* patientRepo,
        IActionLogRepository* actionLogRepo,  // ← Injected dependency
        QObject* parent = nullptr)
        : QObject(parent),
          m_patientRepo(patientRepo),
          m_actionLogRepo(actionLogRepo) {}
    
    Result<void, Error> admitPatient(const AdmitPatientCommand& cmd) {
        // Perform admission
        auto result = m_patientRepo->admitPatient(cmd.mrn, cmd.identity);
        
        // Log action via injected repository
        ActionLogEntry entry;
        entry.userId = cmd.userId;
        entry.actionType = "ADMIT_PATIENT";
        entry.targetType = "PATIENT";
        entry.targetId = cmd.mrn;
        entry.details = QJsonObject{
            {"bedNumber", cmd.bedNumber},
            {"wardId", cmd.wardId}
        };
        entry.result = result.isOk() ? "SUCCESS" : "FAILURE";
        if (result.isError()) {
            entry.errorCode = QString::number(result.error().code());
            entry.errorMessage = result.error().message();
        }
        m_actionLogRepo->logAction(entry);  // ← Use injected dependency
        
        return result;
    }
    
private:
    IPatientRepository* m_patientRepo;
    IActionLogRepository* m_actionLogRepo;  // ← Store injected dependency
};
```

**Database Schema:**
```sql
CREATE TABLE action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,
    timestamp_iso TEXT NOT NULL,
    user_id TEXT NULL,
    user_role TEXT NULL,
    action_type TEXT NOT NULL,
    target_type TEXT NULL,
    target_id TEXT NULL,
    details TEXT NULL,              -- JSON string
    result TEXT NOT NULL,           -- SUCCESS, FAILURE, PARTIAL
    error_code TEXT NULL,
    error_message TEXT NULL,
    device_id TEXT NOT NULL,
    session_token_hash TEXT NULL,
    ip_address TEXT NULL,
    previous_hash TEXT NULL         -- Hash chain for tamper detection
);
```

### 3. Security Audit Logs (IAuditRepository) - Database-Based

**Purpose:** Security events for forensics, compliance, and security incident investigation.

**Storage:** `security_audit_log` table in SQLite database (encrypted with SQLCipher).

**Thread:** Database I/O Thread (asynchronous).

**When to Use:**
- Authentication failures (LOGIN_FAILED, INVALID_CREDENTIALS)
- Session events (SESSION_EXPIRED, SESSION_REVOKED)
- Certificate operations (CERTIFICATE_INSTALLED, CERTIFICATE_REVOKED)
- Network security events (CONNECTION_REJECTED, CERTIFICATE_PINNING_FAILED)
- Security violations (UNAUTHORIZED_ACCESS, PERMISSION_DENIED)

**Note:** Security audit logs use `severity` field (INFO, WARNING, ERROR, CRITICAL), not log levels.

**Example:**
```cpp
// Dependency injection (NEVER use global singleton)
class SecurityService : public QObject {
public:
    SecurityService(
        IUserManagementService* userMgmtService,
        IActionLogRepository* actionLogRepo,
        IAuditRepository* auditRepo,  // ← Injected dependency
        QObject* parent = nullptr)
        : QObject(parent),
          m_userMgmtService(userMgmtService),
          m_actionLogRepo(actionLogRepo),
          m_auditRepo(auditRepo) {}
    
    Result<SessionToken, Error> login(const QString& userId, const QString& secretCode) {
        auto result = m_userMgmtService->authenticate(userId, secretCode);
        
        if (result.isError()) {
            // Log security audit event for failed login
            AuditEntry entry;
            entry.eventType = "LOGIN_FAILED";
            entry.eventCategory = "AUTHENTICATION";
            entry.userId = userId;
            entry.deviceId = getDeviceLabel();
            entry.severity = "WARNING";
            entry.success = false;
            entry.details = QJsonObject{
                {"reason", "INVALID_CREDENTIALS"},
                {"remainingAttempts", QString::number(getRemainingAttempts(userId))}
            };
            entry.errorCode = "AUTH_001";
            entry.errorMessage = "Invalid user ID or secret code";
            m_auditRepo->logAuditEvent(entry);  // ← Use injected dependency
            
            return result.error();
        }
        
        // Login succeeded, log action
        ActionLogEntry actionEntry;
        actionEntry.userId = userId;
        actionEntry.actionType = "LOGIN";
        actionEntry.result = "SUCCESS";
        m_actionLogRepo->logAction(actionEntry);
        
        return result.value();
    }
    
private:
    IUserManagementService* m_userMgmtService;
    IActionLogRepository* m_actionLogRepo;
    IAuditRepository* m_auditRepo;  // ← Store injected dependency
};
```

**Database Schema:**
```sql
CREATE TABLE security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    severity TEXT NOT NULL,         -- INFO, WARNING, ERROR, CRITICAL
    device_id TEXT NULL,
    user_id TEXT NULL,
    source_ip TEXT NULL,
    event_category TEXT NOT NULL,   -- AUTHENTICATION, CERTIFICATE, NETWORK, etc.
    success BOOLEAN NOT NULL,
    details TEXT NULL,              -- JSON string
    error_code TEXT NULL,
    error_message TEXT NULL,
    previous_hash TEXT NULL         -- Hash chain for tamper detection
);
```

## Decision Guide: Which Logging Mechanism to Use?

### Quick Decision Tree

```
What are you logging?
│
├─ Is it a USER ACTION?
│  │
│  ├─ Login, logout, auto-logout?
│  │  └─ Use IActionLogRepository → action_log table
│  │
│  ├─ Patient management (admit/discharge/transfer)?
│  │  └─ Use IActionLogRepository → action_log table
│  │
│  ├─ Configuration change (settings, thresholds)?
│  │  └─ Use IActionLogRepository → action_log table
│  │
│  └─ Notification clearing?
│     └─ Use IActionLogRepository → action_log table
│
├─ Is it a SECURITY EVENT?
│  │
│  ├─ Authentication failure?
│  │  └─ Use IAuditRepository → security_audit_log table
│  │
│  ├─ Certificate operation?
│  │  └─ Use IAuditRepository → security_audit_log table
│  │
│  ├─ Network security event?
│  │  └─ Use IAuditRepository → security_audit_log table
│  │
│  └─ Security violation?
│     └─ Use IAuditRepository → security_audit_log table
│
└─ Is it a SYSTEM EVENT?
   │
   ├─ Error, warning, info message?
   │  └─ Use LogService → log files
   │
   ├─ Debug/trace message?
   │  └─ Use LogService → log files
   │
   └─ System status/health?
      └─ Use LogService → log files
```

### Examples by Scenario

**Scenario 1: User logs in**
```cpp
// ✅ CORRECT: Use IActionLogRepository (user action)
ActionLogEntry entry;
entry.actionType = "LOGIN";
entry.userId = "NURSE001";
entry.result = "SUCCESS";
m_actionLogRepo->logAction(entry);

// ❌ WRONG: Don't use LogService for user actions
// m_logService->info("User logged in");  // Wrong!
```

**Scenario 2: Login attempt fails (security event)**
```cpp
// ✅ CORRECT: Use IAuditRepository (security event)
AuditEntry entry;
entry.eventType = "LOGIN_FAILED";
entry.eventCategory = "AUTHENTICATION";
entry.userId = "NURSE001";
entry.success = false;
m_auditRepo->logAuditEvent(entry);

// Also log to application logs for troubleshooting
m_logService->warning("Login failed", {
    {"userId", "NURSE001"},
    {"reason", "INVALID_CREDENTIALS"}
});
```

**Scenario 3: Network error**
```cpp
// ✅ CORRECT: Use LogService (system event)
m_logService->error("Telemetry send failed", {
    {"deviceId", deviceId},
    {"errorCode", QString::number(error.code())},
    {"retryAttempt", QString::number(attempt)}
});

// ❌ WRONG: Don't use action/audit logs for system errors
// m_actionLogRepo->logAction(...);  // Wrong!
```

**Scenario 4: Patient admitted**
```cpp
// ✅ CORRECT: Use IActionLogRepository (user action)
ActionLogEntry entry;
entry.actionType = "ADMIT_PATIENT";
entry.userId = "NURSE001";
entry.targetType = "PATIENT";
entry.targetId = patientMrn;
entry.result = "SUCCESS";
m_actionLogRepo->logAction(entry);

// Also log to application logs for troubleshooting
m_logService->info("Patient admitted", {
    {"patientMrn", patientMrn},
    {"userId", "NURSE001"}
});
```

## Dependency Injection Pattern (Critical)

**CRITICAL:** All services must use dependency injection for logging. **NO global log objects.**

### Problem with Global Singletons

```cpp
// ❌ BAD: Global singleton (hard to test, tight coupling)
void MyService::doSomething() {
    LogService::instance()->info("Something happened");
    GlobalActionLog::instance()->logAction(...);
}
```

**Problems:**
- Hard to test (can't inject mocks)
- Tight coupling to concrete implementations
- Hidden dependencies (not visible in constructor)
- Global state

### Solution: Dependency Injection

```cpp
// ✅ GOOD: Dependency injection (testable, loosely coupled)
class MyService {
public:
    MyService(
        LogService* logService,           // ← Injected dependency
        IActionLogRepository* actionLog,  // ← Injected dependency
        IAuditRepository* auditLog,       // ← Injected dependency
        QObject* parent = nullptr)
        : QObject(parent),
          m_logService(logService),
          m_actionLog(actionLog),
          m_auditLog(auditLog) {}
    
    void doSomething() {
        m_logService->info("Something happened");  // ← Use injected dependency
        
        ActionLogEntry entry;
        entry.actionType = "DO_SOMETHING";
        m_actionLog->logAction(entry);  // ← Use injected dependency
    }
    
private:
    LogService* m_logService;           // ← Store injected dependency
    IActionLogRepository* m_actionLog;
    IAuditRepository* m_auditLog;
};
```

**Benefits:**
- ✅ **Testable:** Can inject mock repositories in unit tests
- ✅ **Flexible:** Can swap implementations (SQLite, in-memory, remote)
- ✅ **Explicit Dependencies:** Constructor shows all dependencies
- ✅ **No Hidden Coupling:** No global state

## Structured Logging Best Practices

### Use Key-Value Context

```cpp
// ✅ GOOD: Structured context
m_logService->error("Telemetry send failed", {
    {"patientMrn", batch.patientMrn},
    {"errorCode", QString::number(error.code())},
    {"retryAttempt", QString::number(retryCount)},
    {"serverUrl", serverUrl}
});

// ❌ BAD: String concatenation (not machine-readable)
m_logService->error(QString("Telemetry send failed for patient %1, error %2, attempt %3")
    .arg(batch.patientMrn)
    .arg(error.code())
    .arg(retryCount));
```

### Provide Sufficient Context

```cpp
// ✅ GOOD: Includes all relevant context
m_logService->error("Database query failed", {
    {"query", "SELECT * FROM patients WHERE mrn = ?"},
    {"mrn", patientMrn},
    {"errorCode", QString::number(dbError.number())},
    {"errorMessage", dbError.text()},
    {"databasePath", m_database.databaseName()}
});

// ❌ BAD: Missing context
m_logService->error("Database error");
```

### Use Appropriate Log Levels

```cpp
// ✅ GOOD: Correct level for severity
m_logService->trace("Entering function processVitals");  // TRACE: execution trace
m_logService->debug("Cache hit for patient {}", mrn);    // DEBUG: debug info
m_logService->info("Patient admitted: {}", mrn);         // INFO: normal operation
m_logService->warning("Cache miss, fetching from DB");   // WARN: potential issue
m_logService->error("Failed to save patient data");      // ERROR: operation failed
m_logService->critical("Database connection lost");      // CRITICAL: major failure
m_logService->fatal("Cannot open database, exiting");    // FATAL: app cannot continue

// ❌ BAD: Everything is ERROR
m_logService->error("Cache hit for patient {}", mrn);  // Should be DEBUG
m_logService->error("Patient admitted: {}", mrn);      // Should be INFO
```

## Asynchronous Logging Architecture

### Non-Blocking Calls

All logging methods return immediately (< 1μs):

```cpp
// All of these return immediately, no blocking
m_logService->info("Message");                    // < 1μs
m_actionLogRepo->logAction(entry);                // < 1μs
m_auditRepo->logAuditEvent(entry);                // < 1μs
```

### Lock-Free Queue Pattern

```cpp
class LogService {
private:
    // Lock-free queue for async logging
    std::unique_ptr<LockFreeQueue<LogEntry>> m_logQueue;
    QThread* m_logThread;  // Dedicated background thread
    
public:
    void info(const QString& message, const QVariantMap& context = {}) {
        LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = LogLevel::Info;
        entry.message = message;
        entry.context = context;
        
        // Enqueue without blocking (< 1μs)
        m_logQueue->enqueue(entry);
        
        // Background thread processes queue asynchronously
    }
};
```

## Log Rotation and Retention

### Application Logs (File-Based)

- **Rotation:** Daily at midnight or when file exceeds 100MB
- **Retention:** 7 days (configurable)
- **Location:** `logs/z-monitor.log.*`
- **Format:** JSON (production) or human-readable (development)

### Action Logs (Database)

- **Retention:** 90 days minimum (configurable)
- **Cleanup:** Automated background job (runs daily)
- **Archival:** Export to external storage before deletion (compliance requirement)

### Security Audit Logs (Database)

- **Retention:** 90 days minimum (required for compliance)
- **Cleanup:** Automated background job (runs daily)
- **Archival:** Export to external storage before deletion (compliance requirement)
- **Tamper Detection:** Hash chain prevents modification

## Related Documents

- [DOC-GUIDE-011: Error Handling Strategy](./DOC-GUIDE-011_error_handling.md) - When to log errors vs. return them
- [DOC-COMP-029: Async Logging Architecture](../components/DOC-COMP-029_async_logging.md) - Complete async logging implementation
- [DOC-PROC-005: Login Workflow](../processes/DOC-PROC-005_login_workflow_and_action_logging.md) - Example of action and audit logging

## Enforcement

Logging is enforced through:
- **Code Review:** All PRs must use dependency injection (no global singletons)
- **Architecture Tests:** Verify no global log objects
- **Unit Tests:** Mock logging repositories in tests
- **Static Analysis:** Check for proper log levels and structured context
