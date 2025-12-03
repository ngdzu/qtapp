-- Auto-generated DDL for table creation
-- Generated: 2025-12-03 00:53:01 UTC
-- Schema Version: 1.0.0
-- Schema Source Hash: 7d3743f3a635
-- ⚠️ DO NOT EDIT MANUALLY - Edit schema/database.yaml and regenerate


-- Immutable audit trail for all user actions with hash chain for tamper detection
CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    timestamp_ms INTEGER NOT NULL  -- Unix timestamp in milliseconds,
    timestamp_iso TEXT NOT NULL  -- ISO 8601 timestamp for readability,
    user_id TEXT  -- User who performed action (NULL if no login required),
    user_role TEXT  -- User role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR),
    action_type TEXT NOT NULL  -- Action type (LOGIN, LOGOUT, AUTO_LOGOUT, ADMIT_PATIENT, etc.),
    target_type TEXT  -- Type of target (PATIENT, SETTING, NOTIFICATION, etc.),
    target_id TEXT  -- Target identifier (MRN, setting name, notification ID),
    details TEXT  -- JSON string with additional context,
    result TEXT NOT NULL CHECK (result IN ('SUCCESS', 'FAILURE', 'PARTIAL'))  -- SUCCESS, FAILURE, PARTIAL,
    error_code TEXT  -- Error code if result is FAILURE,
    error_message TEXT  -- Error message if result is FAILURE,
    device_id TEXT NOT NULL  -- Device identifier,
    session_token_hash TEXT  -- SHA-256 hash of session token (for audit trail),
    ip_address TEXT  -- IP address (if available, for network actions),
    previous_hash TEXT  -- SHA-256 hash of previous entry (hash chain for tamper detection)
);

-- Patient admission, discharge, and transfer events for audit and compliance
CREATE TABLE IF NOT EXISTS admission_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    timestamp INTEGER NOT NULL  -- Unix milliseconds timestamp of event,
    event_type TEXT NOT NULL CHECK (event_type IN ('admission', 'discharge', 'transfer'))  -- Type of event (admission/discharge/transfer),
    patient_mrn TEXT NOT NULL  -- Medical Record Number,
    patient_name TEXT  -- Patient name,
    device_label TEXT NOT NULL  -- Device identifier/asset tag that performed the action,
    bed_location TEXT  -- Bed/room location associated with the event,
    admission_source TEXT  -- Source of admission (manual/barcode/central_station/transfer),
    user_id TEXT  -- Clinician/Technician who performed the action (NULL if automated),
    details TEXT  -- Additional event details (JSON format)
);

-- Alarm events and lifecycle metadata
CREATE TABLE IF NOT EXISTS alarms (
    alarm_id TEXT PRIMARY KEY  -- Primary key (UUID),
    patient_id TEXT  -- Patient identifier (deprecated, use patient_mrn),
    patient_mrn TEXT NOT NULL  -- Medical Record Number (REQUIRED for patient association),
    start_time INTEGER NOT NULL  -- Unix milliseconds timestamp when alarm started,
    end_time INTEGER  -- Unix milliseconds timestamp when alarm ended,
    alarm_type TEXT NOT NULL  -- Type of alarm (HR_HIGH, SPO2_LOW, etc.),
    priority TEXT NOT NULL CHECK (priority IN ('CRITICAL', 'HIGH', 'MEDIUM', 'LOW'))  -- Alarm priority (CRITICAL/HIGH/MEDIUM/LOW),
    status TEXT  -- Alarm status (ACTIVE/ACKNOWLEDGED/SILENCED/CLEARED),
    acknowledged_by TEXT  -- User ID who acknowledged the alarm,
    acknowledged_time INTEGER  -- Unix milliseconds timestamp when alarm was acknowledged,
    silenced_until INTEGER  -- Unix milliseconds timestamp until which alarm is silenced,
    raw_value REAL  -- Raw value that triggered the alarm,
    threshold_value REAL  -- Threshold that was exceeded (historical snapshot for audit),
    context_snapshot_id INTEGER  -- Reference to snapshot table (if applicable)
);

