---
doc_id: DOC-REQ-005
title: Data Requirements
category: REQ
status: approved
version: 1.0
last_updated: 2025-12-01
author: Z Monitor Team
related_docs:
  - DOC-REQ-003
  - DOC-ARCH-005
  - DOC-ARCH-006
tags:
  - data-requirements
  - database
  - schema
  - retention
  - requirements
---

# Data Requirements

## 1. Overview

This document specifies requirements for data handling, storage, retention, privacy, and integrity in the Z Monitor system.

**Related Documents:**
- **Database Design:** [../architecture_and_design/10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- **Schema Management:** [../architecture_and_design/33_SCHEMA_MANAGEMENT.md](../architecture_and_design/33_SCHEMA_MANAGEMENT.md)
- **Data Migration:** [../architecture_and_design/34_DATA_MIGRATION_WORKFLOW.md](../architecture_and_design/34_DATA_MIGRATION_WORKFLOW.md)

---

## 2. Data Requirements Categories

### 2.1 Data Structures (REQ-DATA-STRUCT-###)
Schema definitions, data types, relationships

### 2.2 Data Retention (REQ-DATA-RET-###)
Retention periods, archival, deletion policies

### 2.3 Data Security (REQ-DATA-SEC-###)
Encryption, access control, privacy

### 2.4 Data Integrity (REQ-DATA-INT-###)
Validation, consistency, transactions

### 2.5 Data Quality (REQ-DATA-QUAL-###)
Accuracy, completeness, timeliness

---

## 3. Data Structure Requirements

### [REQ-DATA-STRUCT-001] Vital Signs Data Structure

**Category:** Data Structures  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall store vital signs data with required fields: timestamp, patient_mrn, device_id, heart_rate, spo2, respiration_rate, signal_quality, and sync status.

**Rationale:**
Structured data enables querying, trending, and analysis. Required fields ensure data completeness for clinical and audit purposes.

**Acceptance Criteria:**
- Vitals table schema defined in `schema/database.yaml`
- All fields properly typed (INTEGER, REAL, TEXT, BOOLEAN)
- Timestamp stored as Unix milliseconds (INTEGER) for performance
- UUID generated for each record
- Foreign key relationships enforced (batch_id → telemetry_metrics)
- Indices created for common queries (patient_mrn, timestamp, is_synced)
- Schema version tracked for migrations

**Related Requirements:**
- REQ-FUN-VITAL-002 (recording)
- REQ-DATA-INT-001 (integrity)

**Traces To:**
- Use Case: UC-VM-001
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (vitals table)
- Design: [33_SCHEMA_MANAGEMENT.md](../architecture_and_design/33_SCHEMA_MANAGEMENT.md)
- Test: Test-DATA-STRUCT-001

---

### [REQ-DATA-STRUCT-002] Patient Data Structure

**Category:** Data Structures  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall store patient information including MRN, name, date of birth, sex, bed location, admission status, and timestamps.

**Rationale:**
Patient record required for identification, association with vitals, and audit trail.

**Acceptance Criteria:**
- Patients table schema defined
- Fields: id, mrn (unique), name, dob, sex, bed_location, admission_status, admitted_at, admission_source, discharged_at, last_lookup_at, lookup_source
- MRN is unique index (cannot duplicate)
- Admission status: ADMITTED, DISCHARGED, TRANSFERRED
- Timestamps for all state changes
- Lookup cache includes source (HIS, CACHE, MANUAL)

**Related Requirements:**
- REQ-FUN-PAT-001 (patient admission)
- REQ-FUN-PAT-011 (caching)

**Traces To:**
- Use Case: UC-PM-001, UC-PM-002, UC-PM-004
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (patients table)
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Test: Test-DATA-STRUCT-002

---

### [REQ-DATA-STRUCT-003] Alarm Data Structure

**Category:** Data Structures  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall store alarm events with priority, type, value, threshold, patient association, timestamps, and acknowledgment information.

**Rationale:**
Alarm history critical for quality review, compliance, and incident investigation.

**Acceptance Criteria:**
- Alarms table schema defined
- Fields: id, uuid, timestamp, priority (HIGH/MEDIUM/LOW), alarm_type, value, threshold, patient_mrn, device_id, acknowledged_at, acknowledged_by, response_time_ms, is_synced
- Priority stored as TEXT (HIGH/MEDIUM/LOW per IEC 60601-1-8)
- Response time calculated: acknowledged_at - timestamp
- Alarm resolution tracked (resolved_at, resolution)

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-020 (acknowledgment)

**Traces To:**
- Use Case: UC-AM-001, UC-AM-002
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (alarms table)
- Test: Test-DATA-STRUCT-003

---

## 4. Data Retention Requirements

### [REQ-DATA-RET-001] Vital Signs Retention

**Category:** Data Retention  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall retain vital signs data locally for 7 days and automatically delete older data to manage storage.

**Rationale:**
7 days provides buffer for sync recovery and local trend analysis. Automated cleanup prevents storage exhaustion.

**Acceptance Criteria:**
- Vitals data retained for exactly 7 days
- Cleanup job runs daily at 2:00 AM (low activity time)
- Data older than 7 days deleted automatically
- Cleanup operation logged (records deleted, timestamp)
- Storage space monitored (warning if > 90% full)
- Critical events (Code Blues) flagged for extended retention (90 days)

**Measurement:**
- Monitor database size over time
- Verify oldest record age daily
- Track cleanup job execution

**Related Requirements:**
- REQ-DATA-STRUCT-001 (vitals structure)
- REQ-FUN-DATA-010 (archival)
- REQ-NFR-SCALE-001 (storage capacity)

**Traces To:**
- Use Case: Implicit (background process)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (retention policy)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DataArchiver)
- Test: Test-DATA-RET-001

