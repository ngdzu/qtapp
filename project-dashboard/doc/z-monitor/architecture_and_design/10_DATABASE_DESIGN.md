# Database Design

**Document ID:** DESIGN-010  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

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

See [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) for complete ADT workflow documentation.

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
	timestamp INTEGER NOT NULL,           -- Time data was measured/created
	timestamp_iso TEXT NULL,
	patient_id TEXT NOT NULL,             -- Patient identifier (typically MRN)
	patient_mrn TEXT NOT NULL,            -- Medical Record Number (REQUIRED for patient association)
	device_id TEXT NULL,
	device_label TEXT NULL,               -- Device asset tag/identifier
	heart_rate REAL NULL,
	spo2 REAL NULL,
	respiration_rate REAL NULL,
	signal_quality INTEGER NULL,
	sample_rate_hz REAL NULL,
	source TEXT NULL,
	is_synced BOOLEAN DEFAULT 0,
	batch_id TEXT NULL                    -- References telemetry_metrics.batch_id (NULL if not yet transmitted)
);
```

**Critical Requirements:**
- **Patient Association:** `patient_mrn` is REQUIRED and must not be NULL. All vitals data must be associated with a patient MRN.
- **Data Integrity:** Both `patient_id` and `patient_mrn` are stored to support different lookup patterns, but `patient_mrn` is the primary identifier for patient association.
- **Standby State:** If no patient is admitted, vitals should not be recorded (device in STANDBY state).

**Design Decision: No Timing Data in `vitals` Table**

**Rationale:**
- **Performance**: `vitals` is the **largest, hottest table** with time-series data at 100-500 Hz. Adding timing columns would slow down the critical write path and impact alarm latency.
- **Storage Efficiency**: Most vitals are never transmitted individually (batched). Adding 9 timing columns to every row wastes storage on 90% of data.
- **Separation of Concerns**: Clinical data (`vitals`) vs. diagnostic/performance data (timing metrics) should be separate.
- **Query Performance**: Timing columns add index overhead and slow down clinical queries.

**Instead: Use `telemetry_metrics` Table for Timing**

Timing and latency tracking is done in the dedicated `telemetry_metrics` table (see below). To correlate vitals with timing:
- Each transmitted batch gets a `batch_id` (UUID)
- Vitals in that batch are linked via `batch_id` column (optional, nullable)
- Query timing for a specific vital: `JOIN vitals ON telemetry_metrics.batch_id = vitals.batch_id`

**Example: Correlating Vitals with Timing:**
```sql
-- Find vitals from slow batches (> 100ms)
SELECT v.*, tm.end_to_end_latency_ms, tm.batch_creation_latency_ms
FROM vitals v
INNER JOIN telemetry_metrics tm ON v.batch_id = tm.batch_id
WHERE tm.end_to_end_latency_ms > 100
ORDER BY tm.end_to_end_latency_ms DESC;
```

**Benefits of This Design:**
1. ✅ **Fast Writes**: No timing overhead on critical vitals insertion
2. ✅ **Storage Efficient**: Timing data only for transmitted batches (~10% of vitals)
3. ✅ **Clean Separation**: Clinical data separate from diagnostics
4. ✅ **Better Queries**: Clinical queries unaffected by timing columns
5. ✅ **Flexible**: Can add more timing metrics without touching `vitals` schema

Recommended indices:

```sql
CREATE INDEX IF NOT EXISTS idx_vitals_patient_time ON vitals(patient_id, timestamp);
CREATE INDEX IF NOT EXISTS idx_vitals_mrn_time ON vitals(patient_mrn, timestamp);  -- Primary lookup by MRN
CREATE INDEX IF NOT EXISTS idx_vitals_is_synced ON vitals(is_synced);
CREATE INDEX IF NOT EXISTS idx_vitals_device ON vitals(device_id, timestamp);
CREATE INDEX IF NOT EXISTS idx_vitals_batch_id ON vitals(batch_id) WHERE batch_id IS NOT NULL;  -- For joining with telemetry_metrics
```

### `alarms`
Stores a history of all alarm events and their lifecycle metadata.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS alarms (
	alarm_id INTEGER PRIMARY KEY AUTOINCREMENT,
	patient_id TEXT NOT NULL,
	patient_mrn TEXT NOT NULL,     -- Medical Record Number (REQUIRED for patient association)
	start_time INTEGER NOT NULL,
	end_time INTEGER NULL,
	alarm_type TEXT NOT NULL,
	priority TEXT NOT NULL,
	status TEXT NULL,
	acknowledged_by TEXT NULL,
	acknowledged_time INTEGER NULL,
	silenced_until INTEGER NULL,
	raw_value REAL NULL,
	threshold_value REAL NULL,     -- Threshold that was exceeded (historical snapshot for audit)
	context_snapshot_id INTEGER NULL
);
```

