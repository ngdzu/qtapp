# Logging Strategy

**Document ID:** DESIGN-021  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the logging architecture, log levels, log formats, log rotation, and log management strategy for the Z Monitor application.

## 1. Overview: Three Types of Logging

Z Monitor uses **three distinct logging mechanisms** for different purposes:

| Logging Type | Purpose | Storage Medium | Format | Retention | When to Use |
|--------------|---------|----------------|--------|-----------|-------------|
| **Application Logs** (`LogService`) | Debug/info/error messages for troubleshooting | **File** (rotated text/JSON files) | Human-readable or JSON | 7 days | System events, errors, warnings, debug info |
| **Action Logs** (`IActionLogRepository`) | User actions for audit/compliance | **Database** (`action_log` table) | Structured database rows | 90 days | Login, logout, config changes, patient management |
| **Security Audit Logs** (`IAuditRepository`) | Security events for forensics | **Database** (`security_audit_log` table) | Structured database rows | 90 days | Auth failures, cert operations, security violations |

**Key Decision Matrix:**

```
What are you logging?
├─ User action (login, logout, config change)?
│  └─ Use IActionLogRepository → action_log table (database)
│
├─ Security event (auth failure, cert operation)?
│  └─ Use IAuditRepository → security_audit_log table (database)
│
└─ System event (error, warning, debug message)?
   └─ Use LogService → log files (file system)
```

## 2. Guiding Principles

- **Structured Logging:** Use structured log entries with key-value pairs for easy parsing and analysis
- **Appropriate Log Levels:** Use correct log levels for application logs (not applicable to action/audit logs)
- **Performance:** Logging must not impact real-time performance (all logging is asynchronous)
- **Security:** Never log sensitive data (passwords, certificates, patient data)
- **Auditability:** Critical operations must be logged for compliance (use action/audit logs)
- **Rotation:** Implement log rotation for file-based logs to prevent disk space exhaustion
- **Dependency Injection:** All logging repositories injected via constructor (no global objects)

## 2. Detailed Logging Types

### 2.1. Application Logs (LogService) - File-Based

**Purpose:** System events, errors, warnings, debug information for troubleshooting and diagnostics.

**Storage:** Rotated text/JSON files in `logs/` directory.

**Format:** Human-readable (development) or JSON (production).

**When to Use:**
- System errors (network failures, database errors)
- Warnings (retries, degraded performance)
- Info messages (connection established, service started)
- Debug messages (function entry/exit, intermediate values)
- Trace messages (per-sample processing, queue operations)

**Example:**
```cpp
// In NetworkManager
m_logService->error("Telemetry send failed", {
    {"deviceId", deviceId},
    {"patientMrn", patientMrn},
    {"errorCode", QString::number(error.code())},
    {"retryAttempt", QString::number(attempt)}
});
```

**Output (JSON format):**
```json
{
  "timestamp": "2025-11-27T14:30:25.123Z",
  "level": "ERROR",
  "category": "NetworkManager",
  "message": "Telemetry send failed",
  "context": {
    "deviceId": "ZM-ICU-04",
    "patientMrn": "MRN-12345",
    "errorCode": 1001,
    "retryAttempt": 2
  },
  "threadId": "NetworkThread",
  "file": "NetworkManager.cpp",
  "line": 245,
  "function": "sendTelemetry"
}
```

**Log Levels (Application Logs Only):**
```
FATAL > CRITICAL > ERROR > WARNING > INFO > DEBUG > TRACE
```

| Level | When to Use | Example |
|-------|-------------|---------|
| **FATAL** | System cannot continue, must restart | Out of memory, disk full, critical corruption |
| **CRITICAL** | System-threatening error, requires immediate attention | Security violation, data integrity failure |
| **ERROR** | Operation failed, but system can continue | Network send failed, database query failed |
| **WARNING** | Unexpected condition, but operation succeeded | Retry occurred, degraded performance |
| **INFO** | Normal operation milestones | Connection established, service started |
| **DEBUG** | Detailed information for debugging | Function entry/exit, intermediate values |
| **TRACE** | Very detailed tracing (disabled in production) | Per-sample processing, queue operations |

