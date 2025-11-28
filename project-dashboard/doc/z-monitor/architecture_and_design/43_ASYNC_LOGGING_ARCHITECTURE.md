# Async Logging Architecture

**Document ID:** DESIGN-043  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the asynchronous logging architecture for the Z Monitor application, ensuring that all logging operations return immediately without blocking the calling thread. It includes the abstraction layer design for logging backends to allow easy switching between logging libraries.

## 1. Overview

### 1.1. Critical Requirements

- **Non-Blocking:** All `LogService` methods (e.g., `warning()`, `error()`, `info()`) must return immediately
- **Async Buffering:** Log entries are buffered in memory and written to disk in a separate thread
- **Library Abstraction:** Easy to switch between logging libraries (spdlog, glog, custom, etc.)
- **Performance:** Zero impact on real-time threads (< 1μs per log call)
- **Thread Safety:** Safe to call from any thread without synchronization

### 1.2. Architecture Overview

```
Calling Thread (any thread)
    ↓
LogService::warning() → Returns immediately (< 1μs)
    ↓
Lock-Free Queue (MPSC - Multiple Producer Single Consumer)
    ↓
Database I/O Thread (shared with database operations)
    ↓
ILogBackend (abstraction)
    ↓
Concrete Backend (spdlog, glog, custom, etc.)
    ↓
File System
```

**Note:** LogService shares the Database I/O Thread with database operations. Both are non-critical background tasks that perform file I/O, so sharing the thread reduces overhead while maintaining non-blocking behavior.

## 2. Component Design

### 2.1. LogService (Public Interface)

**Location:** `src/infrastructure/logging/LogService.h/cpp`

**Responsibility:** Provides the public logging API. All methods enqueue log entries to a lock-free queue and return immediately.

```cpp
/**
 * @class LogService
 * @brief Provides asynchronous, non-blocking logging for application events.
 * 
 * All logging methods return immediately (< 1μs) by enqueueing log entries
 * to a lock-free queue. The Database I/O Thread processes the queue
 * and writes to the configured logging backend (shared with database operations).
 * 
 * @note Thread-safe: Can be called from any thread without synchronization.
 * @note Non-blocking: All methods return immediately.
 * @note Thread: Runs on Database I/O Thread (shared with database operations).
 * 
 * @ingroup Infrastructure
 */
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
    
    /**
     * @brief Constructs a LogService with the specified backend.
     * 
     * @param backend Logging backend implementation (ownership transferred)
     * @param parent Parent QObject
     */
    explicit LogService(ILogBackend* backend, QObject* parent = nullptr);
    
    ~LogService();
    
    // Public logging methods - all return immediately
    void trace(const QString& message, const QVariantMap& context = {});
    void debug(const QString& message, const QVariantMap& context = {});
    void info(const QString& message, const QVariantMap& context = {});
    void warning(const QString& message, const QVariantMap& context = {});
    void error(const QString& message, const QVariantMap& context = {});
    void critical(const QString& message, const QVariantMap& context = {});
    void fatal(const QString& message, const QVariantMap& context = {});
    
    // Configuration
    void setLogLevel(LogLevel level);
    LogLevel logLevel() const;
    
    void setCategoryEnabled(const QString& category, bool enabled);
    bool isCategoryEnabled(const QString& category) const;
    
signals:
    void logEntryAdded(const LogEntry& entry);
    
private slots:
    void processLogQueue();  // Called by Database I/O Thread event loop
    
private:
    struct LogEntry {
        QDateTime timestamp;
        LogLevel level;
        QString category;
        QString message;
        QVariantMap context;
        QString threadId;
        QString file;
        int line;
        QString function;
    };
    
    void enqueueLog(LogLevel level, const QString& message, 
                    const QVariantMap& context = {});
    
    LogLevel m_minLevel;
    QMap<QString, bool> m_categoryEnabled;
    
    // Lock-free queue for async logging
    std::unique_ptr<LockFreeQueue<LogEntry>> m_logQueue;
    
    // Worker object on Database I/O Thread (shared with database operations)
    QObject* m_logWorker;  // Worker object on Database I/O Thread
    
    // Backend abstraction
    std::unique_ptr<ILogBackend> m_backend;
    
    // In-memory buffer for Diagnostics View (last 1000 entries)
    QList<LogEntry> m_recentLogs;
    static constexpr int MAX_RECENT_LOGS = 1000;
};
```

