---
doc_id: DOC-COMP-015
title: ITelemetryRepository
version: v1.0
category: Component
subcategory: Application Layer / Repository Interface
status: Draft
owner: Application Layer Team
reviewers: 
  - Architecture Team
last_reviewed: 2025-01-26
next_review: 2026-01-26
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-004  # TelemetryBatch value object
  - DOC-COMP-010  # MonitoringService
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
tags:
  - repository
  - persistence
  - telemetry
  - vitals
diagram_files: []
---

# DOC-COMP-015: ITelemetryRepository

## 1. Overview

**Purpose:** Repository interface for persisting and retrieving TelemetryBatch entities (vitals snapshots buffered for periodic transmission to Central Server), following DDD repository pattern.

**Responsibilities:**
- Save telemetry batch (30-second vitals snapshots)
- Query historical telemetry batches by time range
- Archive old telemetry batches (>30 days)
- Retrieve unsent batches (network failure recovery)
- Mark batches as sent to Central Server

**Layer:** Application Layer (Repository Interface)

**Module:** `z-monitor/src/domain/repositories/ITelemetryRepository.h`

**Implementation:** SQLiteTelemetryRepository (infrastructure layer)

**Thread Affinity:** Database I/O Thread (single writer thread for SQLite thread safety)

## 2. Public API

```cpp
class ITelemetryRepository {
public:
    virtual ~ITelemetryRepository() = default;

    /**
     * @brief Save telemetry batch.
     * @param batch Telemetry batch to save
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> save(const TelemetryBatch& batch) = 0;

    /**
     * @brief Get historical telemetry batches by time range.
     * @param startTime Start of time range
     * @param endTime End of time range
     * @return Result<QList<TelemetryBatch>> Batches in time range or error
     */
    virtual Result<QList<TelemetryBatch>, Error> getHistorical(
        const QDateTime& startTime, const QDateTime& endTime) const = 0;

    /**
     * @brief Archive old telemetry batches (>30 days).
     * @param olderThan Archive batches older than this timestamp
     * @return Result<int, Error> Number of batches archived or error
     */
    virtual Result<int, Error> archive(const QDateTime& olderThan) = 0;

    /**
     * @brief Get unsent batches (network failure recovery).
     * @return Result<QList<TelemetryBatch>> Unsent batches or error
     */
    virtual Result<QList<TelemetryBatch>, Error> getUnsent() const = 0;

    /**
     * @brief Mark batch as sent to Central Server.
     * @param batchId Batch ID to mark as sent
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> markAsSent(const QString& batchId) = 0;
};
```

## 3. Implementation Details

**SQLiteTelemetryRepository Implementation:**
- Database table: `telemetry_batches` (columns: batch_id, patient_mrn, timestamp, vital_snapshots_json, is_sent, created_at)
- Save: Inserts telemetry batch with `is_sent=false`
- getUnsent(): Queries `WHERE is_sent=false` for network recovery
- markAsSent(): Updates `is_sent=true` after successful transmission
- Archive: Moves batches older than 30 days to `telemetry_archive` table

**Error Handling:**
- Uses `Result<T, Error>` type for all operations
- Database errors: Returns `Error::DATABASE_QUERY_FAILED`
- Invalid time range: Returns `Error::INVALID_ARGUMENT`

## 4. Usage Examples

### 4.1 Save Telemetry Batch

```cpp
TelemetryBatch batch = createBatchFromVitals(vitals);
auto result = telemetryRepo->save(batch);
if (result.isSuccess()) {
    qInfo() << "Telemetry batch saved for transmission";
} else {
    qWarning() << "Save failed:" << result.error().message();
}
```

### 4.2 Retrieve Unsent Batches (Network Recovery)

```cpp
auto result = telemetryRepo->getUnsent();
if (result.isSuccess()) {
    for (const auto& batch : result.value()) {
        transmitToCentralServer(batch);
    }
}
```

## 5. Related Documentation

- DOC-COMP-004: TelemetryBatch - Telemetry batch value object
- DOC-COMP-010: MonitoringService - Telemetry batch creation
- SQLiteTelemetryRepository Implementation - Infrastructure layer persistence

## 6. Changelog

| Version | Date       | Author      | Changes                                           |
| ------- | ---------- | ----------- | ------------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from ITelemetryRepository.h |