---

### [REQ-DATA-RET-002] Alarm History Retention

**Category:** Data Retention  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall retain alarm history for 90 days to support quality review and compliance audits.

**Rationale:**
Quality teams review alarms monthly/quarterly. Regulatory audits may request historical alarm data. 90 days provides adequate history.

**Acceptance Criteria:**
- Alarm data retained for 90 days
- Automated cleanup deletes alarms older than 90 days
- Critical alarms (Code Blue, patient safety events) retained indefinitely
- Cleanup logged
- Alarm data synced to central server for long-term archival

**Related Requirements:**
- REQ-DATA-STRUCT-003 (alarm structure)
- REQ-DATA-RET-001 (vitals retention)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (retention policy)
- Test: Test-DATA-RET-002

---

### [REQ-DATA-RET-003] Audit Log Retention

**Category:** Data Retention  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall retain security audit logs for 90 days locally and sync to central server for long-term archival per HIPAA requirements.

**Rationale:**
HIPAA requires audit trail retention (typically 6 years). Local retention for 90 days with central archival meets compliance.

**Acceptance Criteria:**
- Security audit logs retained locally for 90 days
- Logs synced to central server immediately
- Central server retains logs for 6 years (HIPAA compliance)
- Local logs auto-deleted after 90 days
- Deletion logged (ironically, to audit log)

**Related Requirements:**
- REQ-NFR-SEC-010 (audit logging)
- REQ-REG-HIPAA-003 (audit retention)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (security_audit_log table)
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Test: Test-DATA-RET-003

---

## 5. Data Security Requirements

### [REQ-DATA-SEC-001] Database Encryption

**Category:** Data Security  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt the local database using SQLCipher with AES-256-CBC to protect patient health information from unauthorized access.

**Rationale:**
HIPAA requires encryption of PHI at rest. Device theft or unauthorized access must not expose patient data.

**Acceptance Criteria:**
- SQLCipher enabled for all databases
- Encryption algorithm: AES-256-CBC
- Encryption key: 256-bit randomly generated
- Key stored securely (not in database file)
- Pragma settings: `cipher_page_size=4096`, `kdf_iter=256000`
- Database file unreadable without encryption key
- Key rotation supported (annual or on compromise)

**Measurement:**
- Verify database encrypted (hexdump shows encrypted bytes, not plaintext)
- Penetration test: Attempt to read database without key
- Performance impact measurement (< 10% overhead)

**Related Requirements:**
- REQ-NFR-SEC-003 (encryption at rest)
- REQ-SEC-ENC-002 (key management)
- REQ-REG-HIPAA-001 (encryption mandate)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 2)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (SQLCipher section)
- Test: Test-DATA-SEC-001

---

### [REQ-DATA-SEC-002] Access Control

**Category:** Data Security  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall enforce role-based access control for patient data, ensuring users can only access data appropriate for their role.

**Rationale:**
HIPAA minimum necessary principle. Users should only access data needed for their job function.