### 2.2. ILogBackend (Abstraction Interface)

**Location:** `src/infrastructure/logging/ILogBackend.h`

**Responsibility:** Defines the interface for logging backends, allowing easy switching between libraries.

```cpp
/**
 * @class ILogBackend
 * @brief Abstract interface for logging backends.
 * 
 * This interface allows LogService to work with any logging library
 * (spdlog, glog, custom, etc.) by abstracting the write operations.
 * 
 * @note Implementations must be thread-safe and handle log rotation.
 * 
 * @ingroup Infrastructure
 */
class ILogBackend {
public:
    virtual ~ILogBackend() = default;
    
    /**
     * @brief Initializes the logging backend.
     * 
     * Called once during LogService construction. Should open log files,
     * configure formatters, etc.
     * 
     * @param logDir Directory for log files
     * @param logFileName Base name for log files (e.g., "z-monitor")
     * @return true if initialization succeeded, false otherwise
     */
    virtual bool initialize(const QString& logDir, const QString& logFileName) = 0;
    
    /**
     * @brief Writes a log entry to the backend.
     * 
     * Called from the Database I/O Thread. Must be thread-safe and non-blocking
     * (or at least fast enough to not block the Database I/O Thread).
     * 
     * @param entry Log entry to write
     */
    virtual void write(const LogEntry& entry) = 0;
    
    /**
     * @brief Flushes any buffered log entries.
     * 
     * Called during shutdown or when explicitly requested.
     */
    virtual void flush() = 0;
    
    /**
     * @brief Rotates log files if needed.
     * 
     * Called periodically or when file size threshold is reached.
     */
    virtual void rotateIfNeeded() = 0;
    
    /**
     * @brief Sets the log format.
     * 
     * @param format Format string ("human" or "json")
     */
    virtual void setFormat(const QString& format) = 0;
    
    /**
     * @brief Sets the maximum log file size before rotation.
     * 
     * @param maxSizeBytes Maximum size in bytes
     */
    virtual void setMaxFileSize(qint64 maxSizeBytes) = 0;
    
    /**
     * @brief Sets the maximum number of rotated log files to keep.
     * 
     * @param maxFiles Maximum number of files
     */
    virtual void setMaxFiles(int maxFiles) = 0;
};
```

### 2.3. SpdlogBackend (Example Implementation)

**Location:** `src/infrastructure/logging/backends/SpdlogBackend.h/cpp`

**Responsibility:** Concrete implementation using spdlog library.

```cpp
/**
 * @class SpdlogBackend
 * @brief spdlog-based logging backend implementation.
 * 
 * Uses spdlog library for high-performance, async logging with
 * automatic file rotation and formatting.
 * 
 * @note Requires spdlog library dependency.
 * 
 * @ingroup Infrastructure
 */
class SpdlogBackend : public ILogBackend {
public:
    SpdlogBackend();
    ~SpdlogBackend() override;
    
    bool initialize(const QString& logDir, const QString& logFileName) override;
    void write(const LogEntry& entry) override;
    void flush() override;
    void rotateIfNeeded() override;
    void setFormat(const QString& format) override;
    void setMaxFileSize(qint64 maxSizeBytes) override;
    void setMaxFiles(int maxFiles) override;
    
private:
    std::shared_ptr<spdlog::logger> m_logger;
    QString m_logDir;
    QString m_logFileName;
    QString m_format;
    qint64 m_maxFileSize;
    int m_maxFiles;
};
```

### 2.4. CustomBackend (Fallback Implementation)

**Location:** `src/infrastructure/logging/backends/CustomBackend.h/cpp`

**Responsibility:** Custom Qt-based implementation (no external dependencies).

