# Database Design

This document outlines the design of the local SQLite database used by the Z Monitor application. It includes required tables, schema enhancements, indices, retention and archival policy, encryption and key management guidance, API payload extensions, and testing/sizing recommendations.

## 1. Purpose

This document enumerates the tables, columns, indices and operational behavior necessary to meet the functional, safety, audit, and security requirements of the Z Monitor. It is intended to be consumed by developers implementing `DatabaseManager`, `DataArchiver`, and by test engineers creating validation plans.

## 2. Required Tables (DDL-ready descriptions)

Below are the base tables we require. For each table a sample `CREATE TABLE` is included where appropriate.

### `patients`
Stores patient demographic information. This table serves as both a local patient registry and a cache for patient information retrieved from external systems (HIS/EHR) via `IPatientLookupService`.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS patients (
	patient_id TEXT PRIMARY KEY,
	name TEXT NOT NULL,
	dob TEXT NULL,
	sex TEXT NULL,
	mrn TEXT NULL,
	allergies TEXT NULL,
	room TEXT NULL,
	created_at INTEGER NOT NULL,
	last_lookup_at INTEGER NULL,
	lookup_source TEXT NULL
);
```

**Usage Notes:**
- `patient_id`: Primary identifier used for patient lookups (typically MRN)
- `mrn`: Medical Record Number, primary patient identifier in hospital systems
- `room`: Current room/bed assignment (may be updated from external system or overridden during admission)
- `last_lookup_at`: Timestamp of last successful lookup from external system (NULL if never looked up)
- `lookup_source`: Source of patient data ("local", "his", "ehr", etc.) for audit purposes

**ADT Workflow Enhancements:**
The `patients` table should be enhanced with additional columns for ADT workflow:
- `bed_location`: Current bed/room assignment (e.g., "ICU-4B") - part of Patient object
- `admitted_at`: Timestamp when patient was admitted to device (NULL if not currently admitted)
- `discharged_at`: Timestamp when patient was discharged (NULL if currently admitted)
- `admission_source`: Source of admission ("manual", "barcode", "central_station")
- `device_label`: Device identifier that admitted this patient (e.g., "ICU-MON-04")

See `doc/19_ADT_WORKFLOW.md` for complete ADT workflow documentation.

**Lookup Flow:**
1. When `PatientManager::loadPatientById(id)` is called, it first checks the local `patients` table
2. If found locally, patient data is returned immediately
3. If not found, `IPatientLookupService` is used to query external system
4. On successful lookup, patient data is saved to `patients` table with `last_lookup_at` timestamp
5. This caching reduces external system load and enables offline operation

### `vitals`
Stores time-series physiological data (heartbeat, spo2, rr, etc). This is expected to be the largest table.

Sample DDL (core columns):

```sql
CREATE TABLE IF NOT EXISTS vitals (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	uuid TEXT NULL,
	timestamp INTEGER NOT NULL,
	timestamp_iso TEXT NULL,
	patient_id TEXT NOT NULL,
	device_id TEXT NULL,
	heart_rate REAL NULL,
	spo2 REAL NULL,
	respiration_rate REAL NULL,
	signal_quality INTEGER NULL,
	sample_rate_hz REAL NULL,
	source TEXT NULL,
	is_synced BOOLEAN DEFAULT 0
);
```

Recommended indices:

```sql
CREATE INDEX IF NOT EXISTS idx_vitals_patient_time ON vitals(patient_id, timestamp);
CREATE INDEX IF NOT EXISTS idx_vitals_is_synced ON vitals(is_synced);
```

### `alarms`
Stores a history of all alarm events and their lifecycle metadata.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS alarms (
	alarm_id INTEGER PRIMARY KEY AUTOINCREMENT,
	patient_id TEXT NOT NULL,
	start_time INTEGER NOT NULL,
	end_time INTEGER NULL,
	alarm_type TEXT NOT NULL,
	priority TEXT NOT NULL,
	status TEXT NULL,
	acknowledged_by TEXT NULL,
	acknowledged_time INTEGER NULL,
	silenced_until INTEGER NULL,
	raw_value REAL NULL,
	context_snapshot_id INTEGER NULL
);
```

Recommended index:

```sql
CREATE INDEX IF NOT EXISTS idx_alarms_patient_priority ON alarms(patient_id, priority, start_time);
```

### `snapshots`
Stores captured waveform or contextual snapshot data (blobs), with metadata.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS snapshots (
	snapshot_id INTEGER PRIMARY KEY AUTOINCREMENT,
	patient_id TEXT NOT NULL,
	capture_time INTEGER NOT NULL,
	waveform_type TEXT NOT NULL,
	data BLOB NOT NULL,
	encoding TEXT NULL,
	sample_rate REAL NULL,
	length INTEGER NULL,
	compression TEXT NULL,
	checksum TEXT NULL
);
```

### `annotations`
Stores user-added notes for specific snapshots or records.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS annotations (
	annotation_id INTEGER PRIMARY KEY AUTOINCREMENT,
	snapshot_id INTEGER NOT NULL,
	author_id TEXT NULL,
	sensitivity TEXT NULL,
	note TEXT NOT NULL,
	creation_time INTEGER NOT NULL
);
```