**Acceptance Criteria:**
- Nurses/Physicians: Full access to current patient data
- Technicians: Access to device diagnostics, no patient data
- Administrators: Full system access, all logs
- All data access logged (who, what, when)
- Unauthorized access attempts blocked and logged
- Access control checked at application layer

**Related Requirements:**
- REQ-FUN-USER-004 (RBAC)
- REQ-NFR-SEC-010 (audit logging)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 4)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-DATA-SEC-002

---

### [REQ-DATA-SEC-003] Data Anonymization for Diagnostics

**Category:** Data Security  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall support exporting anonymized/de-identified data for diagnostics, testing, and training purposes without exposing PHI.

**Rationale:**
Developers, testers, and trainers need realistic data but must not access patient PHI. Anonymization enables safe data sharing.

**Acceptance Criteria:**
- Export function available to administrators
- Anonymization replaces:
  - MRN → "MRN-ANON-001"
  - Name → "Patient A"
  - DOB → Randomized (preserve age category)
  - Device ID → Anonymized
- Timestamps preserved (relative to start)
- Vital signs and alarm patterns preserved
- Anonymization irreversible (not decryptable)
- Export logged

**Related Requirements:**
- REQ-REG-HIPAA-004 (de-identification)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- Test: Test-DATA-SEC-003

**Notes:**
- Follows HIPAA Safe Harbor method for de-identification
- Used for training datasets and development/testing

---

## 6. Data Integrity Requirements

### [REQ-DATA-INT-001] Data Validation

**Category:** Data Integrity  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall validate all data at boundaries (user input, sensor input, network input) to ensure data quality and prevent corruption.

**Rationale:**
Invalid data causes errors, false alarms, and poor clinical decisions. Validation at boundaries ensures data quality throughout system.

**Acceptance Criteria:**
- User input validation:
  - MRN format: `^MRN-\d{4,8}$`
  - PIN: 4-6 digits
  - Dates: Valid date ranges
- Sensor input validation:
  - Heart Rate: 30-250 bpm
  - SpO2: 50-100%
  - Respiration Rate: 5-60 rpm
- Network input validation:
  - JSON schema validation
  - Certificate validation
  - Signature verification
- Invalid data rejected with error message
- Validation errors logged

**Related Requirements:**
- REQ-FUN-VITAL-003 (vital signs validation)
- REQ-NFR-SEC-010 (security)

**Traces To:**
- Use Case: All use cases (input validation)
- Design: [20_ERROR_HANDLING_STRATEGY.md](../architecture_and_design/20_ERROR_HANDLING_STRATEGY.md)
- Design: Foundation: [../../foundation/03_security_and_cryptography/07_input_validation.md](../../foundation/03_security_and_cryptography/07_input_validation.md)
- Test: Test-DATA-INT-001

---

### [REQ-DATA-INT-002] Transactional Data Integrity

**Category:** Data Integrity  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall use database transactions to ensure data consistency, preventing partial writes or corrupted state.

**Rationale:**
Database crashes during write can corrupt data. Transactions ensure ACID properties (Atomicity, Consistency, Isolation, Durability).

**Acceptance Criteria:**
- All multi-record operations wrapped in transactions
- Patient admission: Single transaction for patient record + admission event
- Batch vital signs insert: Single transaction
- Transaction rollback on error
- Database journal mode: WAL (Write-Ahead Logging)
- Database integrity checked on startup
- Corruption detected and reported

**Related Requirements:**
- REQ-DATA-STRUCT-001 (data structures)
- REQ-NFR-REL-010 (crash recovery)

**Traces To:**
- Design: [30_DATABASE_ACCESS_STRATEGY.md](../architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md)
- Design: Foundation: [../../foundation/02_database_and_data_management/04_transactions_acid.md](../../foundation/02_database_and_data_management/04_transactions_acid.md)
- Test: Test-DATA-INT-002 (crash testing during writes)

---

### [REQ-DATA-INT-003] Data Integrity Verification

**Category:** Data Integrity  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall verify database integrity on startup and provide tools for administrators to check and repair corrupted databases.

**Rationale:**
Database corruption from crashes, disk errors, or bugs must be detected and repaired to prevent data loss or incorrect operation.

**Acceptance Criteria:**
- PRAGMA integrity_check run on startup
- Corruption detected: Warning displayed, logged, administrator notified
- Repair tool available (sqlite3 .recover or backup restore)
- Database backup created before repair
- Integrity check results logged
- Automatic backup if corruption detected

**Related Requirements:**
- REQ-DATA-INT-002 (transactions)
- REQ-FUN-SYS-010 (diagnostics)

