# Database Module: Class Designs

**Document ID:** DESIGN-009d  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Database Module**, which handles data persistence and archival on the Database I/O Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture (Section 4.4: Database I/O Thread)
> - [Database Design (10_DATABASE_DESIGN.md)](./10_DATABASE_DESIGN.md) - Database schema
> - [Database Access Strategy (30_DATABASE_ACCESS_STRATEGY.md)](./30_DATABASE_ACCESS_STRATEGY.md) - Repository pattern and ORM
> - [Schema Management (33_SCHEMA_MANAGEMENT.md)](./33_SCHEMA_MANAGEMENT.md) - Schema management workflow

---

## 1. Module Overview

**Thread:** Database I/O Thread (Single Writer)  
**Priority:** I/O Priority (background, but responsive)  
**Component Count:** 13 components

**Purpose:**
- Persist data to encrypted SQLite database (SQLCipher)
- Batch writes for performance (non-critical path)
- Periodic persistence from in-memory cache (every 10 minutes)
- Daily cleanup per retention policies
- Schema migrations and management

**Non-Critical Path:** Database operations don't block alarm detection

---

## 2. Module Diagram

[View Database Module Diagram (Mermaid)](./09d_DATABASE_MODULE.mmd)  
[View Database Module Diagram (SVG)](./09d_DATABASE_MODULE.svg)

---

## 3. Core Infrastructure

### 3.1. DatabaseManager

**Responsibility:** SQLite connection management, migrations, schema management, database size monitoring.

**Thread:** Database I/O Thread

**Key Properties:**
- `database`: `QSqlDatabase` - SQLite database instance with SQLCipher encryption
- `encryptionKey`: `QByteArray` - Database encryption key (loaded from secure storage)
- `backupLocation`: `QString` - Location for database backups

**Key Methods:**
- `open(const QString& path, const QString& password)`: Opens encrypted database
- `saveData(const PatientData& data)`: Stores patient data
- `getHistoricalData(QDateTime start, QDateTime end)`: Retrieves historical data for trends
- `cleanupOldData(int retentionDays)`: Removes data older than retention policy
- `backupDatabase(const QString& backupPath)`: Creates encrypted backup of database
- `restoreDatabase(const QString& backupPath, const QString& password)`: Restores database from backup
- `rotateEncryptionKey(const QString& newKey)`: Rotates database encryption key (re-encrypts all data)
- `verifyDataIntegrity()`: Verifies database integrity using checksums
- `getBackupList()`: Returns list of available backups with timestamps

**Database Size Monitoring:**
- `getDatabaseSize()`: Returns current database size in bytes (cached, refreshed every 60 seconds)
- `getDatabaseSizeMB()`: Returns database size in megabytes
- `checkDatabaseSizeLimit()`: Checks if database exceeds size limits (500 MB limit, 400 MB warning, 450 MB critical)
- `getTableSizes()`: Returns size breakdown by table
- `estimateDaysUntilFull()`: Estimates days until database reaches limit
- `monitorDatabaseSize()`: Periodic size monitoring (every 60 seconds)

**Signals:**
- `databaseSizeWarning(double sizeMB, int daysUntilFull)`: Emitted when database exceeds 80% of limit (400 MB)
- `databaseSizeCritical(double sizeMB, int daysUntilFull)`: Emitted when database exceeds 90% of limit (450 MB)
- `databaseSizeExceeded(double sizeMB)`: Emitted when database exceeds 100% of limit (500 MB)
- `databaseSizeNormal()`: Emitted when database size returns to normal (< 80%)

**Dependencies:**
- `KeyManager` - Database encryption key (Background Module)
- `SchemaInfo` - Generated schema constants (from `schema/database.yaml`)

**See:** [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) for complete database schema.

---

## 4. Repository Implementations

All repository implementations run on the Database I/O Thread and use the same `DatabaseManager` instance.

### 4.1. SQLitePatientRepository