### `infusion_events`
Detailed infusion pump logging (start/stop/pause/errors).

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS infusion_events (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	patient_id TEXT NOT NULL,
	timestamp INTEGER NOT NULL,
	drug_name TEXT NULL,
	flow_rate_ml_per_hr REAL NULL,
	total_volume_infused_ml REAL NULL,
	time_remaining_sec INTEGER NULL,
	occlusion_pressure_mmHg REAL NULL,
	event_type TEXT NOT NULL,
	is_synced BOOLEAN DEFAULT 0
);
```

### `device_events`
Device health and telemetry events.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS device_events (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp INTEGER NOT NULL,
	device_id TEXT NOT NULL,
	battery_percent REAL NULL,
	cpu_temp_c REAL NULL,
	mem_percent REAL NULL,
	network_latency_ms REAL NULL,
	event_type TEXT NULL,
	details TEXT NULL
);
```

### `notifications`
UI notifications history for NotificationBell and audit.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS notifications (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp INTEGER NOT NULL,
	level TEXT NOT NULL,
	message TEXT NOT NULL,
	is_read BOOLEAN DEFAULT 0,
	related_alarm_id INTEGER NULL,
	expiration INTEGER NULL
);
```

### `predictive_scores`
Stores predictive analytics outputs and provenance.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS predictive_scores (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp INTEGER NOT NULL,
	patient_id TEXT NULL,
	score_type TEXT NOT NULL,
	score_value REAL NOT NULL,
	model_version TEXT NULL,
	details TEXT NULL
);
```

### `users`
Local device accounts (PIN-based metadata) used for audit and RBAC.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS users (
	user_id TEXT PRIMARY KEY,
	username TEXT UNIQUE NOT NULL,
	role TEXT NOT NULL,
	pin_hash TEXT NOT NULL,
	created_at INTEGER NOT NULL,
	last_login INTEGER NULL
);
```

### `settings`
Stores device configuration settings and user preferences.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS settings (
	key TEXT PRIMARY KEY,
	value TEXT NOT NULL,
	updated_at INTEGER NOT NULL,
	updated_by TEXT NULL
);
```

Recommended default settings:
- `deviceId`: Unique device identifier for telemetry transmission (e.g., "ZM-001")
- `deviceLabel`: Static device identifier/asset tag (e.g., "ICU-MON-04") - fixed technical identifier, separate from patient assignment
- `measurementUnit`: Measurement system preference ("metric" or "imperial")
- `serverUrl`: Central server URL for telemetry transmission (e.g., "https://monitoring.hospital.com:8443", default: "https://localhost:8443")
- `useMockServer`: Boolean flag to use mock server for testing/development ("true" or "false", default: "false")

**Note:** `bedId` setting has been removed. Bed location is now part of the Patient object and managed through the ADT (Admission, Discharge, Transfer) workflow. See `doc/19_ADT_WORKFLOW.md` for details.

### `audit_log`
Immutable audit trail for critical user actions (patient assignments, settings changes, etc.). For security-specific events, see `security_audit_log`.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS audit_log (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp INTEGER NOT NULL,
	user_id TEXT NULL,
	action TEXT NOT NULL,
	target_id TEXT NULL,
	details TEXT NULL
);
```

**Note:** Security-related events (authentication, connections, certificate operations) are logged in `security_audit_log` table for better categorization and compliance.

### `certificates`
Certificate metadata for installed device/server certs (no private keys stored). Tracks certificate lifecycle, validation, and revocation status.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS certificates (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	device_id TEXT NOT NULL,
	cert_serial TEXT NOT NULL,
	cert_subject TEXT NULL,
	cert_issuer TEXT NULL,
	issued_at INTEGER NULL,
	expires_at INTEGER NULL,
	status TEXT NOT NULL,
	last_validated_at INTEGER NULL,
	revocation_reason TEXT NULL,
	revoked_at INTEGER NULL,
	cert_fingerprint TEXT NULL,
	created_at INTEGER NOT NULL
);
```

**Usage Notes:**
- `device_id`: Device identifier this certificate belongs to
- `cert_serial`: Certificate serial number (unique identifier)
- `cert_subject`: Certificate subject (CN, O, etc.)
- `cert_issuer`: Certificate issuer (CA information)
- `status`: Certificate status ("active", "expired", "revoked", "pending_renewal")
- `last_validated_at`: Timestamp of last successful certificate validation
- `revocation_reason`: Reason for revocation if revoked (e.g., "compromised", "key_rotation")
- `revoked_at`: Timestamp when certificate was revoked
- `cert_fingerprint`: SHA-256 fingerprint of certificate for quick lookup