**Traces To:**
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DatabaseManager)
- Test: Test-DATA-INT-003

---

## 7. Data Retention Requirements

### [REQ-DATA-RET-001] Vital Signs Retention Period

**Category:** Data Retention  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall retain vital signs data locally for 7 days and delete older data automatically to manage storage.

**Rationale:**
Balance between local trend analysis needs and storage constraints. Central server provides long-term archival.

**Acceptance Criteria:**
- Vitals retained for 7 days (168 hours)
- Daily cleanup job at 2:00 AM
- DELETE query: `WHERE timestamp < datetime('now', '-7 days')`
- Cleanup logged: records deleted, execution time
- Critical events (Code Blue) flagged for 90-day retention
- Storage monitored: Warning if > 90% full

**Related Requirements:**
- REQ-DATA-STRUCT-001 (vitals structure)
- REQ-FUN-DATA-010 (archival)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (retention section)
- Test: Test-DATA-RET-001

---

### [REQ-DATA-RET-010] Backup and Recovery

**Category:** Data Retention  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall create automatic backups of the database daily and provide recovery mechanism in case of database corruption or failure.

**Rationale:**
Hardware failures, disk errors, or bugs can corrupt database. Backups enable recovery without data loss.

**Acceptance Criteria:**
- Automatic backup daily at 3:00 AM
- Backup location: `/var/lib/z-monitor/backups/`
- Backup filename: `z-monitor_YYYY-MM-DD_HHMMSS.db`
- Backups retained for 7 days (oldest deleted)
- Backup verified after creation (integrity check)
- Recovery tool available for administrators
- Backup/recovery documented in admin guide

**Related Requirements:**
- REQ-DATA-INT-003 (integrity verification)
- REQ-NFR-REL-010 (recovery)

**Traces To:**
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DatabaseManager, BackupManager)
- Test: Test-DATA-RET-010

---

## 8. Data Quality Requirements

### [REQ-DATA-QUAL-001] Data Completeness

**Category:** Data Quality  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall ensure all required fields are populated before storing records, preventing incomplete or null-critical data.

**Rationale:**
Incomplete data causes queries to fail, trends to be inaccurate, and clinical decisions to be uninformed.

**Acceptance Criteria:**
- Database schema enforces NOT NULL for required fields
- Application validates completeness before insert
- Null checks for: patient_mrn, device_id, timestamp
- Optional fields explicitly marked as NULL in schema
- Incomplete records rejected with error message
- Validation errors logged

**Related Requirements:**
- REQ-DATA-INT-001 (validation)
- REQ-DATA-STRUCT-001 (schema)

**Traces To:**
- Design: [33_SCHEMA_MANAGEMENT.md](../architecture_and_design/33_SCHEMA_MANAGEMENT.md)
- Test: Test-DATA-QUAL-001

---

### [REQ-DATA-QUAL-002] Data Accuracy

**Category:** Data Quality  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall ensure vital signs data accurately reflects sensor measurements with timestamp precision of 1 millisecond.

**Rationale:**
Clinical decisions based on vital signs data. Inaccurate data could lead to wrong treatment or missed critical events.

**Acceptance Criteria:**
- Timestamp precision: 1 millisecond (Unix milliseconds)
- Vital signs values stored as REAL (floating point)
- Precision: 1 decimal place (e.g., 98.5% for SpO2)
- No rounding errors from unit conversions
- Sensor calibration data tracked
- Data accuracy verified during testing

**Related Requirements:**
- REQ-DATA-STRUCT-001 (data structure)
- REQ-FUN-VITAL-001 (display)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- Test: Test-DATA-QUAL-002

---

### [REQ-DATA-QUAL-003] Data Timeliness

**Category:** Data Quality  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall timestamp all data at creation time and track data age to ensure timeliness for clinical decisions.

**Rationale:**
Stale data is misleading. Clinicians need current information. Timestamp tracking enables staleness detection.

**Acceptance Criteria:**
- All records timestamped at creation: `created_at`
- Database writes timestamped: `saved_at`
- Network sync tracked: `transmitted_at`, `server_received_at`
- Latency metrics calculated and stored (telemetry_metrics table)
- Stale data warnings (> 10 seconds old for vitals)
- Clock synchronization with NTP

**Related Requirements:**
- REQ-DATA-STRUCT-001 (timestamps)
- REQ-NFR-PERF-200 (latency)

