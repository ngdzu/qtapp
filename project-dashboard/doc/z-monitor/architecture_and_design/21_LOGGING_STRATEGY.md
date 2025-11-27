# Logging Strategy

This document defines the logging architecture, log levels, log formats, log rotation, and log management strategy for the Z Monitor application.

## 1. Guiding Principles

- **Structured Logging:** Use structured log entries with key-value pairs for easy parsing and analysis
- **Appropriate Log Levels:** Use correct log levels to balance detail with noise
- **Performance:** Logging must not impact real-time performance
- **Security:** Never log sensitive data (passwords, certificates, patient data)
- **Auditability:** Critical operations must be logged for compliance
- **Rotation:** Implement log rotation to prevent disk space exhaustion

## 2. Log Levels

### 2.1. Log Level Hierarchy

```
FATAL > CRITICAL > ERROR > WARNING > INFO > DEBUG > TRACE
```

### 2.2. Log Level Definitions

| Level | When to Use | Example |
|-------|-------------|---------|
| **FATAL** | System cannot continue, must restart | Out of memory, disk full, critical corruption |
| **CRITICAL** | System-threatening error, requires immediate attention | Security violation, data integrity failure |
| **ERROR** | Operation failed, but system can continue | Network send failed, database query failed |
| **WARNING** | Unexpected condition, but operation succeeded | Retry occurred, degraded performance |
| **INFO** | Normal operation milestones | Patient admitted, connection established |
| **DEBUG** | Detailed information for debugging | Function entry/exit, intermediate values |
| **TRACE** | Very detailed tracing (disabled in production) | Per-sample processing, queue operations |

### 2.3. Production vs. Development

- **Production:** FATAL, CRITICAL, ERROR, WARNING, INFO
- **Development:** All levels including DEBUG
- **Debugging:** All levels including TRACE

## 3. Logging Architecture

### 3.1. LogService Design

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
    
    void log(LogLevel level, const QString& message, 
             const QVariantMap& context = {});
    
    void trace(const QString& message, const QVariantMap& context = {});
    void debug(const QString& message, const QVariantMap& context = {});
    void info(const QString& message, const QVariantMap& context = {});
    void warning(const QString& message, const QVariantMap& context = {});
    void error(const QString& message, const QVariantMap& context = {});
    void critical(const QString& message, const QVariantMap& context = {});
    void fatal(const QString& message, const QVariantMap& context = {});
    
    void setLogLevel(LogLevel level);
    void setOutputFile(const QString& path);
    void enableConsoleOutput(bool enable);
    
signals:
    void logEntryAdded(const LogEntry& entry);
    
private:
    void writeToFile(const LogEntry& entry);
    void writeToConsole(const LogEntry& entry);
    void rotateLogsIfNeeded();
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

### 3.3. Log Outputs

- **File:** Rotated log files in `logs/` directory
- **Console:** Standard output (development only)
- **Diagnostics View:** In-memory buffer for UI display
- **Security Audit Log:** Security events to `security_audit_log` table
- **Audit Log:** User actions to `audit_log` table

## 4. Structured Logging

### 4.1. Context Data

Always include relevant context:

```cpp
LogService::error(
    "Telemetry send failed",
    {
        {"deviceId", deviceId},
        {"patientMrn", patientMrn},
        {"errorCode", QString::number(error.code())},
        {"retryAttempt", QString::number(attempt)},
        {"serverUrl", serverUrl}
    }
);
```

### 4.2. Common Context Keys

- `deviceId`: Device identifier
- `patientMrn`: Patient MRN (when applicable)
- `userId`: User identifier (for user actions)
- `operation`: Operation name
- `duration`: Operation duration in milliseconds
- `errorCode`: Error code
- `retryAttempt`: Retry attempt number
- `queueDepth`: Queue depth (for performance monitoring)

## 5. Log Categories

### 5.1. Component Categories

Each component should use its own category:

- `NetworkManager`: Network operations
- `DatabaseManager`: Database operations
- `AlarmManager`: Alarm processing
- `PatientManager`: Patient operations
- `AuthenticationService`: Authentication
- `ProvisioningService`: Device provisioning
- `DeviceSimulator`: Simulated device data

### 5.2. Category Filtering

Enable/disable categories at runtime:

```cpp
LogService::setCategoryEnabled("NetworkManager", true);
LogService::setCategoryEnabled("DeviceSimulator", false);  // Disable in production
```

## 6. Log Format

### 6.1. Human-Readable Format

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

Logging should not block the calling thread:

```cpp
class LogService {
private:
    QThread* logThread;
    LockFreeQueue<LogEntry> logQueue;
    
    void log(LogLevel level, const QString& message, const QVariantMap& context) {
        LogEntry entry = createLogEntry(level, message, context);
        logQueue.enqueue(entry);  // Non-blocking
        // Background thread processes queue
    }
};
```

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

| Log Type | Retention Period | Storage Location |
|----------|----------------|------------------|
| Application Logs | 7 days | `logs/z-monitor.log.*` |
| Security Audit Log | 90 days (minimum) | `security_audit_log` table |
| Audit Log | 90 days | `audit_log` table |
| Diagnostics View | In-memory only (last 1000 entries) | RAM |

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

```cpp
// Set log level at runtime
LogService::instance()->setLogLevel(LogService::Debug);

// Enable/disable categories
LogService::instance()->setCategoryEnabled("NetworkManager", true);
LogService::instance()->setCategoryEnabled("DeviceSimulator", false);
```

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

- [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) - Error logging
- [06_SECURITY.md](./06_SECURITY.md) - Security audit logging
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Audit log tables
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - LogService class design

