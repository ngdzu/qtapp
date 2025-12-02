---
doc_id: DOC-ARCH-017
title: Database Design and Schema
version: 1.0
category: Architecture
status: Approved
related_docs:
  - DOC-ARCH-018  # SQLCipher Integration
  - DOC-GUIDE-014 # Database Access Strategy
  - DOC-GUIDE-015 # Data Transfer Objects
  - DOC-COMP-032  # Query Registry
  - DOC-PROC-009  # Schema Management
  - DOC-PROC-010  # Data Migration Workflow
  - DOC-ARCH-001  # Software Architecture
  - DOC-ARCH-010  # Security Architecture
tags:
  - database
  - schema
  - sqlite
  - data-model
  - persistence
  - hipaa
source:
  original_id: DESIGN-010
  file: 10_DATABASE_DESIGN.md
  migrated_date: 2025-12-01
---

# Database Design and Schema

## Purpose

This document outlines the design of the local SQLite database used by the Z Monitor application. It includes required tables, schema enhancements, indices, retention and archival policy, encryption and key management guidance, API payload extensions, and testing/sizing recommendations.

The database serves as the local persistence layer for patient data, vital signs, alarms, telemetry metrics, audit logs, and device configuration. It is designed to meet the functional, safety, audit, and security requirements of the Z Monitor medical device.

**Note:** The actual database schema is defined in `schema/database.yaml` (single source of truth). This document provides detailed explanations and design rationale. For the complete schema definition, see `schema/database.yaml`. For migration workflow, see DOC-PROC-010 (Data Migration Workflow).

## Database Architecture

### Technology Stack

- **Database Engine:** SQLite 3.x
- **Encryption:** SQLCipher (AES-256) for HIPAA compliance (see DOC-ARCH-018)
- **Schema Management:** YAML-based schema definition with code generation (see DOC-PROC-009)
- **Query Management:** Query Registry pattern for type-safe SQL (see DOC-COMP-032)
- **Access Pattern:** Repository pattern with dependency injection (see DOC-GUIDE-014)

### Design Principles

1. **Single Source of Truth:** Schema defined in `schema/database.yaml`, code generated from it
2. **HIPAA Compliance:** AES-256 encryption for PHI (Protected Health Information)
3. **Performance Optimization:** Indices for critical queries, retention policies for time-series data
4. **Data Integrity:** Foreign keys, check constraints, transaction management
5. **Auditability:** Immutable audit logs with hash chains for tamper detection
6. **Type Safety:** Schema constants, query registry, DTOs for compile-time validation

## Core Tables

### Patient Management

#### `patients` Table

Stores patient demographic information and admission status. Serves as both local patient registry and cache for external HIS/EHR data.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS patients (
    patient_id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    dob TEXT NULL,
    sex TEXT NULL,
    mrn TEXT NULL,                       -- Medical Record Number
    allergies TEXT NULL,
    room TEXT NULL,                      -- DEPRECATED: Use bed_location
    created_at INTEGER NOT NULL,
    last_lookup_at INTEGER NULL,
    lookup_source TEXT NULL,
    -- ADT Workflow columns
    bed_location TEXT NULL,              -- Current bed/room (e.g., "ICU-4B")
    admitted_at INTEGER NULL,            -- Admission timestamp (NULL if not admitted)
    discharged_at INTEGER NULL,          -- Discharge timestamp (NULL if admitted)
    admission_source TEXT NULL,          -- "manual", "barcode", "central_station"
    device_label TEXT NULL               -- Device identifier (e.g., "ICU-MON-04")
);
```

**Usage Notes:**
- `patient_id`: Primary identifier (typically MRN)
- `mrn`: Medical Record Number - primary hospital identifier
- `bed_location`: Current bed/room assignment - part of Patient object, managed through ADT workflow
- `admitted_at`/`discharged_at`: Admission lifecycle timestamps
- `last_lookup_at`: Cache timestamp for HIS/EHR lookups
- `lookup_source`: Audit trail for data source ("local", "his", "ehr")

**ADT Workflow:**
The `patients` table includes ADT (Admission, Discharge, Transfer) workflow columns for proper patient lifecycle management. ADT workflow ensures:
1. Patient admission with bed location and device association
2. Discharge workflow with timestamp and reason
3. Transfer workflow to different device/bed
4. Audit trail of all patient movements

**Lookup Flow:**
1. `PatientManager::loadPatientById(id)` checks local `patients` table first
2. If found locally, patient data returned immediately (cache hit)
3. If not found, `IPatientLookupService` queries external system (HIS/EHR)
4. On successful lookup, patient data saved to `patients` table with `last_lookup_at` timestamp
5. Caching reduces external system load and enables offline operation

**Indices:**
```sql
CREATE UNIQUE INDEX idx_patients_mrn ON patients(mrn);
CREATE INDEX idx_patients_admission_status ON patients(admission_status);
CREATE INDEX idx_patients_bed_location ON patients(bed_location) 
    WHERE bed_location IS NOT NULL;