**Critical Requirements:**
- **Patient Association:** `patient_mrn` is REQUIRED and must not be NULL. All alarms must be associated with a patient MRN.
- **Threshold Storage:** `threshold_value` stores the threshold that was exceeded *at the time of the alarm* for historical/audit purposes. This is a snapshot and does not reflect current configured thresholds.

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
- `alarm_thresholds_{patientMrn}`: JSON object storing *configured* alarm thresholds for each patient (e.g., `alarm_thresholds_MRN-12345` → `{"HR": {"low": 60, "high": 120}, "SPO2": {"low": 90, "high": 100}}`)

**Notes:**
- `bedId` setting has been removed. Bed location is now part of the Patient object and managed through the ADT (Admission, Discharge, Transfer) workflow. See [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) for details.
- **Alarm Threshold Storage Clarification:**
  - **Settings table** (`alarm_thresholds_{patientMrn}` key): Stores *currently configured* alarm thresholds per patient. Updated when clinician changes thresholds.
  - **Alarms table** (`threshold_value` column): Stores *historical snapshot* of threshold value that was exceeded at alarm time. Immutable for audit/compliance.

### `action_log`
**UPDATED:** Immutable audit trail for all user actions (login, logout, patient management, settings changes, etc.). Enhanced schema with hash chain for tamper detection. For security-specific events, see `security_audit_log`.

**Purpose:**
- Log all user actions (login, logout, auto-logout, configuration changes)
- Track who performed what action and when
- Support compliance and audit requirements
- Detect tampering via hash chain

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,              -- Unix timestamp in milliseconds
    timestamp_iso TEXT NOT NULL,                 -- ISO 8601 timestamp for readability
    user_id TEXT NULL,                           -- User who performed action (NULL if no login required)
    user_role TEXT NULL,                          -- User role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR)
    action_type TEXT NOT NULL,                   -- Action type (LOGIN, LOGOUT, AUTO_LOGOUT, ADMIT_PATIENT, etc.)
    target_type TEXT NULL,                        -- Type of target (PATIENT, SETTING, NOTIFICATION, etc.)
    target_id TEXT NULL,                          -- Target identifier (MRN, setting name, notification ID)
    details TEXT NULL,                            -- JSON string with additional context
    result TEXT NOT NULL,                         -- SUCCESS, FAILURE, PARTIAL
    error_code TEXT NULL,                         -- Error code if result is FAILURE
    error_message TEXT NULL,                      -- Error message if result is FAILURE
    device_id TEXT NOT NULL,                      -- Device identifier
    session_token_hash TEXT NULL,                 -- SHA-256 hash of session token (for audit trail)
    ip_address TEXT NULL,                          -- IP address (if available, for network actions)
    previous_hash TEXT NULL                       -- SHA-256 hash of previous entry (hash chain for tamper detection)
);