**Production vs. Development:**
- **Production:** FATAL, CRITICAL, ERROR, WARNING, INFO
- **Development:** All levels including DEBUG
- **Debugging:** All levels including TRACE

### 2.2. Action Logs (IActionLogRepository) - Database-Based

**Purpose:** User actions for audit, compliance, and accountability. Tracks who did what and when.

**Storage:** `action_log` table in SQLite database (encrypted with SQLCipher).

**Format:** Structured database rows with JSON details field.

**When to Use:**
- User authentication actions (LOGIN, LOGOUT, AUTO_LOGOUT)
- Patient management actions (ADMIT_PATIENT, DISCHARGE_PATIENT, TRANSFER_PATIENT)
- Configuration actions (CHANGE_SETTING, ADJUST_ALARM_THRESHOLD)
- Notification actions (CLEAR_NOTIFICATIONS, DISMISS_NOTIFICATION)
- Administrative actions (VIEW_AUDIT_LOG, EXPORT_DATA, ACCESS_DIAGNOSTICS)

**Example:**
```cpp
// In SecurityService
ActionLogEntry entry;
entry.userId = "NURSE001";
entry.userRole = "NURSE";
entry.actionType = "LOGIN";
entry.targetType = nullptr;
entry.targetId = nullptr;
entry.details = QJsonObject{
    {"login_method", "secret_code"},
    {"device_id", getDeviceLabel()}
};
entry.result = "SUCCESS";
entry.sessionTokenHash = hashSessionToken(userProfile.sessionToken);
entry.deviceId = getDeviceLabel();

m_actionLogRepo->logAction(entry);
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
    result TEXT NOT NULL,
    error_code TEXT NULL,
    error_message TEXT NULL,
    device_id TEXT NOT NULL,
    session_token_hash TEXT NULL,
    ip_address TEXT NULL,
    previous_hash TEXT NULL         -- Hash chain for tamper detection
);
```

**Stored Data Example:**
```json
{
  "timestamp_ms": 1632500000123,
  "timestamp_iso": "2025-11-27T14:30:00.123Z",
  "user_id": "NURSE001",
  "user_role": "NURSE",
  "action_type": "LOGIN",
  "target_type": null,
  "target_id": null,
  "details": "{\"login_method\":\"secret_code\",\"device_id\":\"ZM-ICU-04\"}",
  "result": "SUCCESS",
  "error_code": null,
  "error_message": null,
  "device_id": "ZM-ICU-04",
  "session_token_hash": "a1b2c3d4e5f6...",
  "ip_address": null,
  "previous_hash": "previous-entry-hash"
}
```

**Note:** Action logs do NOT use log levels. They use `result` field (SUCCESS, FAILURE, PARTIAL).

### 2.3. Security Audit Logs (IAuditRepository) - Database-Based

**Purpose:** Security events for forensics, compliance, and security incident investigation.

**Storage:** `security_audit_log` table in SQLite database (encrypted with SQLCipher).

**Format:** Structured database rows with JSON details field.

**When to Use:**
- Authentication failures (LOGIN_FAILED, INVALID_CREDENTIALS)
- Session events (SESSION_EXPIRED, SESSION_REVOKED)
- Certificate operations (CERTIFICATE_INSTALLED, CERTIFICATE_REVOKED, CERTIFICATE_VALIDATION_FAILED)
- Network security events (CONNECTION_REJECTED, CERTIFICATE_PINNING_FAILED)
- Security violations (UNAUTHORIZED_ACCESS, PERMISSION_DENIED)