**Responsibility:** Patient data persistence.

**Interface:** `IPatientRepository`

**Key Methods:**
- `findByMrn(const QString& mrn)`: Finds patient by MRN
- `save(const PatientAggregate& patient)`: Saves patient data
- `getAdmissionHistory(const QString& mrn)`: Retrieves admission/discharge history

**Technology:** SQLite + SQLCipher (ORM or Manual Qt SQL with Schema constants)

---

### 4.2. SQLiteVitalsRepository

**Responsibility:** Vitals time-series persistence (periodic, non-critical).

**Interface:** `IVitalsRepository`

**Key Methods:**
- `saveBatch(const QList<VitalRecord>& vitals)`: Batch saves vitals (called by `PersistenceScheduler`)
- `getRange(const QDateTime& startTime, const QDateTime& endTime)`: Retrieves vitals in time range
- `getUnsynced()`: Retrieves vitals not yet synced to server

**Technology:** Manual Qt SQL (for time-series performance)

**See:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) for caching architecture.

---

### 4.3. SQLiteAlarmRepository

**Responsibility:** Alarm history persistence.

**Interface:** `IAlarmRepository`

**Key Methods:**
- `save(const AlarmAggregate& alarm)`: Saves alarm to history
- `getActive()`: Retrieves active alarms (from database, not in-memory cache)
- `getHistory(const QString& patientMrn, const QDateTime& startTime, const QDateTime& endTime)`: Retrieves alarm history

---

### 4.4. SQLiteTelemetryRepository

**Responsibility:** Telemetry data persistence.

**Interface:** `ITelemetryRepository`

**Key Methods:**
- `save(const TelemetryBatch& batch)`: Saves telemetry batch
- `getHistorical(const QDateTime& startTime, const QDateTime& endTime)`: Retrieves historical telemetry
- `archive(const QDateTime& beforeDate)`: Archives old telemetry data

---

### 4.5. SQLiteProvisioningRepository

**Responsibility:** Provisioning state persistence.

**Interface:** `IProvisioningRepository`

**Key Methods:**
- `save(const ProvisioningSession& session)`: Saves provisioning state
- `findByDeviceId(const QString& deviceId)`: Finds provisioning session by device ID
- `getHistory()`: Retrieves provisioning history

---

### 4.6. SQLiteUserRepository

**Responsibility:** User account persistence.

**Interface:** `IUserRepository`

**Key Methods:**
- `findByUserId(const QString& userId)`: Finds user by ID
- `save(const UserSession& user)`: Saves user data
- `updateLastLogin(const QString& userId)`: Updates last login timestamp

---

### 4.7. SQLiteAuditRepository

**Responsibility:** Security audit log persistence.

**Interface:** `IAuditRepository`

**Key Methods:**
- `logAuditEvent(const AuditEntry& entry)`: Logs security audit event (batch write)
- `queryAuditEvents(const AuditFilter& filter)`: Queries audit log entries
- `getAuditEventCount(const QString& eventType, const QDateTime& startTime, const QDateTime& endTime)`: Gets event count

**Implementation Details:**
- Batch writes for performance (every 10 seconds or 100 records)
- Hash chain for tamper detection (`previous_hash` column)
- Append-only (immutable audit trail)

**See:** [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) for logging strategy.

---

### 4.8. SQLiteActionLogRepository

**Responsibility:** Action log persistence (user actions).

**Interface:** `IActionLogRepository`

**Key Methods:**
- `logAction(const ActionLogEntry& entry)`: Logs user action (batch write)
- `queryActions(const ActionLogFilter& filter)`: Queries action log entries
- `getActionCount(const QString& userId, const QDateTime& startTime, const QDateTime& endTime)`: Gets action count

**Implementation Details:**
- Batch writes for performance (every 10 seconds or 100 records)
- Hash chain for tamper detection (`previous_hash` column)

**See:** [39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md](./39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md) for action logging workflow.

---

## 5. Application Services

