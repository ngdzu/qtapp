-- Migration: 0001_initial.sql
-- Description: Initial database schema creation
-- Date: 2025-01-15
-- 
-- This migration creates all base tables for the Z Monitor application.
-- Generated from schema/database.yaml - DO NOT EDIT MANUALLY
-- To update: Edit schema/database.yaml and regenerate this file
--
-- Schema Version: 1.0.0

-- -----------------------------------------------------------
-- PATIENTS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS patients (
    mrn TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    dob TEXT,
    sex TEXT CHECK (sex IS NULL OR sex IN ('M', 'F', 'O', 'U')),
    allergies TEXT,
    room TEXT,
    created_at INTEGER NOT NULL,
    last_lookup_at INTEGER,
    lookup_source TEXT,
    bed_location TEXT,
    admitted_at INTEGER,
    discharged_at INTEGER,
    admission_source TEXT CHECK (admission_source IS NULL OR admission_source IN ('manual', 'barcode', 'central_station', 'emergency', 'transfer')),
    admission_status TEXT DEFAULT 'DISCHARGED' CHECK (admission_status IN ('ADMITTED', 'DISCHARGED', 'TRANSFERRED')),
    device_label TEXT,
    CHECK (admitted_at IS NULL OR discharged_at IS NULL OR discharged_at >= admitted_at)
);

-- -----------------------------------------------------------
-- VITALS TABLE (Time-Series)
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS vitals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT,
    timestamp INTEGER NOT NULL,
    timestamp_iso TEXT,
    patient_id TEXT,
    patient_mrn TEXT NOT NULL,
    device_id TEXT,
    device_label TEXT,
    heart_rate REAL CHECK (heart_rate IS NULL OR (heart_rate >= 0 AND heart_rate <= 300)),
    spo2 REAL CHECK (spo2 IS NULL OR (spo2 >= 0 AND spo2 <= 100)),
    respiration_rate REAL CHECK (respiration_rate IS NULL OR (respiration_rate >= 0 AND respiration_rate <= 100)),
    signal_quality INTEGER CHECK (signal_quality IS NULL OR (signal_quality >= 0 AND signal_quality <= 100)),
    sample_rate_hz REAL,
    source TEXT,
    is_synced BOOLEAN DEFAULT 0,
    batch_id TEXT,
    FOREIGN KEY (patient_mrn) REFERENCES patients(mrn) ON DELETE CASCADE,
    FOREIGN KEY (batch_id) REFERENCES telemetry_metrics(batch_id) ON DELETE SET NULL ON UPDATE CASCADE
);

-- -----------------------------------------------------------
-- TELEMETRY METRICS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS telemetry_metrics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    batch_id TEXT NOT NULL UNIQUE,
    device_id TEXT NOT NULL,
    device_label TEXT,
    patient_mrn TEXT,
    data_created_at INTEGER NOT NULL,
    batch_created_at INTEGER NOT NULL,
    signed_at INTEGER,
    queued_for_tx_at INTEGER,
    transmitted_at INTEGER,
    server_received_at INTEGER,
    server_processed_at INTEGER,
    server_ack_at INTEGER,
    batch_creation_latency_ms INTEGER,
    signing_latency_ms INTEGER,
    queue_wait_latency_ms INTEGER,
    network_latency_ms INTEGER,
    server_processing_latency_ms INTEGER,
    end_to_end_latency_ms INTEGER,
    record_count INTEGER,
    batch_size_bytes INTEGER,
    compressed_size_bytes INTEGER,
    status TEXT NOT NULL CHECK (status IN ('success', 'failed', 'timeout', 'retrying')),
    error_message TEXT,
    retry_count INTEGER DEFAULT 0,
    latency_class TEXT CHECK (latency_class IS NULL OR latency_class IN ('excellent', 'good', 'acceptable', 'slow')),
    created_at INTEGER NOT NULL,
    updated_at INTEGER
);

-- -----------------------------------------------------------
-- ALARMS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS alarms (
    alarm_id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id TEXT,
    patient_mrn TEXT NOT NULL,
    start_time INTEGER NOT NULL,
    end_time INTEGER,
    alarm_type TEXT NOT NULL,
    priority TEXT NOT NULL CHECK (priority IN ('CRITICAL', 'HIGH', 'MEDIUM', 'LOW')),
    status TEXT,
    acknowledged_by TEXT,
    acknowledged_time INTEGER,
    silenced_until INTEGER,
    raw_value REAL,
    threshold_value REAL,
    context_snapshot_id INTEGER
);