**Example:**
```cpp
// In SecurityService
AuditEntry entry;
entry.eventType = "LOGIN_FAILED";
entry.eventCategory = "AUTHENTICATION";
entry.userId = "NURSE001";
entry.deviceId = getDeviceLabel();
entry.severity = "WARNING";
entry.success = false;
entry.details = QJsonObject{
    {"reason", "INVALID_CREDENTIALS"},
    {"remainingAttempts", 2}
};
entry.errorCode = "AUTH_001";
entry.errorMessage = "Invalid user ID or secret code";

m_auditRepo->logAuditEvent(entry);
```

**Database Schema:**
```sql
CREATE TABLE security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    severity TEXT NOT NULL,
    device_id TEXT NULL,
    user_id TEXT NULL,
    source_ip TEXT NULL,
    event_category TEXT NOT NULL,
    success BOOLEAN NOT NULL,
    details TEXT NULL,              -- JSON string
    error_code TEXT NULL,
    error_message TEXT NULL,
    previous_hash TEXT NULL         -- Hash chain for tamper detection
);
```

**Stored Data Example:**
```json
{
  "timestamp": 1632500000123,
  "event_type": "LOGIN_FAILED",
  "severity": "WARNING",
  "device_id": "ZM-ICU-04",
  "user_id": "NURSE001",
  "source_ip": null,
  "event_category": "AUTHENTICATION",
  "success": false,
  "details": "{\"reason\":\"INVALID_CREDENTIALS\",\"remainingAttempts\":2}",
  "error_code": "AUTH_001",
  "error_message": "Invalid user ID or secret code",
  "previous_hash": "previous-entry-hash"
}
```

**Note:** Security audit logs use `severity` field (INFO, WARNING, ERROR, CRITICAL), not log levels.

## 3. Logging Architecture

### 3.1. LogService Design

**Location:** `src/infrastructure/logging/LogService.h/cpp`

**Thread:** Database I/O Thread (shared with database operations)

**Key Characteristics:**
- **Non-Blocking:** All methods return immediately (< 1μs)
- **Async Buffering:** Log entries are buffered in a lock-free queue
- **Library Abstraction:** Uses `ILogBackend` interface for easy library switching
- **Thread-Safe:** Can be called from any thread without synchronization

**See:** [43_ASYNC_LOGGING_ARCHITECTURE.md](./43_ASYNC_LOGGING_ARCHITECTURE.md) for complete async logging architecture and implementation details.

```cpp
class LogService : public QObject {
    Q_OBJECT
    
public:
    enum LogLevel {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5,
        Fatal = 6
    };
    
    explicit LogService(ILogBackend* backend, QObject* parent = nullptr);
    
    // All methods return immediately (< 1μs) - non-blocking
    void trace(const QString& message, const QVariantMap& context = {});
    void debug(const QString& message, const QVariantMap& context = {});
    void info(const QString& message, const QVariantMap& context = {});
    void warning(const QString& message, const QVariantMap& context = {});
    void error(const QString& message, const QVariantMap& context = {});
    void critical(const QString& message, const QVariantMap& context = {});
    void fatal(const QString& message, const QVariantMap& context = {});
    
    void setLogLevel(LogLevel level);
    LogLevel logLevel() const;
    
    void setCategoryEnabled(const QString& category, bool enabled);
    bool isCategoryEnabled(const QString& category) const;
    
signals:
    void logEntryAdded(const LogEntry& entry);
    
private:
    // Lock-free queue for async logging
    std::unique_ptr<LockFreeQueue<LogEntry>> m_logQueue;
    QThread* m_logThread;  // Dedicated background thread
    std::unique_ptr<ILogBackend> m_backend;  // Backend abstraction
};
```

### 3.2. Log Entry Structure

```cpp
struct LogEntry {
    QDateTime timestamp;
    LogService::LogLevel level;
    QString category;           // Component name (e.g., "NetworkManager")
    QString message;
    QVariantMap context;        // Key-value pairs for structured data
    QString threadId;
    QString file;
    int line;
    QString function;
};
```

### 3.3. Dependency Injection for Logging

**CRITICAL:** All services must use dependency injection for logging repositories. **NO global log objects.**