-- User-added notes for specific snapshots or records
CREATE TABLE IF NOT EXISTS annotations (
    annotation_id INTEGER PRIMARY KEY AUTOINCREMENT,
    snapshot_id INTEGER NOT NULL,
    author_id TEXT,
    sensitivity TEXT,
    note TEXT NOT NULL,
    creation_time INTEGER NOT NULL
);

-- Tracks archival/export jobs and their status
CREATE TABLE IF NOT EXISTS archival_jobs (
    archive_id INTEGER PRIMARY KEY AUTOINCREMENT,
    table_name TEXT NOT NULL,
    record_count INTEGER NOT NULL,
    archived_before_ts INTEGER NOT NULL,
    archive_location TEXT,
    archived_at INTEGER NOT NULL,
    status TEXT NOT NULL
);

-- Certificate metadata for installed device/server certs (no private keys stored)
CREATE TABLE IF NOT EXISTS certificates (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    device_id TEXT NOT NULL  -- Device identifier this certificate belongs to,
    cert_serial TEXT NOT NULL  -- Certificate serial number (unique identifier),
    cert_subject TEXT  -- Certificate subject (CN, O, etc.),
    cert_issuer TEXT  -- Certificate issuer (CA information),
    issued_at INTEGER  -- Unix milliseconds timestamp when certificate was issued,
    expires_at INTEGER  -- Unix milliseconds timestamp when certificate expires,
    status TEXT NOT NULL CHECK (status IN ('active', 'expired', 'revoked', 'pending_renewal'))  -- Certificate status (active/expired/revoked/pending_renewal),
    last_validated_at INTEGER  -- Unix milliseconds timestamp of last successful certificate validation,
    revocation_reason TEXT  -- Reason for revocation if revoked (e.g., compromised, key_rotation),
    revoked_at INTEGER  -- Unix milliseconds timestamp when certificate was revoked,
    cert_fingerprint TEXT  -- SHA-256 fingerprint of certificate for quick lookup,
    created_at INTEGER NOT NULL  -- Unix milliseconds timestamp when record was created
);

-- Metadata about DB encryption key and algorithm (not the key itself)
CREATE TABLE IF NOT EXISTS db_encryption_meta (
    key_version TEXT,
    algorithm TEXT,
    salt TEXT,
    created_at INTEGER
);

-- Device health and telemetry events
CREATE TABLE IF NOT EXISTS device_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    device_id TEXT NOT NULL,
    battery_percent REAL,
    cpu_temp_c REAL,
    mem_percent REAL,
    network_latency_ms REAL,
    event_type TEXT,
    details TEXT
);

-- Detailed infusion pump logging (start/stop/pause/errors)
CREATE TABLE IF NOT EXISTS infusion_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id TEXT NOT NULL,
    timestamp INTEGER NOT NULL,
    drug_name TEXT,
    flow_rate_ml_per_hr REAL,
    total_volume_infused_ml REAL,
    time_remaining_sec INTEGER,
    occlusion_pressure_mmHg REAL,
    event_type TEXT NOT NULL,
    is_synced BOOLEAN DEFAULT 0
);

-- UI notifications history for NotificationBell and audit
CREATE TABLE IF NOT EXISTS notifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL,
    is_read BOOLEAN DEFAULT 0,
    related_alarm_id INTEGER,
    expiration INTEGER
);

-- Patient demographic and admission information. Serves as both local patient registry and cache for external system lookups.
CREATE TABLE IF NOT EXISTS patients (
    mrn TEXT PRIMARY KEY NOT NULL  -- Medical Record Number (unique patient identifier, primary key),
    name TEXT NOT NULL  -- Patient full name,
    dob TEXT  -- Date of birth (ISO 8601 format: YYYY-MM-DD),
    sex TEXT CHECK (sex IS NULL OR sex IN ('M', 'F', 'O', 'U'))  -- Biological sex (M/F/O/U),
    allergies TEXT  -- Known allergies (comma-separated or JSON),
    room TEXT  -- Legacy room/bed assignment (deprecated, use bed_location instead),
    created_at INTEGER NOT NULL  -- Unix milliseconds timestamp when patient record was created,
    last_lookup_at INTEGER  -- Unix milliseconds timestamp of last HIS lookup (for caching),
    lookup_source TEXT  -- Source of last lookup (HIS/Mock/Manual),
    bed_location TEXT  -- Current bed/room assignment (e.g., ICU-4B) - part of ADT workflow,
    admitted_at INTEGER  -- Unix milliseconds timestamp when patient was admitted to device (NULL if not currently admitted),
    discharged_at INTEGER  -- Unix milliseconds timestamp when patient was discharged (NULL if currently admitted),
    admission_source TEXT CHECK (admission_source IS NULL OR admission_source IN ('manual', 'barcode', 'central_station', 'emergency', 'transfer'))  -- Source of admission (Manual/Barcode/Central/Emergency),
    admission_status TEXT DEFAULT 'DISCHARGED' CHECK (admission_status IN ('ADMITTED', 'DISCHARGED', 'TRANSFERRED'))  -- Admission status (ADMITTED/DISCHARGED/TRANSFERRED),
    device_label TEXT  -- Device identifier that admitted this patient (e.g., ICU-MON-04),
    CONSTRAINT chk_admission_dates CHECK (admitted_at IS NULL OR discharged_at IS NULL OR discharged_at >= admitted_at)
);