```cpp
/**
 * @class CustomBackend
 * @brief Custom Qt-based logging backend (no external dependencies).
 * 
 * Pure Qt implementation using QFile and QTextStream. Suitable for
 * environments where external logging libraries are not available.
 * 
 * @ingroup Infrastructure
 */
class CustomBackend : public ILogBackend {
public:
    CustomBackend();
    ~CustomBackend() override;
    
    bool initialize(const QString& logDir, const QString& logFileName) override;
    void write(const LogEntry& entry) override;
    void flush() override;
    void rotateIfNeeded() override;
    void setFormat(const QString& format) override;
    void setMaxFileSize(qint64 maxSizeBytes) override;
    void setMaxFiles(int maxFiles) override;
    
private:
    QFile* m_logFile;
    QTextStream* m_stream;
    QString m_logFilePath;
    qint64 m_maxFileSize;
    int m_maxFiles;
    QString m_format;
    
    void rotateLogFile();
    QString formatEntry(const LogEntry& entry) const;
    QString formatJson(const LogEntry& entry) const;
    QString formatHuman(const LogEntry& entry) const;
};
```

## 3. Threading Model

### 3.1. Rationale: Why Share Database I/O Thread Instead of Dedicated Thread?

**Question:** Why not have a dedicated "Log Thread" for logging?

**Answer:** Logging shares the Database I/O Thread because:

1. **Both are non-critical background tasks:**
   - Logging: Non-blocking, can tolerate delays
   - Database operations: Background persistence (every 10 min), cleanup (daily at 3 AM)
   - Neither affects real-time alarm detection or UI responsiveness

2. **Both perform file I/O:**
   - Logging: Writes to log files
   - Database: Writes to SQLite database files
   - Both benefit from the same I/O priority and scheduling

3. **Reduced overhead:**
   - One less thread = less context switching
   - Less memory usage (thread stack, TLS)
   - Simpler thread management

4. **Event loop sharing:**
   - Both can use Qt's event loop for async operations
   - Queue processing can be scheduled via `QTimer` or signals
   - No need for separate event loop

5. **No interference:**
   - Log queue processing is lightweight (batched, yields to other operations)
   - Database operations are infrequent (every 10 min or daily)
   - Both are non-blocking (async I/O)

**When to use a dedicated thread:**
- If logging becomes high-frequency (> 10,000 entries/second)
- If database operations become time-critical
- If profiling shows interference between logging and database operations

**For Z Monitor:** Sharing is appropriate because logging is moderate frequency (~1,000 entries/second) and database operations are infrequent background tasks.

### 3.2. Log Thread (Shared with Database I/O Thread)

**Thread:** Database I/O Thread (shared with database operations)

**Priority:** I/O Priority (background, but responsive)

**Purpose:** Processes log queue and writes to file system

**Rationale for Sharing:**
- **Both are non-critical:** Logging and database operations are background tasks
- **Both do file I/O:** Log files and database files are both disk operations
- **Reduced overhead:** One less thread = less context switching and memory usage
- **Event loop sharing:** Both can use the same Qt event loop for queue processing
- **No interference:** Log queue processing is lightweight and won't delay database operations

**Components:**
- `LogService::m_logWorker` (QObject on Database I/O Thread)
- `ILogBackend` implementation
- File I/O operations (log files)
- Shares thread with: DatabaseManager, repositories, PersistenceScheduler, DataCleanupService, DataArchiveService

### 3.2. Communication

**Producer (Any Thread):**
```cpp
// Called from any thread (real-time, UI, network, etc.)
m_logService->warning("Network error", {{"errorCode", "1001"}});
// Returns immediately (< 1μs)
```

**Queue (Lock-Free):**
- **Type:** SPSC (Single Producer Single Consumer) or MPSC (Multiple Producer Single Consumer)
- **Implementation:** Use `moodycamel::ConcurrentQueue` or `boost::lockfree::queue`
- **Size:** Configurable (default: 10,000 entries)
- **Behavior:** Non-blocking enqueue, blocking dequeue with timeout