```

**Critical Requirements:**
- **Patient Association:** All clinical data (vitals, alarms) MUST reference `patient_mrn`
- **Standby State:** If no patient admitted, vitals should not be recorded (device in STANDBY)
- **Data Integrity:** Patient MRN must be valid and associated with admitted patient

### Time-Series Data

#### `vitals` Table

Stores physiological time-series data (heart rate, SpO2, respiration rate, etc.). Expected to be the largest table with high write throughput.

**Schema (Core Columns):**
```sql
CREATE TABLE IF NOT EXISTS vitals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NULL,
    timestamp INTEGER NOT NULL,           -- Time data was measured (Unix ms)
    timestamp_iso TEXT NULL,
    patient_id TEXT NOT NULL,             -- DEPRECATED: Use patient_mrn
    patient_mrn TEXT NOT NULL,            -- Medical Record Number (REQUIRED)
    device_id TEXT NULL,
    device_label TEXT NULL,               -- Device asset tag
    heart_rate REAL NULL,
    spo2 REAL NULL,
    respiration_rate REAL NULL,
    signal_quality INTEGER NULL,
    sample_rate_hz REAL NULL,
    source TEXT NULL,
    is_synced BOOLEAN DEFAULT 0,
    batch_id TEXT NULL                    -- FK to telemetry_metrics.batch_id
);
```

**Critical Requirements:**
- **Patient Association:** `patient_mrn` is REQUIRED and must not be NULL
- **Data Integrity:** Both `patient_id` and `patient_mrn` stored for migration compatibility
- **Performance:** No timing data in this table (kept in `telemetry_metrics` for efficiency)

**Design Decision: No Timing Data in `vitals` Table**

**Rationale:**
- **Performance:** `vitals` is the **largest, hottest table** with time-series data at 100-500 Hz. Adding timing columns would slow down critical write path and impact alarm latency
- **Storage Efficiency:** Most vitals never transmitted individually (batched). Adding timing columns wastes storage on 90% of data
- **Separation of Concerns:** Clinical data (`vitals`) vs. diagnostic/performance data (timing metrics) should be separate
- **Query Performance:** Timing columns add index overhead and slow down clinical queries

**Instead: Use `telemetry_metrics` Table for Timing**

Timing and latency tracking done in dedicated `telemetry_metrics` table. To correlate vitals with timing:
- Each transmitted batch gets a `batch_id` (UUID)
- Vitals in that batch linked via `batch_id` column (optional, nullable)
- Query timing for specific vital: `JOIN vitals ON telemetry_metrics.batch_id = vitals.batch_id`

**Example: Correlating Vitals with Timing:**
```sql
-- Find vitals from slow batches (> 100ms)
SELECT v.*, tm.end_to_end_latency_ms, tm.batch_creation_latency_ms
FROM vitals v
INNER JOIN telemetry_metrics tm ON v.batch_id = tm.batch_id
WHERE tm.end_to_end_latency_ms > 100
ORDER BY tm.end_to_end_latency_ms DESC;
```

**Benefits:**
1. ✅ **Fast Writes:** No timing overhead on critical vitals insertion
2. ✅ **Storage Efficient:** Timing data only for transmitted batches (~10% of vitals)
3. ✅ **Clean Separation:** Clinical data separate from diagnostics
4. ✅ **Better Queries:** Clinical queries unaffected by timing columns
5. ✅ **Flexible:** Can add more timing metrics without touching `vitals` schema

**Indices:**
```sql
CREATE INDEX idx_vitals_patient_time ON vitals(patient_id, timestamp);
CREATE INDEX idx_vitals_mrn_time ON vitals(patient_mrn, timestamp);  -- Primary lookup
CREATE INDEX idx_vitals_is_synced ON vitals(is_synced);
CREATE INDEX idx_vitals_device ON vitals(device_id, timestamp);
CREATE INDEX idx_vitals_batch_id ON vitals(batch_id) WHERE batch_id IS NOT NULL;
```

**Retention Policy:**
- **Local Storage:** 7 days rolling window
- **Deletion:** Daily cleanup at 3 AM via `DataCleanupService`
- **Archival:** Old data archived to external storage before deletion (optional)

### Alarm Management

#### `alarms` Table

Stores alarm event history and lifecycle metadata.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS alarms (
    alarm_id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id TEXT NOT NULL,
    patient_mrn TEXT NOT NULL,            -- REQUIRED for patient association
    start_time INTEGER NOT NULL,
    end_time INTEGER NULL,
    alarm_type TEXT NOT NULL,
    priority TEXT NOT NULL,
    status TEXT NULL,
    acknowledged_by TEXT NULL,
    acknowledged_time INTEGER NULL,
    silenced_until INTEGER NULL,
    raw_value REAL NULL,
    threshold_value REAL NULL,            -- Threshold at alarm time (historical)
    context_snapshot_id INTEGER NULL
);
```