-- Predictive analytics outputs and provenance
CREATE TABLE IF NOT EXISTS predictive_scores (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    patient_id TEXT,
    score_type TEXT NOT NULL,
    score_value REAL NOT NULL,
    model_version TEXT,
    details TEXT
);

-- Security-relevant events for audit, forensics, and compliance with hash chain for tamper detection
CREATE TABLE IF NOT EXISTS security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    timestamp INTEGER NOT NULL  -- Unix milliseconds timestamp,
    event_type TEXT NOT NULL  -- Event type (authentication, connection, data_transmission, etc.),
    severity TEXT NOT NULL CHECK (severity IN ('info', 'warning', 'error', 'critical'))  -- Severity level (info/warning/error/critical),
    device_id TEXT  -- Device identifier,
    user_id TEXT  -- User ID (if applicable),
    source_ip TEXT  -- Source IP address,
    event_category TEXT NOT NULL  -- Event category (authentication/connection/data_transmission/certificate/authorization/security_config),
    success BOOLEAN NOT NULL  -- Whether operation succeeded,
    details TEXT  -- Additional event details (JSON),
    error_code TEXT  -- Error code if success is false,
    error_message TEXT  -- Error message if success is false,
    previous_hash TEXT  -- SHA-256 hash of previous entry (hash chain for tamper detection)
);

-- Device configuration settings and user preferences
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY  -- Setting key (e.g., deviceId, deviceLabel, measurementUnit),
    value TEXT NOT NULL  -- Setting value,
    updated_at INTEGER NOT NULL  -- Unix milliseconds timestamp when setting was last updated,
    updated_by TEXT  -- User ID who updated the setting (NULL if system/default)
);

-- Captured waveform or contextual snapshot data (blobs) with metadata
CREATE TABLE IF NOT EXISTS snapshots (
    snapshot_id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id TEXT NOT NULL,
    capture_time INTEGER NOT NULL,
    waveform_type TEXT NOT NULL,
    data BLOB NOT NULL,
    encoding TEXT,
    sample_rate REAL,
    length INTEGER,
    compression TEXT,
    checksum TEXT
);