**Traces To:**
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (telemetry_metrics)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (ClockSyncService)
- Test: Test-DATA-QUAL-003

---

## 9. Privacy Requirements

### [REQ-DATA-PRIV-001] Minimum Necessary Access

**Category:** Privacy  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement HIPAA "minimum necessary" principle, displaying only patient data required for current task.

**Rationale:**
HIPAA requires limiting PHI access to minimum necessary. Reduces privacy risk and supports compliance.

**Acceptance Criteria:**
- Dashboard shows current patient only (not other patients)
- Historical data access requires explicit user action
- Trend view shows only selected time range (not all data)
- Logs show summary, not full patient records
- Data exports require justification (logged)

**Related Requirements:**
- REQ-DATA-SEC-002 (access control)
- REQ-REG-HIPAA-002 (privacy)

**Traces To:**
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Test: Test-DATA-PRIV-001

---

### [REQ-DATA-PRIV-002] Data Deletion on Demand

**Category:** Privacy  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall support secure deletion of patient data upon request (right to be forgotten, data breach response).

**Rationale:**
Privacy regulations may require patient data deletion. Security incidents may require secure data erasure.

**Acceptance Criteria:**
- Administrator can delete patient data by MRN
- Deletion is secure (SQLite secure_delete=ON, or overwrite)
- Deletion cascades (patient + vitals + alarms + events)
- Deletion logged (who, what, when, why)
- Deletion irreversible (confirmation required)
- Deletion synced to central server

**Related Requirements:**
- REQ-DATA-SEC-002 (access control)
- REQ-REG-HIPAA-005 (data deletion)

**Traces To:**
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DatabaseManager)
- Test: Test-DATA-PRIV-002

---

## 10. Data Migration Requirements

### [REQ-DATA-MIG-001] Schema Versioning

**Category:** Data Migration  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall track database schema version and automatically apply migrations when schema changes are deployed.

**Rationale:**
Schema evolves over time. Automated migrations prevent data loss and ensure smooth upgrades.

**Acceptance Criteria:**
- Schema version tracked in schema_version table
- Current version: INTEGER (e.g., 1, 2, 3)
- Migration scripts in `schema/migrations/schema/`
- System checks version on startup
- If version mismatch, migrations applied automatically
- Backup created before migration
- Migration logged (version, timestamp, success/failure)
- Failed migration: Rollback, error displayed, administrator notified

**Related Requirements:**
- REQ-DATA-INT-002 (transactions)
- REQ-DATA-RET-010 (backup)

**Traces To:**
- Design: [34_DATA_MIGRATION_WORKFLOW.md](../architecture_and_design/34_DATA_MIGRATION_WORKFLOW.md)
- Design: [33_SCHEMA_MANAGEMENT.md](../architecture_and_design/33_SCHEMA_MANAGEMENT.md)
- Test: Test-DATA-MIG-001

---

## 11. Data Requirements Summary

### Total Requirements: 15 (of ~40-50 planned)

| Category        | Requirements | Critical | Must Have | Should Have |
| --------------- | ------------ | -------- | --------- | ----------- |
| Data Structures | 3            | 0        | 3         | 0           |
| Data Retention  | 3            | 0        | 3         | 0           |
| Data Security   | 3            | 1        | 1         | 1           |
| Data Integrity  | 3            | 0        | 3         | 0           |
| Data Quality    | 3            | 0        | 3         | 0           |
| Privacy         | 2            | 0        | 1         | 1           |
| Data Migration  | 1            | 0        | 1         | 0           |
| **Total**       | **18**       | **1**    | **15**    | **2**       |

### Remaining Requirements (to be added):
- ~10 additional data structure requirements (settings, certificates, logs)
- ~5 data export/import requirements
- ~5 data archival requirements (compression, long-term storage)
- ~5 data synchronization requirements (conflict resolution, consistency)
- ~10 privacy requirements (access logs, consent, breach notification)

---

## 12. Related Documents

- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](./03_FUNCTIONAL_REQUIREMENTS.md)
- **Security Requirements:** [08_SECURITY_REQUIREMENTS.md](./08_SECURITY_REQUIREMENTS.md)
- **Database Design:** [../architecture_and_design/10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- **Schema Management:** [../architecture_and_design/33_SCHEMA_MANAGEMENT.md](../architecture_and_design/33_SCHEMA_MANAGEMENT.md)

---

*Data requirements ensure the system handles patient information safely, accurately, and in compliance with regulations.*