**Critical Requirements:**
- **Patient Association:** `patient_mrn` is REQUIRED and must not be NULL
- **Threshold Storage:** `threshold_value` stores threshold that was exceeded *at time of alarm* for historical/audit purposes (immutable snapshot)
- **Current Thresholds:** Stored in `settings` table as `alarm_thresholds_{patientMrn}` (mutable configuration)

**Alarm Threshold Storage Clarification:**
- **Settings table** (`alarm_thresholds_{patientMrn}` key): Stores *currently configured* alarm thresholds per patient (updated when clinician changes thresholds)
- **Alarms table** (`threshold_value` column): Stores *historical snapshot* of threshold value exceeded at alarm time (immutable for audit/compliance)

**Indices:**
```sql
CREATE INDEX idx_alarms_patient_priority ON alarms(patient_id, priority, start_time);
```

### Diagnostics and Metrics

#### `telemetry_metrics` Table

Stores telemetry transmission timing and performance metrics for benchmarking and diagnostics.

**Purpose:** Track end-to-end latency of telemetry batch transmission from data creation to server acknowledgment.

**Schema (Key Columns):**
```sql
CREATE TABLE IF NOT EXISTS telemetry_metrics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    batch_id TEXT NOT NULL UNIQUE,        -- Batch UUID
    device_id TEXT NOT NULL,
    device_label TEXT NULL,
    patient_mrn TEXT NULL,                -- NULL if no patient admitted
    
    -- Timing milestones (Unix milliseconds)
    data_created_at INTEGER NOT NULL,     -- Oldest data point in batch
    batch_created_at INTEGER NOT NULL,    -- Batch object creation
    signed_at INTEGER NULL,               -- Digital signature timestamp
    queued_for_tx_at INTEGER NULL,        -- Queued for transmission
    transmitted_at INTEGER NULL,          -- Transmission started
    server_received_at INTEGER NULL,      -- Server received batch
    server_processed_at INTEGER NULL,     -- Server processed batch
    server_ack_at INTEGER NULL,           -- Server acknowledgment
    
    -- Computed latencies (milliseconds)
    batch_creation_latency_ms INTEGER NULL,
    signing_latency_ms INTEGER NULL,
    queue_wait_latency_ms INTEGER NULL,
    network_latency_ms INTEGER NULL,
    server_processing_latency_ms INTEGER NULL,
    end_to_end_latency_ms INTEGER NULL,
    
    -- Statistics
    record_count INTEGER NULL,
    batch_size_bytes INTEGER NULL,
    compressed_size_bytes INTEGER NULL,
    
    -- Status
    status TEXT NOT NULL,                 -- "success", "failed", "timeout", "retrying"
    error_message TEXT NULL,
    retry_count INTEGER DEFAULT 0,
    
    created_at INTEGER NOT NULL
);
```