**Problem with Global Log Objects:**
```cpp
// ❌ BAD: Global singleton (hard to test, tight coupling)
LogService::instance()->info("User logged in");
```

**Solution: Dependency Injection:**
```cpp
// ✅ GOOD: Dependency injection (testable, loosely coupled)
class SecurityService {
public:
    SecurityService(
        IUserManagementService* userMgmtService,
        IActionLogRepository* actionLogRepo,  // ← Injected dependency
        IAuditRepository* auditRepo,          // ← Injected dependency
        QObject* parent = nullptr);
    
    void login(const QString& userId, const QString& secretCode) {
        // ... authentication logic ...
        
        // Log action via injected repository
        ActionLogEntry entry;
        entry.userId = userId;
        entry.actionType = "LOGIN";
        entry.result = "SUCCESS";
        m_actionLogRepo->logAction(entry);  // ← Use injected dependency
    }
    
private:
    IActionLogRepository* m_actionLogRepo;  // ← Store injected dependency
    IAuditRepository* m_auditRepo;
};
```

**Benefits:**
- ✅ **Testable:** Can inject mock repositories in unit tests
- ✅ **Flexible:** Can swap implementations (SQLite, in-memory, remote)
- ✅ **Explicit Dependencies:** Constructor shows all dependencies
- ✅ **No Hidden Coupling:** No global state

**Logging Repositories:**
- `IActionLogRepository` - For user actions (login, logout, configuration changes)
- `IAuditRepository` - For security audit events (authentication, certificate operations)
- `LogService` - For application logs (debug, info, error messages)

**See:** [39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md](./39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md) for complete dependency injection strategy.

### 3.4. Log Outputs and Storage

**Application Logs (LogService):**
- **File:** Rotated text/JSON files in `logs/z-monitor.log.*` (7-day retention)
- **Console:** Standard output (development only, disabled in production)
- **Diagnostics View:** In-memory buffer for UI display (last 1000 entries, RAM only)

**Action Logs (IActionLogRepository):**
- **Database:** `action_log` table in SQLite (encrypted with SQLCipher)
- **Retention:** 90 days minimum (configurable)
- **Format:** Structured database rows with JSON `details` field
- **Thread:** Database I/O Thread (asynchronous, non-blocking)

**Security Audit Logs (IAuditRepository):**
- **Database:** `security_audit_log` table in SQLite (encrypted with SQLCipher)
- **Retention:** 90 days minimum (configurable, required for compliance)
- **Format:** Structured database rows with JSON `details` field
- **Thread:** Database I/O Thread (asynchronous, non-blocking)

**Storage Summary:**

| Log Type | Storage Medium | Location | Format | Retention |
|----------|---------------|----------|--------|-----------|
| Application Logs | File System | `logs/z-monitor.log.*` | Text/JSON | 7 days |
| Action Logs | Database | `action_log` table | Structured rows + JSON | 90 days |
| Security Audit Logs | Database | `security_audit_log` table | Structured rows + JSON | 90 days |
| Diagnostics View | RAM | In-memory buffer | Structured objects | Last 1000 entries |

## 4. Decision Guide: Which Logging Mechanism to Use?

### 4.1. Quick Decision Tree

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

### 4.2. Examples by Scenario

**Scenario 1: User logs in**
```cpp
// ✅ CORRECT: Use IActionLogRepository (user action)
ActionLogEntry entry;
entry.actionType = "LOGIN";
entry.userId = "NURSE001";
entry.result = "SUCCESS";
m_actionLogRepo->logAction(entry);

// ❌ WRONG: Don't use LogService for user actions
// LogService::info("User logged in");  // Wrong!
```

**Scenario 2: Login attempt fails**
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

## 5. Structured Logging Formats

### 5.1. Application Logs Format (LogService)

