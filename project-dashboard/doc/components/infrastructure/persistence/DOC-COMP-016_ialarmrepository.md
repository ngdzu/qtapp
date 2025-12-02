---
doc_id: DOC-COMP-016
title: IAlarmRepository
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
  - DOC-COMP-005  # AlarmAggregate
  - DOC-COMP-010  # MonitoringService
related_tasks:
  - TASK-3B-001  # Phase 3B Migration
tags:
  - repository
  - persistence
  - alarm
  - safety
diagram_files: []
---

# DOC-COMP-016: IAlarmRepository

## 1. Overview

**Purpose:** Repository interface for persisting and retrieving AlarmAggregate domain entities (clinical alarm snapshots with lifecycle management), following DDD repository pattern.

**Responsibilities:**
- Save alarm aggregate (new alarm or status update)
- Retrieve active alarms (AlarmState::ACTIVE or AlarmState::ACKNOWLEDGED)
- Query alarm history by patient and time range
- Find specific alarm by ID
- Update alarm status (acknowledge, silence, resolve)

**Layer:** Application Layer (Repository Interface)

**Module:** `z-monitor/src/domain/repositories/IAlarmRepository.h`

**Implementation:** SQLiteAlarmRepository (infrastructure layer)

**Thread Affinity:** Database I/O Thread (single writer thread for SQLite thread safety)

## 2. Public API

```cpp
class IAlarmRepository {
public:
    virtual ~IAlarmRepository() = default;

    /**
     * @brief Save alarm aggregate (insert or update).
     * @param alarm Alarm aggregate to save
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> save(std::shared_ptr<AlarmAggregate> alarm) = 0;

    /**
     * @brief Get active alarms (ACTIVE or ACKNOWLEDGED states).
     * @return Result<QList<std::shared_ptr<AlarmAggregate>>> Active alarms or error
     */
    virtual Result<QList<std::shared_ptr<AlarmAggregate>>, Error> getActive() const = 0;

    /**
     * @brief Get alarm history by patient and time range.
     * @param patientMrn Patient MRN (empty for all patients)
     * @param startTime Start of time range
     * @param endTime End of time range
     * @return Result<QList<std::shared_ptr<AlarmAggregate>>> Alarm history or error
     */
    virtual Result<QList<std::shared_ptr<AlarmAggregate>>, Error> getHistory(
        const QString& patientMrn,
        const QDateTime& startTime,
        const QDateTime& endTime) const = 0;

    /**
     * @brief Find alarm by ID.
     * @param alarmId Alarm unique identifier
     * @return Result<std::shared_ptr<AlarmAggregate>> Alarm or error
     */
    virtual Result<std::shared_ptr<AlarmAggregate>, Error> findById(const QString& alarmId) const = 0;

    /**
     * @brief Update alarm status.
     * @param alarmId Alarm ID to update
     * @param newState New alarm state (ACKNOWLEDGED, SILENCED, RESOLVED)
     * @return Result<void, Error> Success or error
     */
    virtual Result<void, Error> updateStatus(const QString& alarmId, AlarmState newState) = 0;
};
```

## 3. Implementation Details

**SQLiteAlarmRepository Implementation:**
- Database table: `alarms` (columns: alarm_id, patient_mrn, alarm_type, alarm_severity, alarm_state, triggered_at, acknowledged_at, resolved_at, vital_snapshot_json, created_at, updated_at)
- Save: Uses UPSERT (`INSERT OR REPLACE`) for atomic save
- getActive(): Queries `WHERE alarm_state IN ('ACTIVE', 'ACKNOWLEDGED')`
- getHistory(): Queries with patient MRN and time range filters
- updateStatus(): Updates `alarm_state` and timestamps (acknowledged_at, resolved_at)

**Error Handling:**
- Uses `Result<T, Error>` type for all operations
- Database errors: Returns `Error::DATABASE_QUERY_FAILED`
- Alarm not found: Returns `Error::ALARM_NOT_FOUND`

## 4. Usage Examples

### 4.1 Save Alarm

```cpp
auto alarm = std::make_shared<AlarmAggregate>(
    alarmId, patientMrn, AlarmType::HEART_RATE_HIGH, AlarmSeverity::HIGH, vitalSnapshot);

auto result = alarmRepo->save(alarm);
if (result.isSuccess()) {
    qInfo() << "Alarm saved";
} else {
    qWarning() << "Save failed:" << result.error().message();
}
```

### 4.2 Get Active Alarms

```cpp
auto result = alarmRepo->getActive();
if (result.isSuccess()) {
    for (const auto& alarm : result.value()) {
        displayAlarm(alarm);
    }
}
```

### 4.3 Update Alarm Status (Acknowledge)

```cpp
auto result = alarmRepo->updateStatus(alarmId, AlarmState::ACKNOWLEDGED);
if (result.isSuccess()) {
    qInfo() << "Alarm acknowledged";
}
```

## 5. Related Documentation

- DOC-COMP-005: AlarmAggregate - Alarm domain entity
- DOC-COMP-010: MonitoringService - Alarm creation and lifecycle
- SQLiteAlarmRepository Implementation - Infrastructure layer persistence

## 6. Changelog

| Version | Date       | Author      | Changes                                       |
| ------- | ---------- | ----------- | --------------------------------------------- |
| v1.0    | 2025-01-26 | Dustin Wind | Initial documentation from IAlarmRepository.h |