### 5.1. PersistenceScheduler

**Responsibility:** Periodic persistence of in-memory cache (every 10 minutes or 10,000 records).

**Thread:** Database I/O Thread

**Key Methods:**
- `schedulePersistence()`: Schedules periodic persistence (every 10 minutes)
- `persistFromCache()`: Reads data from `VitalsCache` (RT Thread) and persists to database
  - **Non-Critical:** Database writes don't block alarm detection
  - **Latency Target:** < 5 seconds (background task)

**Dependencies:**
- `VitalsCache` - Source of vitals data (RT Thread, accessed via thread-safe interface)
- `SQLiteVitalsRepository` - Vitals persistence

**See:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) for caching architecture.

---

### 5.2. DataCleanupService

**Responsibility:** Daily data cleanup (7-day retention, runs at 3 AM).

**Thread:** Database I/O Thread

**Key Methods:**
- `scheduleCleanup()`: Schedules daily cleanup (3 AM)
- `cleanupOldData()`: Removes data older than retention policies
  - Vitals: 7 days
  - Alarms: 90 days
  - Action logs: 90 days
  - Security audit logs: 90 days
- `performEmergencyCleanup()`: Emergency cleanup when database exceeds 500 MB limit
  - Deletes vitals older than 3 days (instead of 7)
  - Archives resolved alarms older than 7 days
  - Compacts database (SQLite VACUUM)

**Dependencies:**
- All repositories - For data deletion
- `DatabaseManager` - For database compaction

---

### 5.3. DataArchiveService

**Responsibility:** Archives old data per retention policies.

**Thread:** Database I/O Thread

**Key Methods:**
- `archiveData(const QList<PatientData>& data)`: Archives old data
- `getArchivedData(const QDateTime& startTime, const QDateTime& endTime)`: Retrieves archived data

**Signals:**
- `dataArchived(int recordCount)`: Emitted when archival completes

---

### 5.4. LogService

**Responsibility:** Centralized logging mechanism for application events (file-based).

**Thread:** Database I/O Thread (shared with database operations)

**Key Methods:**
- `log(LogLevel level, const QString& message, const QVariantMap& context = {})`: Logs message
- `getRecentLogs(int count = 1000)`: Returns recent log entries for Diagnostics View

**Storage:**
- File-based (rotated text/JSON files)
- Not database-based (separate from action logs and audit logs)

**See:** [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) for complete logging strategy.

---

## 6. Module Communication

### 6.1. Inbound (From Other Modules)

**From Real-Time Processing Module (RT Thread):**
- `MPSC Queue` for telemetry batches (non-critical, background persistence)

**From Application Services Module (App Services Thread):**
- `MPSC Queue` for repository calls (patient data, provisioning state, audit logs)

**From Interface Module (UI Thread):**
- `Qt::QueuedConnection` calls for trend data queries

### 6.2. Outbound (To Other Modules)

**To Interface Module (UI Thread):**
- `Qt::QueuedConnection` signals for query completion (trend data loaded)

**To Real-Time Processing Module (RT Thread):**
- `VitalsCache` accessed via thread-safe interface (read-only for persistence)

---

## 7. Performance Requirements

**Latency Targets (Non-Critical):**
- Background batch persistence (10 min schedule): < 5 seconds
- Daily cleanup (3 AM): < 30 seconds
- Query operations (trends): < 2 seconds for 24-hour window

**Database Optimization:**
- WAL mode enabled (Write-Ahead Logging)
- Prepared statements with query caching
- Batch writes (10 records per transaction)
- Time-series indices for vitals table

---

## 8. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.4: Database I/O Thread)
- **[10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md)** - Database schema
- **[30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md)** - Repository pattern and ORM
- **[33_SCHEMA_MANAGEMENT.md](./33_SCHEMA_MANAGEMENT.md)** - Schema management workflow
- **[36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md)** - Caching architecture

---

*This document provides detailed class designs for the Database Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*