**Human-Readable Format (Development):**
```
[2025-11-27 14:30:25.123] [ERROR] [NetworkManager] Telemetry send failed
  deviceId: "ZM-ICU-04"
  patientMrn: "MRN-12345"
  errorCode: 1001
  retryAttempt: 2
  serverUrl: "https://server.example.com"
```

**JSON Format (Production):**
```json
{
  "timestamp": "2025-11-27T14:30:25.123Z",
  "level": "ERROR",
  "category": "NetworkManager",
  "message": "Telemetry send failed",
  "context": {
    "deviceId": "ZM-ICU-04",
    "patientMrn": "MRN-12345",
    "errorCode": 1001,
    "retryAttempt": 2,
    "serverUrl": "https://server.example.com"
  },
  "threadId": "NetworkThread",
  "file": "NetworkManager.cpp",
  "line": 245,
  "function": "sendTelemetry"
}
```

### 5.2. Action Logs Format (IActionLogRepository)

**Database Row Format:**
```sql
-- Stored in action_log table
INSERT INTO action_log (
    timestamp_ms, timestamp_iso, user_id, user_role,
    action_type, target_type, target_id, details,
    result, device_id, session_token_hash, previous_hash
) VALUES (
    1632500000123,
    '2025-11-27T14:30:00.123Z',
    'NURSE001',
    'NURSE',
    'LOGIN',
    NULL,
    NULL,
    '{"login_method":"secret_code","device_id":"ZM-ICU-04"}',  -- JSON string
    'SUCCESS',
    'ZM-ICU-04',
    'a1b2c3d4e5f6...',
    'previous-entry-hash'
);
```

**Details Field (JSON):**
```json
{
  "login_method": "secret_code",
  "device_id": "ZM-ICU-04"
}
```

### 5.3. Security Audit Logs Format (IAuditRepository)

**Database Row Format:**
```sql
-- Stored in security_audit_log table
INSERT INTO security_audit_log (
    timestamp, event_type, severity, device_id, user_id,
    event_category, success, details, error_code, error_message, previous_hash
) VALUES (
    1632500000123,
    'LOGIN_FAILED',
    'WARNING',
    'ZM-ICU-04',
    'NURSE001',
    'AUTHENTICATION',
    false,
    '{"reason":"INVALID_CREDENTIALS","remainingAttempts":2}',  -- JSON string
    'AUTH_001',
    'Invalid user ID or secret code',
    'previous-entry-hash'
);
```

**Details Field (JSON):**
```json
{
  "reason": "INVALID_CREDENTIALS",
  "remainingAttempts": 2
}
```

### 5.4. Common Context Keys

**Application Logs (LogService):**
- `deviceId`: Device identifier
- `patientMrn`: Patient MRN (when applicable)
- `userId`: User identifier (for user actions)
- `operation`: Operation name
- `duration`: Operation duration in milliseconds
- `errorCode`: Error code
- `retryAttempt`: Retry attempt number
- `queueDepth`: Queue depth (for performance monitoring)

**Action Logs (IActionLogRepository):**
- `actionType`: Action type (LOGIN, ADMIT_PATIENT, etc.)
- `targetType`: Type of target (PATIENT, SETTING, NOTIFICATION)
- `targetId`: Target identifier (MRN, setting name, etc.)
- `result`: SUCCESS, FAILURE, PARTIAL
- `details`: JSON object with additional context

**Security Audit Logs (IAuditRepository):**
- `eventType`: Event type (LOGIN_FAILED, CERTIFICATE_INSTALLED, etc.)
- `eventCategory`: Category (AUTHENTICATION, CERTIFICATE, NETWORK)
- `severity`: INFO, WARNING, ERROR, CRITICAL
- `success`: Boolean (true/false)
- `details`: JSON object with additional context

## 6. Log Categories (Application Logs Only)

**Note:** Categories are only used for application logs (LogService). Action logs and security audit logs do not use categories.

### 6.1. Component Categories

Each component should use its own category for application logs:

- `NetworkManager`: Network operations
- `DatabaseManager`: Database operations
- `AlarmManager`: Alarm processing
- `PatientManager`: Patient operations
- `SecurityService`: Authentication and security
- `ProvisioningService`: Device provisioning
- `DeviceSimulator`: Simulated device data
- `MonitoringService`: Vitals monitoring
- `AdmissionService`: Patient admission/discharge

### 6.2. Category Filtering

Enable/disable categories at runtime via SettingsManager:

```cpp
// Via SettingsManager (no global LogService::instance())
SettingsManager::instance()->setValue("log.category.NetworkManager.enabled", true);
SettingsManager::instance()->setValue("log.category.DeviceSimulator.enabled", false);  // Disable in production
```

## 7. Log Format (Application Logs Only)

**Note:** Format selection only applies to application logs (LogService). Action logs and security audit logs are always stored as structured database rows.

### 7.1. Human-Readable Format

```
[2025-01-15 14:30:25.123] [ERROR] [NetworkManager] Telemetry send failed
  deviceId: "ZM-001"
  patientMrn: "12345"
  errorCode: 1001
  retryAttempt: 2
  serverUrl: "https://server.example.com"
```

### 6.2. JSON Format (For Analysis)

```json
{
  "timestamp": "2025-01-15T14:30:25.123Z",
  "level": "ERROR",
  "category": "NetworkManager",
  "message": "Telemetry send failed",
  "context": {
    "deviceId": "ZM-001",
    "patientMrn": "12345",
    "errorCode": 1001,
    "retryAttempt": 2,
    "serverUrl": "https://server.example.com"
  },
  "threadId": "NetworkThread",
  "file": "NetworkManager.cpp",
  "line": 245,
  "function": "sendTelemetry"
}
```

### 6.3. Format Selection

- **Development:** Human-readable format for easy reading
- **Production:** JSON format for log aggregation and analysis
- **Configurable:** Format selection via settings

## 7. Log Rotation

### 7.1. Rotation Policy

- **Size-based:** Rotate when log file exceeds size limit (default: 10 MB)
- **Time-based:** Rotate daily at midnight
- **Count-based:** Keep maximum number of rotated files (default: 7)

### 7.2. Rotation Strategy

```
logs/
  z-monitor.log          (current)
  z-monitor.log.1        (yesterday)
  z-monitor.log.2        (2 days ago)
  ...
  z-monitor.log.7        (7 days ago, then deleted)
```

### 7.3. Implementation

```cpp
void LogService::rotateLogsIfNeeded() {
    QFileInfo fileInfo(logFilePath);
    if (fileInfo.size() > maxLogSize) {
        rotateLogFile();
    }
    
    // Remove old log files
    QDir logDir = fileInfo.dir();
    QStringList logFiles = logDir.entryList(
        {"z-monitor.log.*"}, 
        QDir::Files, 
        QDir::Time | QDir::Reversed
    );
    
    while (logFiles.size() >= maxLogFiles) {
        logDir.remove(logFiles.first());
        logFiles.removeFirst();
    }
}
```

## 8. Performance Considerations

### 8.1. Asynchronous Logging

**Critical:** All logging operations are asynchronous and non-blocking. `LogService` methods return immediately (< 1μs) by enqueueing log entries to a lock-free queue. The Database I/O Thread processes the queue and writes to the file system (shared with database operations).

**Architecture:**
```
Calling Thread (any thread)
    ↓
LogService::warning() → Returns immediately (< 1μs)
    ↓
Lock-Free Queue (MPSC - Multiple Producer Single Consumer)
    ↓
Database I/O Thread (shared with database operations)
    ↓
ILogBackend (abstraction layer)
    ↓
Concrete Backend (spdlog, custom, etc.)
    ↓
File System
```

**Implementation:**
- **Queue:** Lock-free queue (moodycamel::ConcurrentQueue or boost::lockfree::queue)
- **Thread:** Database I/O Thread (shared with database operations - both are non-critical background tasks)
- **Backend Abstraction:** `ILogBackend` interface allows switching between logging libraries
- **Performance:** < 1μs per log call, < 100ns queue enqueue
- **Rationale:** Logging and database operations both perform file I/O and are non-critical, so sharing the thread reduces overhead