-- Indexes for common queries
CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON action_log(timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_user ON action_log(user_id, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON action_log(action_type, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_target ON action_log(target_type, target_id, timestamp_ms DESC);
CREATE INDEX IF NOT EXISTS idx_action_log_device ON action_log(device_id, timestamp_ms DESC);
```

**Action Types:**
- `LOGIN` - User successfully logged in
- `LOGIN_FAILED` - Login attempt failed
- `LOGOUT` - User manually logged out
- `AUTO_LOGOUT` - Automatic logout due to inactivity
- `SESSION_EXPIRED` - Session expired (timeout)
- `ADMIT_PATIENT` - Patient admitted to device
- `DISCHARGE_PATIENT` - Patient discharged from device
- `TRANSFER_PATIENT` - Patient transferred to different device
- `CHANGE_SETTING` - Any setting changed
- `ADJUST_ALARM_THRESHOLD` - Alarm threshold adjusted
- `CLEAR_NOTIFICATIONS` - Recent notifications cleared
- `DISMISS_NOTIFICATION` - Single notification dismissed
- `VIEW_AUDIT_LOG` - Audit log accessed
- `EXPORT_DATA` - Data exported
- `ACCESS_DIAGNOSTICS` - Diagnostics view accessed
- `PROVISIONING_MODE_ENTERED` - Device entered provisioning mode

**Hash Chain for Tamper Detection:**
The `previous_hash` column implements a hash chain that detects unauthorized modifications:
1. **First Entry:** `previous_hash` is NULL (genesis entry)
2. **Subsequent Entries:** `previous_hash` = SHA-256(previous entry's: id || timestamp_ms || action_type || user_id || target_id || details || result)
3. **Validation:** To detect tampering, recompute hash chain from beginning

**Retention Policy:**
- **Retention:** 90 days minimum (configurable, default: 90 days)
- **Archival:** Old entries can be archived to external storage
- **Compliance:** Required for regulatory audits and compliance reporting

**Note:** Security-related events (authentication failures, certificate operations, network connections) are logged in `security_audit_log` table for better categorization. User actions (login, logout, configuration changes) are logged in `action_log` table.

**Related Documentation:**
- See [39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md](./39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md) for complete action logging workflow and requirements.

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
	error_message TEXT NULL,
	previous_hash TEXT NULL        -- SHA-256 hash of previous entry (hash chain for tamper detection)
);
```

**Hash Chain for Tamper Detection (REQ-SEC-AUDIT-002):**

The `previous_hash` column implements a hash chain that detects unauthorized modifications to audit log entries:

1. **First Entry:** `previous_hash` is NULL (genesis entry)
2. **Subsequent Entries:** `previous_hash` = SHA-256(previous entry's: id + timestamp + event_type + severity + user_id + details)
3. **Validation:** To detect tampering, recompute hash chain from beginning:
   ```sql
   -- Compute expected hash for entry N
   SELECT id, 
          SHA256(CAST(prev.id AS TEXT) || 
                 CAST(prev.timestamp AS TEXT) || 
                 prev.event_type || 
                 prev.severity || 
                 COALESCE(prev.user_id, '') || 
                 COALESCE(prev.details, '')) AS expected_hash,
          current.previous_hash AS stored_hash
   FROM security_audit_log current
   JOIN security_audit_log prev ON prev.id = current.id - 1
   WHERE expected_hash != stored_hash;  -- Finds tampered entries
   ```
4. **Security:** Any modification, insertion, or deletion of an entry breaks the hash chain, making tampering detectable
5. **Performance:** Hash validation performed periodically (e.g., nightly) or on-demand, not on every query

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

**ADT Workflow:** See [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) for complete ADT workflow documentation.

### `telemetry_metrics`
Stores comprehensive timing and performance metrics for telemetry transmission. This table is specifically designed for benchmarking, diagnostics, and latency analysis.

Sample DDL:

```sql
CREATE TABLE IF NOT EXISTS telemetry_metrics (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	batch_id TEXT NOT NULL UNIQUE,       -- UUID identifying the telemetry batch (unique, referenced by vitals.batch_id)
	device_id TEXT NOT NULL,
	device_label TEXT NULL,
	patient_mrn TEXT NULL,               -- NULL if device health/status data (no patient admitted)
	
	-- Timing milestones (all Unix timestamps in milliseconds)
	data_created_at INTEGER NOT NULL,    -- Time first data point in batch was created
	batch_created_at INTEGER NOT NULL,   -- Time batch object was created
	signed_at INTEGER NOT NULL,          -- Time batch was signed
	queued_for_tx_at INTEGER NOT NULL,   -- Time batch was queued for transmission
	transmitted_at INTEGER NOT NULL,     -- Time batch was sent over network
	server_received_at INTEGER NULL,     -- Time server received batch (from server response)
	server_processed_at INTEGER NULL,    -- Time server finished processing (from server response)
	server_ack_at INTEGER NULL,          -- Time server sent acknowledgment (from server response)
	
	-- Computed latency metrics (milliseconds)
	batch_creation_latency_ms INTEGER NULL,    -- batch_created_at - data_created_at
	signing_latency_ms INTEGER NULL,           -- signed_at - batch_created_at
	queue_wait_latency_ms INTEGER NULL,        -- transmitted_at - queued_for_tx_at
	network_latency_ms INTEGER NULL,           -- server_received_at - transmitted_at
	server_processing_latency_ms INTEGER NULL, -- server_processed_at - server_received_at
	end_to_end_latency_ms INTEGER NULL,        -- server_ack_at - data_created_at
	
	-- Batch statistics
	record_count INTEGER NOT NULL,       -- Number of records in batch (vitals, alarms, etc.)
	batch_size_bytes INTEGER NOT NULL,   -- Size of batch payload in bytes
	compressed_size_bytes INTEGER NULL,  -- Size after compression (if applicable)
	
	-- Status and error tracking
	status TEXT NOT NULL,                -- "success", "failed", "timeout", "retrying"
	error_message TEXT NULL,             -- Error details if status = "failed"
	retry_count INTEGER DEFAULT 0,       -- Number of retry attempts
	
	-- Performance classification
	latency_class TEXT NULL,             -- "excellent" (<10ms), "good" (<50ms), "acceptable" (<100ms), "slow" (>100ms)
	
	-- Metadata
	created_at INTEGER NOT NULL,         -- Time this metrics record was created
	updated_at INTEGER NULL              -- Time this metrics record was last updated (for retries)
);
```

**Usage Notes:**
- **`batch_id`**: UUID for correlating metrics across retries and server logs. **UNIQUE** to enable foreign key relationship from `vitals.batch_id`.
- **`patient_mrn`**: NULL indicates device health/status data (no patient admitted)
- **Timing Milestones**: Capture every stage of telemetry pipeline for bottleneck analysis
- **Computed Metrics**: Pre-computed for fast queries; updated via triggers or application code
- **`latency_class`**: Human-readable classification for quick filtering

**Relationship with `vitals` Table:**
- Each batch contains multiple vitals records
- `vitals.batch_id` references `telemetry_metrics.batch_id` (nullable, only set when transmitted)
- This enables correlation: "Which vitals were in the slow batch?"
- Keeps timing data **separate** from clinical data for performance

**Use Cases:**
1. **Real-Time Monitoring**: Dashboard showing current P95 latencies
2. **Alerting**: Trigger alerts if P95 exceeds thresholds (e.g., 100ms)
3. **Bottleneck Detection**: Identify if delays are in signing, network, or server processing
4. **Capacity Planning**: Analyze batch sizes and transmission rates
5. **Compliance Reporting**: Generate latency reports for regulatory audits
6. **A/B Testing**: Compare performance before/after code changes

**Example Queries:**

```sql
-- Real-time P95 latency (last hour)
SELECT 
    PERCENTILE_CONT(0.95) WITHIN GROUP (ORDER BY end_to_end_latency_ms) as p95_latency_ms,
    PERCENTILE_CONT(0.95) WITHIN GROUP (ORDER BY network_latency_ms) as p95_network_ms,
    PERCENTILE_CONT(0.95) WITHIN GROUP (ORDER BY server_processing_latency_ms) as p95_server_ms,
    AVG(batch_size_bytes) as avg_batch_size,
    COUNT(*) as total_batches,
    SUM(CASE WHEN status = 'success' THEN 1 ELSE 0 END) as successful,
    SUM(CASE WHEN status = 'failed' THEN 1 ELSE 0 END) as failed
FROM telemetry_metrics
WHERE created_at > (strftime('%s', 'now') - 3600) * 1000;

-- Identify slow batches (outliers)
SELECT 
    batch_id, 
    device_id, 
    patient_mrn,
    end_to_end_latency_ms,
    network_latency_ms,
    server_processing_latency_ms,
    batch_size_bytes,
    record_count
FROM telemetry_metrics
WHERE end_to_end_latency_ms > 100
  AND created_at > (strftime('%s', 'now') - 86400) * 1000
ORDER BY end_to_end_latency_ms DESC
LIMIT 50;

-- Performance trends over time (hourly buckets for last 24 hours)
SELECT 
    strftime('%Y-%m-%d %H:00:00', created_at / 1000, 'unixepoch') as hour,
    AVG(end_to_end_latency_ms) as avg_latency,
    MIN(end_to_end_latency_ms) as min_latency,
    MAX(end_to_end_latency_ms) as max_latency,
    PERCENTILE_CONT(0.95) WITHIN GROUP (ORDER BY end_to_end_latency_ms) as p95_latency,
    COUNT(*) as batch_count,
    SUM(batch_size_bytes) as total_bytes,
    SUM(CASE WHEN status = 'success' THEN 1 ELSE 0 END) * 100.0 / COUNT(*) as success_rate
FROM telemetry_metrics
WHERE created_at > (strftime('%s', 'now') - 86400) * 1000
GROUP BY hour
ORDER BY hour;

-- Latency breakdown by stage (where is the bottleneck?)
SELECT 
    AVG(batch_creation_latency_ms) as avg_batch_creation_ms,
    AVG(signing_latency_ms) as avg_signing_ms,
    AVG(queue_wait_latency_ms) as avg_queue_wait_ms,
    AVG(network_latency_ms) as avg_network_ms,
    AVG(server_processing_latency_ms) as avg_server_processing_ms,
    COUNT(*) as sample_count
FROM telemetry_metrics
WHERE created_at > (strftime('%s', 'now') - 3600) * 1000
  AND status = 'success';

-- Performance by latency class (distribution)
SELECT 
    latency_class,
    COUNT(*) as count,
    COUNT(*) * 100.0 / (SELECT COUNT(*) FROM telemetry_metrics) as percentage,
    AVG(end_to_end_latency_ms) as avg_latency_ms
FROM telemetry_metrics
WHERE created_at > (strftime('%s', 'now') - 86400) * 1000
GROUP BY latency_class
ORDER BY avg_latency_ms;
```

**Recommended Indices:**

```sql
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_batch ON telemetry_metrics(batch_id);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_device ON telemetry_metrics(device_id, created_at);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_patient ON telemetry_metrics(patient_mrn, created_at);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_status ON telemetry_metrics(status, created_at);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_latency ON telemetry_metrics(end_to_end_latency_ms) WHERE end_to_end_latency_ms IS NOT NULL;
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_latency_class ON telemetry_metrics(latency_class, created_at);
CREATE INDEX IF NOT EXISTS idx_telemetry_metrics_created_at ON telemetry_metrics(created_at);
```

**Retention Policy:**
- Keep detailed metrics for 90 days
- Archive aggregated statistics (hourly/daily) for 1 year
- Critical for compliance audits and performance analysis

**Trigger for Automatic Latency Classification:**

```sql
CREATE TRIGGER IF NOT EXISTS trg_telemetry_metrics_classify
AFTER INSERT ON telemetry_metrics
FOR EACH ROW
WHEN NEW.end_to_end_latency_ms IS NOT NULL
BEGIN
    UPDATE telemetry_metrics
    SET latency_class = CASE
        WHEN NEW.end_to_end_latency_ms < 10 THEN 'excellent'
        WHEN NEW.end_to_end_latency_ms < 50 THEN 'good'
        WHEN NEW.end_to_end_latency_ms < 100 THEN 'acceptable'
        ELSE 'slow'
    END
    WHERE id = NEW.id;
END;
```

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

See [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) for complete ADT workflow documentation.

## 10. Next Steps

- (A) Add an OpenAPI/JSON Schema for `/api/telemetry` in `doc/openapi/telemetry.yaml`.
- (B) Implement `DatabaseManager::cleanupOldData` and `DataArchiver::archiveData` in `src/core`.
- (C) Add migration SQL files under `doc/migrations/` to apply schema changes incrementally.

```
