---
doc_id: DOC-COMP-020
title: SQLiteTelemetryRepository
version: v1.0
category: Component
subcategory: Infrastructure Layer / Persistence
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-COMP-015, DOC-COMP-018, DOC-COMP-010]
related_tasks: [TASK-3C-001]
tags: [infrastructure, repository, telemetry, sqlite, persistence]
diagram_files: []
---

# DOC-COMP-020: SQLiteTelemetryRepository

## 1. Overview

**Purpose:** SQLite implementation of ITelemetryRepository for persisting telemetry batches (vitals snapshots for Central Server transmission) with transmission tracking.

**Responsibilities:**
- Save telemetry batch to `telemetry_metrics` table (status='retrying')
- Retrieve historical batches by time range
- Archive old batches (>30 days, status='success' only)
- Get unsent batches for network recovery (status != 'success')
- Mark batch as sent (update status='success')

**Layer:** Infrastructure Layer

**Module:** `z-monitor/src/infrastructure/persistence/SQLiteTelemetryRepository.h` (159 lines)

**Thread Affinity:** Database I/O Thread

**Dependencies:**
- **IDatabaseManager:** Connection management
- **TelemetryBatch:** Domain value object
- **Query Registry:** SQL queries (no magic strings)
- **Schema Constants:** Column names (SchemaInfo.h)

## 2. Public API

```cpp
class SQLiteTelemetryRepository : public QObject, public ITelemetryRepository {
    Q_OBJECT

public:
    explicit SQLiteTelemetryRepository(std::shared_ptr<IDatabaseManager> dbManager, QObject *parent = nullptr);
    ~SQLiteTelemetryRepository() override;

    Result<void> save(const TelemetryBatch &batch) override;
    std::vector<std::shared_ptr<TelemetryBatch>> getHistorical(int64_t startTimeMs, int64_t endTimeMs) override;
    size_t archive(int64_t cutoffTimeMs) override;
    std::vector<std::shared_ptr<TelemetryBatch>> getUnsent() override;
    Result<void> markAsSent(const std::string &batchId) override;

private:
    std::shared_ptr<IDatabaseManager> m_dbManager;
};
```

## 3. Performance Targets

- Batch save: <20ms (prepared statement)
- Pending batch retrieval: <50ms (indexed by status)
- Mark as sent: <5ms (UPDATE by primary key)

## 4. Usage Example

```cpp
TelemetryBatch batch = createBatchFromVitals(vitals);
auto result = telemetryRepo->save(batch);

// Network recovery
auto unsent = telemetryRepo->getUnsent();
for (const auto& batch : unsent) {
    transmitToCentralServer(batch);
}
```

## 5. Related Documentation

- DOC-COMP-015: ITelemetryRepository
- DOC-COMP-010: MonitoringService (telemetry batch creation)
- DOC-COMP-018: DatabaseManager

## 6. Changelog

| Version | Date       | Author      | Changes                                                |
| ------- | ---------- | ----------- | ------------------------------------------------------ |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from SQLiteTelemetryRepository.h |