**See:** [43_ASYNC_LOGGING_ARCHITECTURE.md](./43_ASYNC_LOGGING_ARCHITECTURE.md) for complete implementation details.

### 8.2. Pre-allocated Buffers

Use pre-allocated buffers for log messages in hot paths:

```cpp
class LogBuffer {
    char buffer[1024];  // Pre-allocated
    int pos = 0;
    
    void append(const QString& str) {
        // Write to pre-allocated buffer
    }
};
```

### 8.3. Conditional Compilation

Disable expensive logging in production:

```cpp
#ifdef QT_DEBUG
    LogService::trace("Processing sample", {{"sampleId", sampleId}});
#endif
```

## 9. Security and Privacy

### 9.1. Never Log Sensitive Data

**DO NOT LOG:**
- Passwords or PINs (even hashed)
- Private keys or certificates
- Full patient data (only MRN when necessary)
- Authentication tokens
- Encryption keys

**DO LOG:**
- Operation outcomes (success/failure)
- Error codes
- User IDs (not passwords)
- Device IDs
- Timestamps
- Operation types

### 9.2. Patient Data Logging

- **MRN:** Can be logged for audit purposes
- **Patient Name:** Only in security audit log, not in general logs
- **Vitals Data:** Never log actual vitals values in general logs
- **Alarms:** Log alarm type and priority, not patient details

### 9.3. Log Access Control

- Log files must be readable only by authorized processes
- Security audit logs must have restricted access
- Log rotation must preserve file permissions

## 10. Log Retention

### 10.1. Retention Policy

| Log Type | Retention Period | Storage Location | Format | Cleanup |
|----------|----------------|------------------|--------|---------|
| **Application Logs** | 7 days | `logs/z-monitor.log.*` (file system) | Text/JSON files | Automatic rotation (keep 7 files) |
| **Action Logs** | 90 days (minimum, configurable) | `action_log` table (database) | Database rows | `DataCleanupService` (daily at 3 AM) |
| **Security Audit Logs** | 90 days (minimum, configurable, required for compliance) | `security_audit_log` table (database) | Database rows | `DataCleanupService` (daily at 3 AM) |
| **Diagnostics View** | In-memory only (last 1000 entries) | RAM (in-memory buffer) | Structured objects | Automatic (FIFO queue) |

**Retention Details:**

1. **Application Logs (File-Based):**
   - Rotated daily at midnight
   - Keep maximum 7 rotated files (7 days)
   - Old files automatically deleted
   - Format: Human-readable (dev) or JSON (production)

2. **Action Logs (Database):**
   - Stored in `action_log` table
   - Retention: 90 days minimum (configurable via settings)
   - Cleanup: `DataCleanupService` runs daily at 3 AM
   - Deletes entries older than retention period
   - Format: Structured database rows with JSON `details` field

3. **Security Audit Logs (Database):**
   - Stored in `security_audit_log` table
   - Retention: 90 days minimum (required for compliance)
   - Cleanup: `DataCleanupService` runs daily at 3 AM
   - Deletes entries older than retention period
   - Format: Structured database rows with JSON `details` field

4. **Diagnostics View (In-Memory):**
   - Last 1000 log entries kept in RAM
   - FIFO queue (oldest entries removed when limit reached)
   - Lost on application restart
   - Format: Structured `LogEntry` objects

### 10.2. Archival

- Old logs can be archived to external storage
- Archived logs must be encrypted
- Archive format: Compressed tar.gz with encryption

## 11. Log Analysis and Monitoring

### 11.1. Log Aggregation

- Collect logs from all devices to central server
- Use structured format (JSON) for easy parsing
- Index logs by timestamp, level, category, deviceId

### 11.2. Log Queries

Common queries:

- Error rate over time
- Most common errors
- Errors by device
- Errors by category
- Performance metrics (operation duration)

### 11.3. Alerting

Alert on:

- CRITICAL or FATAL log entries
- Error rate exceeds threshold
- Security-related log entries
- Log rotation failures

## 12. Diagnostics View Integration

### 12.1. In-Memory Buffer

Diagnostics View displays last N log entries from in-memory buffer:

```cpp
class LogService {
private:
    QList<LogEntry> recentLogs;  // Last 1000 entries
    static const int MAX_RECENT_LOGS = 1000;
    
    void addLogEntry(const LogEntry& entry) {
        recentLogs.append(entry);
        if (recentLogs.size() > MAX_RECENT_LOGS) {
            recentLogs.removeFirst();
        }
        emit logEntryAdded(entry);
    }
};
```

### 12.2. UI Display

- Color-coded by log level
- Filterable by level and category
- Searchable by message content
- Auto-scroll to latest entries

## 13. Testing Logging

### 13.1. Unit Tests

- Verify log entries are created correctly
- Verify log levels are respected
- Verify context is included
- Verify sensitive data is not logged

### 13.2. Integration Tests

- Verify logs are written to file
- Verify log rotation works
- Verify diagnostics view receives logs
- Verify security audit logs are created

## 14. Configuration

### 14.1. Log Settings

Configurable via `SettingsManager`:

- `log.level`: Minimum log level (default: INFO)
- `log.file.enabled`: Enable file logging (default: true)
- `log.console.enabled`: Enable console logging (default: false in production)
- `log.file.path`: Log file path (default: `logs/z-monitor.log`)
- `log.file.maxSize`: Maximum log file size in MB (default: 10)
- `log.file.maxFiles`: Maximum number of rotated files (default: 7)
- `log.format`: Log format ("human" or "json", default: "json" in production)

### 14.2. Runtime Configuration

**Application Logs (LogService):**
```cpp
// LogService is injected via dependency injection (no global instance)
// Configuration is done via SettingsManager

// Set log level at runtime (via SettingsManager)
SettingsManager::instance()->setValue("log.level", "DEBUG");

// Enable/disable categories (via SettingsManager)
SettingsManager::instance()->setValue("log.category.NetworkManager.enabled", true);
SettingsManager::instance()->setValue("log.category.DeviceSimulator.enabled", false);
```

**Action Logs (IActionLogRepository):**
- No runtime configuration needed
- All actions are logged automatically
- Retention period configurable via SettingsManager: `action_log.retention_days`

**Security Audit Logs (IAuditRepository):**
- No runtime configuration needed
- All security events are logged automatically
- Retention period configurable via SettingsManager: `security_audit_log.retention_days`

## 15. Best Practices

### 15.1. Do's

- ✅ Use appropriate log levels
- ✅ Include relevant context
- ✅ Use structured logging
- ✅ Log at service boundaries
- ✅ Log errors with full context
- ✅ Use categories for filtering
- ✅ Test logging in unit tests

### 15.2. Don'ts

- ❌ Don't log sensitive data
- ❌ Don't log in hot paths (use conditional compilation)
- ❌ Don't use string concatenation for log messages (use structured context)
- ❌ Don't log at TRACE level in production
- ❌ Don't block on logging
- ❌ Don't log too frequently (throttle if needed)

## 16. Related Documents

- [43_ASYNC_LOGGING_ARCHITECTURE.md](./43_ASYNC_LOGGING_ARCHITECTURE.md) - **Async logging architecture, ILogBackend abstraction, implementation details** ⭐
- [39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md](./39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md) - Action logging workflow and IActionLogRepository interface
- [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) - Error logging
- [06_SECURITY.md](./06_SECURITY.md) - Security audit logging
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Database schema for `action_log` and `security_audit_log` tables
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - LogService, IActionLogRepository, IAuditRepository class designs
- [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) - Dependency injection strategy (required for logging)
- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Thread architecture (LogService shares Database I/O Thread)

