---
doc_id: DOC-COMP-021
title: SQLiteAlarmRepository
version: v1.0
category: Component
subcategory: Infrastructure Layer / Persistence
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-COMP-016, DOC-COMP-018, DOC-COMP-010]
related_tasks: [TASK-3C-001]
tags: [infrastructure, repository, alarm, sqlite, persistence]
diagram_files: []
---

# DOC-COMP-021: SQLiteAlarmRepository

## 1. Overview

**Purpose:** SQLite implementation of IAlarmRepository for persisting alarm snapshots with lifecycle tracking (ACTIVE → ACKNOWLEDGED → SILENCED → RESOLVED).

**Responsibilities:**
- Save alarm snapshot to `alarms` table
- Get active alarms (status='ACTIVE')
- Query alarm history by patient and time range
- Find alarm by ID
- Update alarm status (acknowledge, silence, resolve)

**Layer:** Infrastructure Layer

**Module:** `z-monitor/src/infrastructure/persistence/SQLiteAlarmRepository.h` (193 lines)

**Thread Affinity:** Database I/O Thread

**Dependencies:**
- **IDatabaseManager:** Connection management
- **AlarmSnapshot:** Domain value object
- **Query Registry:** SQL queries from QueryCatalog
- **Schema Constants:** Column names from SchemaInfo.h

## 2. Public API

```cpp
class SQLiteAlarmRepository : public QObject, public IAlarmRepository {
    Q_OBJECT

public:
    explicit SQLiteAlarmRepository(std::shared_ptr<IDatabaseManager> dbManager, QObject *parent = nullptr);
    ~SQLiteAlarmRepository() override = default;

    Result<void> save(const AlarmSnapshot &alarm) override;
    std::vector<AlarmSnapshot> getActive() override;
    std::vector<AlarmSnapshot> getHistory(const std::string &patientMrn, int64_t startTimeMs, int64_t endTimeMs) override;
    AlarmSnapshot findById(const std::string &alarmId) override;
    Result<void> updateStatus(const std::string &alarmId, AlarmStatus status, const std::string &userId) override;

private:
    std::shared_ptr<IDatabaseManager> m_dbManager;
};
```

## 3. Performance Targets

- Save: <10ms (prepared statement)
- Active alarms query: <20ms (indexed by status)
- History query: <50ms (indexed by patient_mrn + start_time)
- Status update: <5ms (UPDATE by primary key)

## 4. Usage Example

```cpp
AlarmSnapshot alarm(...);
auto result = alarmRepo->save(alarm);

// Get active alarms
auto activeAlarms = alarmRepo->getActive();

// Update status
auto updateResult = alarmRepo->updateStatus(alarmId, AlarmStatus::ACKNOWLEDGED, "nurse-smith");
```

## 5. Related Documentation

- DOC-COMP-016: IAlarmRepository
- DOC-COMP-005: AlarmAggregate
- DOC-COMP-018: DatabaseManager

## 6. Changelog

| Version | Date       | Author      | Changes                                            |
| ------- | ---------- | ----------- | -------------------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from SQLiteAlarmRepository.h |