**Usage:** Correlate with `vitals` table via `batch_id` to analyze timing for specific data points.

**Indices:**
```sql
CREATE INDEX idx_telemetry_batch_id ON telemetry_metrics(batch_id);
CREATE INDEX idx_telemetry_device ON telemetry_metrics(device_id);
CREATE INDEX idx_telemetry_status ON telemetry_metrics(status);
CREATE INDEX idx_telemetry_created ON telemetry_metrics(created_at);
```

**Retention Policy:** 30 days

### Audit and Security

#### `action_log` Table

Immutable audit trail for all user actions (login, logout, patient management, settings changes). Enhanced with hash chain for tamper detection.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,        -- Unix timestamp (milliseconds)
    timestamp_iso TEXT NOT NULL,          -- ISO 8601 for readability
    user_id TEXT NULL,                    -- User who performed action
    user_role TEXT NULL,                  -- User role (NURSE, PHYSICIAN, etc.)
    action_type TEXT NOT NULL,            -- Action type (LOGIN, LOGOUT, etc.)
    target_type TEXT NULL,                -- Target type (PATIENT, SETTING, etc.)
    target_id TEXT NULL,                  -- Target identifier (MRN, setting name)
    details TEXT NULL,                    -- JSON with additional context
    result TEXT NOT NULL,                 -- SUCCESS, FAILURE, PARTIAL
    error_code TEXT NULL,
    error_message TEXT NULL,
    device_id TEXT NOT NULL,
    session_token_hash TEXT NULL,         -- SHA-256 hash of session token
    ip_address TEXT NULL,
    previous_hash TEXT NULL               -- SHA-256 hash of previous entry (hash chain)
);
```

**Hash Chain for Tamper Detection:**
1. **First Entry:** `previous_hash` is NULL (genesis entry)
2. **Subsequent Entries:** `previous_hash` = SHA-256(previous entry's: id || timestamp_ms || action_type || user_id || target_id || details || result)
3. **Validation:** Recompute hash chain from beginning to detect tampering

**Action Types:**
- `LOGIN`, `LOGIN_FAILED`, `LOGOUT`, `AUTO_LOGOUT`, `SESSION_EXPIRED`
- `ADMIT_PATIENT`, `DISCHARGE_PATIENT`, `TRANSFER_PATIENT`
- `CHANGE_SETTING`, `ADJUST_ALARM_THRESHOLD`
- `CLEAR_NOTIFICATIONS`, `DISMISS_NOTIFICATION`
- `VIEW_AUDIT_LOG`, `EXPORT_DATA`, `ACCESS_DIAGNOSTICS`
- `PROVISIONING_MODE_ENTERED`

**Retention Policy:** 90 days minimum (configurable)

**Note:** Security-related events (authentication failures, certificate operations, network connections) logged in `security_audit_log` table for categorization.

**Indices:**
```sql
CREATE INDEX idx_action_log_timestamp ON action_log(timestamp_ms DESC);
CREATE INDEX idx_action_log_user ON action_log(user_id, timestamp_ms DESC);
CREATE INDEX idx_action_log_action_type ON action_log(action_type, timestamp_ms DESC);
CREATE INDEX idx_action_log_target ON action_log(target_type, target_id, timestamp_ms DESC);
CREATE INDEX idx_action_log_device ON action_log(device_id, timestamp_ms DESC);
```

#### `security_audit_log` Table

Stores security-relevant events for audit, forensics, and compliance. Immutable audit trail with hash chain.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    severity TEXT NOT NULL,
    device_id TEXT NULL,
    user_id TEXT NULL,
    source_ip TEXT NULL,
    event_category TEXT NOT NULL,
    success BOOLEAN NOT NULL,
    details TEXT NULL,
    error_code TEXT NULL,
    error_message TEXT NULL,
    previous_hash TEXT NULL               -- Hash chain for tamper detection
);
```