**Consumer (Database I/O Thread):**
```cpp
// In Database I/O Thread event loop (shared with database operations)
void LogWorker::processLogQueue() {
    LogEntry entry;
    int processed = 0;
    const int MAX_BATCH = 100; // Process up to 100 entries per call
    
    // Process queue in batches to avoid blocking database operations
    while (processed < MAX_BATCH && m_logQueue->try_dequeue(entry)) {
        m_backend->write(entry);
        // Update in-memory buffer for Diagnostics View
        emit logEntryAdded(entry);
        processed++;
    }
    
    // If queue still has entries, schedule another processing cycle
    if (!m_logQueue->empty_approx()) {
        QTimer::singleShot(10, this, &LogWorker::processLogQueue);
    }
}
```

**Note:** Queue processing is batched and uses `QTimer::singleShot()` to yield to other Database I/O Thread operations, ensuring database operations are not delayed by logging.

## 4. Lock-Free Queue Implementation

### 4.1. Queue Selection

**Options:**
1. **moodycamel::ConcurrentQueue** (Recommended)
   - Header-only, MIT license
   - Supports MPSC (Multiple Producer Single Consumer)
   - High performance, lock-free
   - Easy to integrate

2. **boost::lockfree::queue**
   - Requires Boost library
   - SPSC (Single Producer Single Consumer) or MPSC
   - Well-tested, part of Boost

3. **Custom Lock-Free Queue**
   - If no external dependencies allowed
   - More complex to implement correctly

### 4.2. Queue Configuration

```cpp
// In LogService constructor
m_logQueue = std::make_unique<moodycamel::ConcurrentQueue<LogEntry>>(10000);
```

## 5. Logging Library Options

### 5.1. spdlog (Recommended)

**Pros:**
- Very fast (async mode)
- Header-only or compiled
- Automatic file rotation
- Multiple sinks (file, console, etc.)
- JSON formatting support
- Well-maintained

**Cons:**
- External dependency
- C++17 required

**Integration:**
```cpp
// SpdlogBackend implementation
void SpdlogBackend::initialize(const QString& logDir, const QString& logFileName) {
    std::string logPath = (logDir + "/" + logFileName + ".log").toStdString();
    
    // Create async file logger with rotation
    m_logger = spdlog::rotating_logger_mt(
        "z-monitor",
        logPath,
        m_maxFileSize,
        m_maxFiles
    );
    
    // Set format
    if (m_format == "json") {
        m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
    } else {
        m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] [%n] %v");
    }
}
```

### 5.2. Custom Qt Implementation (Fallback)

**Pros:**
- No external dependencies
- Full control
- Qt integration

**Cons:**
- More code to maintain
- Potentially slower than spdlog

**Use When:**
- External dependencies not allowed
- Need maximum control
- Qt-only environment

### 5.3. glog (Google Logging)

**Pros:**
- Well-tested
- Good performance
- Automatic rotation

**Cons:**
- Less flexible than spdlog
- Google-style API
- May require more integration work

## 6. Directory Structure

```
src/infrastructure/logging/
├── LogService.h
├── LogService.cpp
├── ILogBackend.h
├── LogEntry.h
├── backends/
│   ├── SpdlogBackend.h
│   ├── SpdlogBackend.cpp
│   ├── CustomBackend.h
│   ├── CustomBackend.cpp
│   └── GlogBackend.h/cpp (optional)
└── utils/
    └── LogFormatter.h/cpp (shared formatting utilities)
```

## 7. Usage Example

### 7.1. Creating LogService

```cpp
// In application initialization
#include "infrastructure/logging/LogService.h"
#include "infrastructure/logging/backends/SpdlogBackend.h"

// Create backend
auto backend = std::make_unique<SpdlogBackend>();
backend->setMaxFileSize(10 * 1024 * 1024);  // 10 MB
backend->setMaxFiles(7);
backend->setFormat("json");

// Create LogService
auto logService = new LogService(backend.release(), parent);

// Configure
logService->setLogLevel(LogService::Info);
logService->setCategoryEnabled("NetworkManager", true);
```