**Recommended index:**
```sql
CREATE INDEX IF NOT EXISTS idx_certificates_device_status ON certificates(device_id, status);
CREATE INDEX IF NOT EXISTS idx_certificates_expires ON certificates(expires_at) WHERE status = 'active';
```

### `security_audit_log`
Stores security-relevant events for audit, forensics, and compliance. Immutable audit trail.

Sample DDL:

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
	error_message TEXT NULL
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
- `error`: Failed operations (authentication failures)
- `critical`: Security violations (certificate revocation, unauthorized access)

**Recommended indices:**
```sql
CREATE INDEX IF NOT EXISTS idx_security_audit_timestamp ON security_audit_log(timestamp);
CREATE INDEX IF NOT EXISTS idx_security_audit_category ON security_audit_log(event_category, timestamp);
CREATE INDEX IF NOT EXISTS idx_security_audit_device ON security_audit_log(device_id, timestamp);
```

**Usage Notes:**
- All security events should be logged immediately (no batching)
- Logs are append-only (never updated or deleted)
- Retention policy: 90 days (configurable)
- Critical events may trigger alerts/notifications

### `archival_jobs`
Tracks archival/export jobs and their status.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS archival_jobs (
	archive_id INTEGER PRIMARY KEY AUTOINCREMENT,
	table_name TEXT NOT NULL,
	record_count INTEGER NOT NULL,
	archived_before_ts INTEGER NOT NULL,
	archive_location TEXT NULL,
	archived_at INTEGER NOT NULL,
	status TEXT NOT NULL
);
```

### `admission_events`
Tracks patient admission, discharge, and transfer events for audit and compliance. Part of the ADT (Admission, Discharge, Transfer) workflow.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS admission_events (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp INTEGER NOT NULL,
	event_type TEXT NOT NULL,
	patient_mrn TEXT NOT NULL,
	patient_name TEXT NULL,
	device_label TEXT NOT NULL,
	bed_location TEXT NULL,
	admission_source TEXT NULL,
	user_id TEXT NULL,
	details TEXT NULL
);
```

**Usage Notes:**
- `event_type`: Type of event ("admission", "discharge", "transfer")
- `patient_mrn`: Medical Record Number of the patient
- `device_label`: Device identifier/asset tag that performed the action
- `bed_location`: Bed/room location associated with the event
- `admission_source`: Source of admission ("manual", "barcode", "central_station", "transfer")
- `user_id`: Clinician/Technician who performed the action (NULL if automated)
- `details`: Additional event details in JSON format (optional)

**Recommended indices:**
```sql
CREATE INDEX IF NOT EXISTS idx_admission_events_patient ON admission_events(patient_mrn, timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_device ON admission_events(device_label, timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_type ON admission_events(event_type, timestamp);
```

**ADT Workflow:** See `doc/19_ADT_WORKFLOW.md` for complete ADT workflow documentation.

