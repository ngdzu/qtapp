---
doc_id: DOC-COMP-025
title: LogService
version: v1.0
category: Component
subcategory: Infrastructure Layer / Logging
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-COMP-018, DOC-ARCH-002]
related_tasks: [TASK-3C-001]
tags: [infrastructure, logging, asynchronous, non-blocking, queue]
diagram_files: []
---

# DOC-COMP-025: LogService

## 1. Overview

**Purpose:** Asynchronous, non-blocking logging service for application events. All logging methods return immediately (<1μs) by enqueueing entries to a lock-free queue. Database I/O Thread processes the queue.

**Responsibilities:**
- Enqueue log entries to lock-free queue (< 1μs per call)
- Process log queue on Database I/O Thread
- Write log entries to ILogBackend (database, file, console)
- Support multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Provide context metadata (timestamp, thread, file, line, function)
- Thread-safe from any thread

**Layer:** Infrastructure Layer (Logging)

**Module:** `z-monitor/src/infrastructure/logging/LogService.h` (298 lines)

**Thread Affinity:** Database I/O Thread (shared with database operations)

**Dependencies:**
- **ILogBackend:** Logging backend interface (SQLite, file, console)
- **LogEntry:** Log entry value object
- **TemporaryQueue:** Lock-free queue (using QQueue + mutex, TODO: replace with moodycamel::ConcurrentQueue)

## 2. Public API

```cpp
class LogService : public QObject {
    Q_OBJECT

public:
    explicit LogService(ILogBackend* backend, QObject* parent = nullptr);

    // Logging methods (all non-blocking, < 1μs)
    void debug(const QString& message, const QVariantMap& context = {});
    void info(const QString& message, const QVariantMap& context = {});
    void warning(const QString& message, const QVariantMap& context = {});
    void error(const QString& message, const QVariantMap& context = {});
    void critical(const QString& message, const QVariantMap& context = {});

    // Queue management
    void start();
    void stop();
    void flush();

signals:
    void logEntryWritten(const LogEntry& entry);
    void queueOverflow(int droppedEntries);

private:
    ILogBackend* m_backend;
    TemporaryQueue<LogEntry> m_queue;
    QTimer* m_flushTimer;
};
```

## 3. Key Features

**Non-Blocking:** All log methods return immediately by enqueueing to lock-free queue

**Asynchronous Processing:** Database I/O Thread processes queue and writes to backend

**Queue Overflow:** When queue is full, drops oldest entries and emits queueOverflow() signal

**Backends:** Supports SQLite, file, console backends (ILogBackend interface)

## 4. Usage Example

```cpp
LogService* logService = new LogService(sqliteBackend);
logService->start();

// Non-blocking logging (< 1μs)
logService->info("Patient admitted", {{"mrn", "MRN-12345"}});
logService->error("Database connection failed", {{"error", dbError}});
logService->debug("Vital processed", {{"type", "HR"}, {"value", 75}});

// Flush on shutdown
logService->flush();
logService->stop();
```

## 5. Related Documentation

- ILogBackend Interface - Logging backend contract
- LogEntry Value Object - Log entry structure
- DOC-COMP-018: DatabaseManager (shares Database I/O Thread)

## 6. Changelog

| Version | Date       | Author      | Changes                                 |
| ------- | ---------- | ----------- | --------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from LogService.h |