**Event Categories:**
- `authentication`: Login attempts, certificate validation, token operations
- `connection`: mTLS handshakes, connection establishment, disconnections
- `data_transmission`: Telemetry sends, sensor data sends, transmission failures
- `certificate`: Certificate validation, renewal, revocation events
- `authorization`: Access control violations, permission checks
- `security_config`: Security setting changes, certificate updates

**Severity Levels:**
- `info`: Informational events (successful operations)
- `warning`: Suspicious but non-critical events (rate limit warnings)

**Hash Chain:** Same mechanism as `action_log` (REQ-SEC-AUDIT-002)

### Configuration and Settings

#### `settings` Table

Stores device configuration settings and user preferences.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at INTEGER NOT NULL,
    updated_by TEXT NULL
);
```

**Recommended Default Settings:**
- `deviceId`: Unique device identifier for telemetry (e.g., "ZM-001")
- `deviceLabel`: Static device asset tag (e.g., "ICU-MON-04") - fixed technical identifier
- `measurementUnit`: "metric" or "imperial"
- `serverUrl`: Central server URL (e.g., "https://monitoring.hospital.com:8443", default: "https://localhost:8443")
- `useMockServer`: Boolean flag ("true" or "false", default: "false")
- `alarm_thresholds_{patientMrn}`: JSON object storing configured alarm thresholds per patient

**Note:** `bedId` setting removed - bed location now part of Patient object, managed through ADT workflow.

#### `certificates` Table

Certificate metadata for installed device/server certs (no private keys stored). Tracks certificate lifecycle, validation, and revocation status.

**Schema:**
```sql
CREATE TABLE IF NOT EXISTS certificates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    cert_serial TEXT NOT NULL,
    cert_subject TEXT NULL,
    cert_issuer TEXT NULL,
    issued_at INTEGER NULL,
    expires_at INTEGER NULL,
    status TEXT NOT NULL,                 -- "active", "expired", "revoked", "pending_renewal"
    last_validated_at INTEGER NULL,
    revocation_reason TEXT NULL,
    revoked_at INTEGER NULL,
    cert_fingerprint TEXT NULL,
    created_at INTEGER NOT NULL
);
```

**Indices:**
```sql
CREATE INDEX idx_certificates_device_status ON certificates(device_id, status);
CREATE INDEX idx_certificates_expires ON certificates(expires_at) WHERE status = 'active';
```

### Additional Tables

Other tables include:
- `snapshots`: Waveform/contextual snapshot data (blobs)
- `annotations`: User-added notes for snapshots
- `infusion_events`: Infusion pump logging
- `device_events`: Device health and telemetry events
- `notifications`: UI notification history
- `predictive_scores`: Predictive analytics outputs
- `users`: Local device accounts (PIN-based, for RBAC)

See `schema/database.yaml` for complete schema definitions.

## Data Integrity and Constraints

### Foreign Keys

- `vitals.patient_mrn` → `patients.mrn` (CASCADE on delete)
- `vitals.batch_id` → `telemetry_metrics.batch_id` (SET NULL on delete)
- `alarms.patient_mrn` → `patients.mrn` (CASCADE on delete)

### Check Constraints

- `patients.sex IN ('M', 'F', 'O', 'U')`
- `vitals.heart_rate >= 0 AND heart_rate <= 300`
- `vitals.spo2 >= 0 AND spo2 <= 100`
- `telemetry_metrics.status IN ('success', 'failed', 'timeout', 'retrying')`

### Unique Constraints

- `patients.mrn` (UNIQUE)
- `telemetry_metrics.batch_id` (UNIQUE)
- `certificates.cert_serial` (UNIQUE)

## Performance Optimization

### Indexing Strategy

1. **Primary Keys:** All tables have primary keys (auto-increment or natural key)
2. **Foreign Keys:** Indexed for join performance
3. **Time-Series Queries:** Composite indices on `(patient_mrn, timestamp)`
4. **Partial Indices:** WHERE clauses for filtered queries (e.g., `is_synced = 0`)
5. **Covering Indices:** Include commonly queried columns

### Retention Policies

| Table                | Retention Period | Cleanup Schedule | Archival              |
| -------------------- | ---------------- | ---------------- | --------------------- |
| `vitals`             | 7 days           | Daily at 3 AM    | Optional              |
| `telemetry_metrics`  | 30 days          | Daily at 3 AM    | Recommended           |
| `action_log`         | 90 days          | Weekly           | Required (compliance) |
| `security_audit_log` | 90 days          | Weekly           | Required (compliance) |
| `alarms`             | 30 days          | Weekly           | Recommended           |

### Query Optimization

- Use prepared statements (via Query Registry - see DOC-COMP-032)
- Batch inserts for vitals (10k records per transaction)
- Read-only connections for reporting queries
- Write connection isolation for data integrity
- Connection pooling for concurrent access

## Security and Encryption

### SQLCipher Integration

- **Encryption:** AES-256 encryption for all database files (see DOC-ARCH-018)
- **Key Management:** Keys derived from device certificate or hardware ID
- **Compliance:** HIPAA-compliant encryption at rest (REQ-SEC-ENC-001)

### Access Control

- **Authentication:** PIN-based authentication for local users
- **Authorization:** Role-based access control (RBAC) via `users` table
- **Audit Trail:** All access logged in `action_log` and `security_audit_log`

### Data Protection

- **PHI Protection:** All patient data encrypted at rest and in transit
- **Tamper Detection:** Hash chains in audit logs detect unauthorized modifications
- **Backup Security:** Encrypted backups with same or stronger encryption

## Verification Guidelines

### Schema Validation

1. **YAML Validation:** Verify `schema/database.yaml` syntax and completeness
2. **Code Generation:** Run schema generator and verify `SchemaInfo.h` constants
3. **Migration Testing:** Test migrations on sample database
4. **Integrity Checks:** Run foreign key and constraint checks

### Data Integrity Testing

1. **Foreign Key Constraints:** Verify cascading deletes work correctly
2. **Check Constraints:** Test constraint violations are properly rejected
3. **Unique Constraints:** Verify duplicate prevention
4. **Transaction Testing:** Verify ACID properties maintained

### Performance Testing

1. **Write Performance:** Test vitals insertion rate (target: 500 Hz sustained)
2. **Query Performance:** Test time-range queries (target: < 2 seconds for 24 hours)
3. **Index Effectiveness:** Verify indices used by query planner (EXPLAIN QUERY PLAN)
4. **Cleanup Performance:** Test retention policy cleanup (target: < 30 seconds)

### Security Testing

1. **Encryption Verification:** Verify encrypted database cannot be read without key
2. **Hash Chain Validation:** Test tamper detection in audit logs
3. **Access Control:** Verify RBAC prevents unauthorized access
4. **Backup Security:** Verify backups are encrypted

## Document Metadata

**Original Document ID:** DESIGN-010  
**Migration Date:** 2025-12-01  
**New Document ID:** DOC-ARCH-017  
**Schema Source:** `schema/database.yaml` (single source of truth)

## Revision History

| Version | Date       | Changes                                                                                                                                                       |
| ------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-010 to DOC-ARCH-017. Complete database design with all tables, indices, constraints, retention policies, and security features. |