### `db_encryption_meta`
Stores metadata about the DB encryption key and algorithm (not the key itself).

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS db_encryption_meta (
	key_version TEXT,
	algorithm TEXT,
	salt TEXT,
	created_at INTEGER
);
```

## 3. Enhancements to Existing Tables

Enhance existing `vitals`, `alarms`, `snapshots`, and `annotations` with additional metadata to support traceability and analytics.

`vitals` additions:
- `uuid`, `timestamp_iso`, `device_id`, `source`, `signal_quality`, `sample_rate_hz`, `predictive_score_ref`

Note: The `device_id` column in `vitals` should reference the `deviceId` setting from the `settings` table to ensure consistency across telemetry data.

`alarms` additions:
- `acknowledged_by`, `acknowledged_time`, `silenced_until`, `raw_value`, `context_snapshot_id`

`snapshots` additions:
- `encoding`, `sample_rate`, `length`, `compression`, `checksum`

`annotations` additions:
- `author_id`, `sensitivity`

## 4. Retention, Archival and Cleanup

**Data Retention Policies:**
- **Vitals data:** 7 days (configurable via `SettingsManager`, default: 7 days)
- **Alarm history:** 90 days (configurable, default: 90 days)
- **Security audit log:** 90 days minimum (configurable, default: 90 days, required for compliance)
- **Device events:** 30 days (configurable, default: 30 days)
- **Patient data:** Until patient discharge + 30 days (configurable, default: 30 days after discharge)
- **Notifications:** 7 days (configurable, default: 7 days)
- **Predictive scores:** 7 days (configurable, default: 7 days)

**Archival Process:**
- Archival: `DataArchiver::archiveData(cutoff_ts)` exports data older than cutoff into an archive file or remote store, creates an `archival_jobs` entry and, on success, deletes original rows inside a transaction.

Example transactional archive pattern:

```sql
BEGIN TRANSACTION;
INSERT INTO archived_vitals (SELECT * FROM vitals WHERE timestamp < :cutoff_ts);
DELETE FROM vitals WHERE timestamp < :cutoff_ts;
COMMIT;
```

Notes:
- `archived_vitals` may be a local table or an exported file (CSV/NDJSON). Use `archival_jobs.archive_location` to store the export path.
- Always archive within a transaction and record counts in `archival_jobs`.

## 5. Encryption & Key Management

- Use SQLCipher (AES-256) for DB encryption.
- The DB encryption key MUST be stored in a secure keystore (HSM / OS keystore). For development testing use a protected config file with strict permissions.
- Do NOT store encryption keys in the DB or repo.

## 6. API / Telemetry Payload Extensions

Extend `/api/telemetry` payload to include:
- `deviceInfo`: `{ deviceId, deviceCertificateSerial, deviceFirmwareVersion, batchId }`
- `vitals`: list of vitals (include `uuid`, `timestamp`, `signal_quality`, `sample_rate_hz`)
- `alarms`: list of alarm objects
- `infusionEvents`: list of infusion events
- `predictiveScores`: list of predictive scores

Server response should return processed IDs grouped per table so the device can mark only acknowledged records as `is_synced`.

## 7. Testing Considerations

- Unit tests for DB CRUD, `DatabaseManager::open`, `saveData`, `getHistoricalData`.
- Network sync tests: simulate partial success and ensure `is_synced` marking is correct.
- Archival tests: verify `archival_jobs` counts and restore paths.
- Security tests: attempt DB access without key and test key rotation.

## 8. Sizing Guidance

- 1 Hz vitals at 200 bytes/record => ~17 MB/day. 7 days => ~119 MB.
- Waveform snapshots (250 Hz, 2 channels, 2 bytes/sample) => ~86 MB/day per channel.

## 9. Migration Notes

When updating an existing schema, prefer these steps:

1. Create new tables/columns with `IF NOT EXISTS` or `ALTER TABLE ADD COLUMN`.
2. Backfill data in small batches inside transactions.
3. Add indices after backfill to avoid excessive index maintenance during migration.

### 9.1. ADT Workflow Migration

**Migration from Bed ID to ADT Workflow:**

1. **Create `admission_events` table:**
   ```sql
   CREATE TABLE IF NOT EXISTS admission_events (
       id INTEGER PRIMARY KEY AUTOINCREMENT,
       timestamp INTEGER NOT NULL,
       event_type TEXT NOT NULL,
       patient_mrn TEXT NOT NULL,
       patient_name TEXT NULL,
       device_label TEXT NOT NULL,
       bed_location TEXT NULL,
       admission_source TEXT NULL,
       user_id TEXT NULL,
       details TEXT NULL
   );
   ```

2. **Enhance `patients` table with ADT columns:**
   ```sql
   ALTER TABLE patients ADD COLUMN bed_location TEXT NULL;
   ALTER TABLE patients ADD COLUMN admitted_at INTEGER NULL;
   ALTER TABLE patients ADD COLUMN discharged_at INTEGER NULL;
   ALTER TABLE patients ADD COLUMN admission_source TEXT NULL;
   ALTER TABLE patients ADD COLUMN device_label TEXT NULL;
   ```

3. **Migrate `bedId` setting to `deviceLabel`:**
   ```sql
   -- Extract bedId value and create deviceLabel
   INSERT INTO settings (key, value, updated_at)
   SELECT 'deviceLabel', value, strftime('%s', 'now')
   FROM settings WHERE key = 'bedId';
   
   -- Remove bedId setting
   DELETE FROM settings WHERE key = 'bedId';
   ```

4. **Backfill existing patient data (if any):**
   - If patients table has records with `room` column, migrate to `bed_location`
   - Set `admission_source` to 'migration' for existing records

5. **Create indices:**
   ```sql
   CREATE INDEX IF NOT EXISTS idx_admission_events_patient ON admission_events(patient_mrn, timestamp);
   CREATE INDEX IF NOT EXISTS idx_admission_events_device ON admission_events(device_label, timestamp);
   ```

See `doc/19_ADT_WORKFLOW.md` for complete ADT workflow documentation.

## 10. Next Steps

- (A) Add an OpenAPI/JSON Schema for `/api/telemetry` in `doc/openapi/telemetry.yaml`.
- (B) Implement `DatabaseManager::cleanupOldData` and `DataArchiver::archiveData` in `src/core`.
- (C) Add migration SQL files under `doc/migrations/` to apply schema changes incrementally.

```