### 7.2. Using LogService

```cpp
// In any service/component
class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    NetworkManager(LogService* logService, QObject* parent = nullptr)
        : QObject(parent), m_logService(logService) {}
    
    void sendTelemetry(const TelemetryBatch& batch) {
        // This returns immediately (< 1μs)
        m_logService->info("Sending telemetry", {
            {"deviceId", batch.deviceId},
            {"patientMrn", batch.patientMrn},
            {"recordCount", QString::number(batch.vitals.size())}
        });
        
        // ... actual telemetry sending ...
    }
    
private:
    LogService* m_logService;
};
```

## 8. Performance Characteristics

### 8.1. Latency Targets

| Operation | Target | Notes |
|-----------|--------|-------|
| `LogService::warning()` | < 1μs | Enqueue to lock-free queue |
| Queue enqueue | < 100ns | Lock-free operation |
| Database I/O Thread processing | < 10ms | Batch processing |
| File write | < 5ms | Async I/O |

### 8.2. Throughput

- **Queue Capacity:** 10,000 entries (configurable)
- **Processing Rate:** ~1,000 entries/second (depends on backend)
- **Backpressure:** If queue is full, drop oldest entries (configurable)

## 9. Configuration

### 9.1. Settings

Configurable via `SettingsManager`:

- `log.backend`: Backend type ("spdlog", "custom", "glog")
- `log.level`: Minimum log level
- `log.file.maxSize`: Maximum file size in bytes
- `log.file.maxFiles`: Maximum rotated files
- `log.format`: Format ("human" or "json")
- `log.queue.size`: Queue capacity
- `log.queue.dropOldest`: Drop oldest entries when full (true/false)

### 9.2. Runtime Configuration

```cpp
// Switch backends at runtime (requires restart)
SettingsManager::instance()->setValue("log.backend", "spdlog");

// Change log level without restart
logService->setLogLevel(LogService::Debug);

// Enable/disable categories
logService->setCategoryEnabled("DeviceSimulator", false);
```

## 10. Testing

### 10.1. Unit Tests

```cpp
TEST(LogService, NonBlocking) {
    auto backend = std::make_unique<MockLogBackend>();
    LogService service(backend.release());
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        service.warning("Test message", {{"index", QString::number(i)}});
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);  // < 1ms for 1000 calls
}
```

### 10.2. Integration Tests

- Verify logs are written to file
- Verify log rotation works
- Verify queue doesn't block
- Verify thread safety

## 11. Migration Plan

### 11.1. Current State

- `LogService` exists but may not be fully async
- No abstraction layer for backends
- May be in wrong thread (Database I/O Thread)

### 11.2. Migration Steps

1. **Create ILogBackend interface** (`src/infrastructure/logging/ILogBackend.h`)
2. **Move LogService** to `src/infrastructure/logging/` (from `src/infrastructure/qt/`)
3. **Implement async queue** (lock-free queue + Database I/O Thread processing)
4. **Create SpdlogBackend** implementation
5. **Create CustomBackend** fallback
6. **Update thread model** (LogService shares Database I/O Thread)
7. **Update all LogService usages** (inject via DI)
8. **Add configuration** (SettingsManager integration)

## 12. Related Documents

- [Logging Strategy (21_LOGGING_STRATEGY.md)](./21_LOGGING_STRATEGY.md) - Logging strategy and formats
- [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture
- [Code Organization (22_CODE_ORGANIZATION.md)](./22_CODE_ORGANIZATION.md) - Directory structure
- [Memory Management (23_MEMORY_RESOURCE_MANAGEMENT.md)](./23_MEMORY_RESOURCE_MANAGEMENT.md) - Lock-free queue details

---

**Next Steps:**
1. Implement `ILogBackend` interface
2. Implement `SpdlogBackend` (or choose alternative)
3. Refactor `LogService` to use async queue
4. Update thread model documentation
5. Add unit and integration tests