-- -----------------------------------------------------------
-- ADMISSION EVENTS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS admission_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL CHECK (event_type IN ('admission', 'discharge', 'transfer')),
    patient_mrn TEXT NOT NULL,
    patient_name TEXT,
    device_label TEXT NOT NULL,
    bed_location TEXT,
    admission_source TEXT,
    user_id TEXT,
    details TEXT
);

-- -----------------------------------------------------------
-- ACTION LOG TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,
    timestamp_iso TEXT NOT NULL,
    user_id TEXT,
    user_role TEXT,
    action_type TEXT NOT NULL,
    target_type TEXT,
    target_id TEXT,
    details TEXT,
    result TEXT NOT NULL CHECK (result IN ('SUCCESS', 'FAILURE', 'PARTIAL')),
    error_code TEXT,
    error_message TEXT,
    device_id TEXT NOT NULL,
    session_token_hash TEXT,
    ip_address TEXT,
    previous_hash TEXT
);

-- -----------------------------------------------------------
-- SETTINGS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at INTEGER NOT NULL,
    updated_by TEXT
);

-- -----------------------------------------------------------
-- USERS TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('NURSE', 'PHYSICIAN', 'TECHNICIAN', 'ADMINISTRATOR', 'OBSERVER')),
    pin_hash TEXT NOT NULL,
    created_at INTEGER NOT NULL,
    last_login INTEGER
);

-- -----------------------------------------------------------
-- CERTIFICATES TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS certificates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    cert_serial TEXT NOT NULL,
    cert_subject TEXT,
    cert_issuer TEXT,
    issued_at INTEGER,
    expires_at INTEGER,
    status TEXT NOT NULL CHECK (status IN ('active', 'expired', 'revoked', 'pending_renewal')),
    last_validated_at INTEGER,
    revocation_reason TEXT,
    revoked_at INTEGER,
    cert_fingerprint TEXT,
    created_at INTEGER NOT NULL
);

-- -----------------------------------------------------------
-- SECURITY AUDIT LOG TABLE
-- -----------------------------------------------------------
CREATE TABLE IF NOT EXISTS security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    severity TEXT NOT NULL CHECK (severity IN ('info', 'warning', 'error', 'critical')),
    device_id TEXT,
    user_id TEXT,
    source_ip TEXT,
    event_category TEXT NOT NULL,
    success BOOLEAN NOT NULL,
    details TEXT,
    error_code TEXT,
    error_message TEXT,
    previous_hash TEXT
);

-- -----------------------------------------------------------
-- ADDITIONAL TABLES
-- -----------------------------------------------------------

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

CREATE TABLE IF NOT EXISTS annotations (
    annotation_id INTEGER PRIMARY KEY AUTOINCREMENT,
    snapshot_id INTEGER NOT NULL,
    author_id TEXT,
    sensitivity TEXT,
    note TEXT NOT NULL,
    creation_time INTEGER NOT NULL
);

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

CREATE TABLE IF NOT EXISTS notifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL,
    is_read BOOLEAN DEFAULT 0,
    related_alarm_id INTEGER,
    expiration INTEGER
);

CREATE TABLE IF NOT EXISTS predictive_scores (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    patient_id TEXT,
    score_type TEXT NOT NULL,
    score_value REAL NOT NULL,
    model_version TEXT,
    details TEXT
);

CREATE TABLE IF NOT EXISTS archival_jobs (
    archive_id INTEGER PRIMARY KEY AUTOINCREMENT,
    table_name TEXT NOT NULL,
    record_count INTEGER NOT NULL,
    archived_before_ts INTEGER NOT NULL,
    archive_location TEXT,
    archived_at INTEGER NOT NULL,
    status TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS db_encryption_meta (
    key_version TEXT,
    algorithm TEXT,
    salt TEXT,
    created_at INTEGER
);

-- Create schema version tracking table
-- Note: migrate.py script manages version tracking automatically
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL,
    description TEXT,
    migration_type TEXT DEFAULT 'schema'
);

