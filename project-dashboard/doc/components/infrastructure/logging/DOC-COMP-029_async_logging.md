---
doc_id: DOC-COMP-029
title: Async Logging Architecture
version: 1.0
category: Components
subcategory: Infrastructure
status: Draft
owner: Infrastructure Team
reviewers: [Architecture, Performance]
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-001_software_architecture.md
  - DOC-ARCH-011_thread_model.md
  - DOC-GUIDE-012_logging_guidelines.md
tags: [logging, async, lock-free, spdlog, thread-safety]
source:
  path: project-dashboard/doc/z-monitor/architecture_and_design/43_ASYNC_LOGGING_ARCHITECTURE.md
  original_id: DESIGN-043
  last_updated: 2025-11-27
---

# Purpose
Defines async, non-blocking logging ensuring all `LogService` methods return immediately (< 1μs). Log entries buffered in lock-free queue, processed on Database I/O thread (shared with database operations).

# Critical Requirements
- **Non-Blocking:** All log methods return < 1μs
- **Async Buffering:** Memory buffer, disk writes on separate thread
- **Library Abstraction:** Easy switch between spdlog, glog, custom
- **Performance:** Zero impact on RT threads
- **Thread Safety:** Safe from any thread

# Architecture

## Data Flow
```
Calling Thread (any)
  → LogService::warning() (returns < 1μs)
  → Lock-Free Queue (MPSC)
  → Database I/O Thread (shared with DB ops)
  → ILogBackend (abstraction)
  → Concrete Backend (spdlog/glog/custom)
  → File System
```

## LogService (Public Interface)
```cpp
class LogService : public QObject {
    enum LogLevel { Trace, Debug, Info, Warning, Error, Critical, Fatal };
    
    void trace/debug/info/warning/error/critical/fatal(
        const QString& message, 
        const QVariantMap& context = {});
    
    void setLogLevel(LogLevel);
    void setCategoryEnabled(const QString& category, bool);

signals:
    void logEntryAdded(const LogEntry&);

private slots:
    void processLogQueue();  // Database I/O Thread event loop
};
```

## ILogBackend (Abstraction)
```cpp
class ILogBackend {
    virtual bool initialize(const QString& logDir, const QString& logFileName) = 0;
    virtual void write(const LogEntry&) = 0;
    virtual void flush() = 0;
    virtual void rotateIfNeeded() = 0;
    virtual void setFormat(const QString& format) = 0;  // "human" or "json"
    virtual void setMaxFileSize(qint64) = 0;
    virtual void setMaxFiles(int) = 0;
};
```

## Implementations
- **SpdlogBackend:** Fast async logging, auto rotation, JSON support (recommended)
- **CustomBackend:** Pure Qt implementation (QFile/QTextStream), no external deps
- **GlogBackend:** Google logging library (alternative)

# Threading Model

## Why Share Database I/O Thread?
1. Both non-critical background tasks (tolerate delays)
2. Both perform file I/O (log files, database files)
3. Reduced overhead (one less thread, less context switching)
4. Event loop sharing (Qt timers, signals)
5. No interference (batched queue processing, yields to DB ops)

## Queue Processing (Database I/O Thread)
```cpp
void LogWorker::processLogQueue() {
    int processed = 0;
    const int MAX_BATCH = 100;
    
    while (processed < MAX_BATCH && m_logQueue->try_dequeue(entry)) {
        m_backend->write(entry);
        emit logEntryAdded(entry);
        processed++;
    }
    
    if (!m_logQueue->empty_approx()) {
        QTimer::singleShot(10, this, &LogWorker::processLogQueue);
    }
}
```

# Lock-Free Queue
- **Recommended:** `moodycamel::ConcurrentQueue` (header-only, MIT, MPSC)
- **Alternative:** `boost::lockfree::queue` (requires Boost)
- **Capacity:** 10,000 entries (configurable)
- **Behavior:** Non-blocking enqueue, blocking dequeue with timeout

# Logging Library Options

## spdlog (Recommended)
- **Pros:** Very fast (async), header-only, auto rotation, JSON, well-maintained
- **Cons:** External dependency, C++17
- **Use Case:** Production deployments, high performance

## Custom Qt Backend (Fallback)
- **Pros:** No external deps, full control, Qt integration
- **Cons:** More maintenance, potentially slower
- **Use Case:** External deps not allowed, Qt-only environment

## glog (Google Logging)
- **Pros:** Well-tested, good performance, auto rotation
- **Cons:** C-style API, less modern

# Configuration Example
```cpp
LogService* logService = new LogService(new SpdlogBackend());
logService->setLogLevel(LogService::Debug);
logService->setCategoryEnabled("network", true);
logService->setCategoryEnabled("database", false);
```

# Diagnostics View
- In-memory buffer: Last 1,000 log entries
- Accessible via UI for troubleshooting
- Signal: `logEntryAdded(const LogEntry&)`

# Verification
- Functional: Log from multiple threads simultaneously; verify no blocking, correct ordering
- Code Quality: Abstract backend interface; no hardcoded paths/formats
- Documentation: Diagram showing queue flow, thread assignments
- Integration: End-to-end test with spdlog backend; verify rotation, JSON format
- Tests: Unit tests for queue, batching; stress tests for high-frequency logging (10k entries/sec)

# Document Metadata
| Field          | Value        |
| -------------- | ------------ |
| Original Doc   | DESIGN-043   |
| Migration Date | 2025-12-01   |
| New Doc ID     | DOC-COMP-029 |

# Revision History
- 1.0 (2025-12-01): Migrated from 43_ASYNC_LOGGING_ARCHITECTURE.md; consolidated async logging architecture.