-- Telemetry transmission timing and performance metrics for benchmarking and diagnostics
CREATE TABLE IF NOT EXISTS telemetry_metrics (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    batch_id TEXT NOT NULL UNIQUE  -- Unique batch identifier (UUID, referenced by vitals.batch_id),
    device_id TEXT NOT NULL  -- Device serial number,
    device_label TEXT  -- Device asset tag,
    patient_mrn TEXT  -- Patient MRN (NULL if no patient admitted),
    data_created_at INTEGER NOT NULL  -- Timestamp of oldest data point in batch (Unix milliseconds),
    batch_created_at INTEGER NOT NULL  -- Timestamp when batch object was created (Unix milliseconds),
    signed_at INTEGER  -- Timestamp when batch was digitally signed (Unix milliseconds),
    queued_for_tx_at INTEGER  -- Timestamp when batch was queued for transmission (Unix milliseconds),
    transmitted_at INTEGER  -- Timestamp when transmission started (Unix milliseconds),
    server_received_at INTEGER  -- Timestamp when server received batch (Unix milliseconds),
    server_processed_at INTEGER  -- Timestamp when server processed batch (Unix milliseconds),
    server_ack_at INTEGER  -- Timestamp when server sent acknowledgment (Unix milliseconds),
    batch_creation_latency_ms INTEGER  -- Computed: batch_created_at - data_created_at,
    signing_latency_ms INTEGER  -- Computed: signed_at - batch_created_at,
    queue_wait_latency_ms INTEGER  -- Computed: transmitted_at - queued_for_tx_at,
    network_latency_ms INTEGER  -- Computed: server_received_at - transmitted_at,
    server_processing_latency_ms INTEGER  -- Computed: server_processed_at - server_received_at,
    end_to_end_latency_ms INTEGER  -- Computed: server_ack_at - data_created_at,
    record_count INTEGER  -- Number of records in batch,
    batch_size_bytes INTEGER  -- Size of raw payload in bytes,
    compressed_size_bytes INTEGER  -- Size after compression (if applicable),
    status TEXT NOT NULL CHECK (status IN ('success', 'failed', 'timeout', 'retrying'))  -- Status (success/failed/timeout/retrying),
    error_message TEXT  -- Error message if status is failed,
    retry_count INTEGER DEFAULT 0  -- Number of retry attempts,
    latency_class TEXT CHECK (latency_class IS NULL OR latency_class IN ('excellent', 'good', 'acceptable', 'slow'))  -- Latency classification (excellent/good/acceptable/slow),
    created_at INTEGER NOT NULL  -- Timestamp when metrics record was created (Unix milliseconds),
    updated_at INTEGER  -- Timestamp when metrics record was last updated (Unix milliseconds)
);

-- Local device accounts (PIN-based metadata) used for audit and RBAC
CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY  -- User identifier,
    username TEXT NOT NULL UNIQUE  -- Username (unique),
    role TEXT NOT NULL CHECK (role IN ('NURSE', 'PHYSICIAN', 'TECHNICIAN', 'ADMINISTRATOR', 'OBSERVER'))  -- User role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR, OBSERVER),
    pin_hash TEXT NOT NULL  -- Hashed PIN (never store plaintext),
    created_at INTEGER NOT NULL  -- Unix milliseconds timestamp when user was created,
    last_login INTEGER  -- Unix milliseconds timestamp of last login
);

-- Vital signs time-series data. Largest table with high-frequency writes.
CREATE TABLE IF NOT EXISTS vitals (
    id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
    uuid TEXT  -- UUID for distributed systems,
    timestamp INTEGER NOT NULL  -- Unix milliseconds when data was measured,
    timestamp_iso TEXT  -- ISO 8601 timestamp (for readability),
    patient_id TEXT  -- DEPRECATED: Use patient_mrn instead,
    patient_mrn TEXT NOT NULL  -- Patient MRN (FK to patients.mrn, REQUIRED for patient association),
    device_id TEXT  -- Device serial number,
    device_label TEXT  -- Device asset tag (e.g., ICU-MON-04),
    heart_rate REAL CHECK (heart_rate IS NULL OR (heart_rate >= 0 AND heart_rate <= 300))  -- Heart rate in BPM,
    spo2 REAL CHECK (spo2 IS NULL OR (spo2 >= 0 AND spo2 <= 100))  -- SpO2 oxygen saturation (0-100%),
    respiration_rate REAL CHECK (respiration_rate IS NULL OR (respiration_rate >= 0 AND respiration_rate <= 100))  -- Respiration rate in breaths per minute,
    signal_quality INTEGER CHECK (signal_quality IS NULL OR (signal_quality >= 0 AND signal_quality <= 100))  -- Signal quality (0-100),
    sample_rate_hz REAL  -- Sample rate in Hz,
    source TEXT  -- Data source (Simulator/Device/Test),
    is_synced BOOLEAN DEFAULT 0  -- Whether data has been synced to server,
    batch_id TEXT  -- Telemetry batch ID (FK to telemetry_metrics.batch_id),
    FOREIGN KEY (patient_mrn) REFERENCES patients(mrn) ON DELETE CASCADE,
    FOREIGN KEY (batch_id) REFERENCES telemetry_metrics(batch_id) ON DELETE SET NULL ON UPDATE CASCADE
);
