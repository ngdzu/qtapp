-- Migration: 0001_5_base_tables.sql
-- Description: Create core base tables (patients, vitals, telemetry, alarms, etc.)
-- Date: 2025-11-30
--
-- This migration creates the core tables that were missing from the resource-bundled 0001_initial.sql

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
    admission_source TEXT CHECK (admission_source IS NULL OR admission_source IN ('manual','barcode','central_station','emergency','transfer')),
    admission_status TEXT DEFAULT 'DISCHARGED' CHECK (admission_status IN ('ADMITTED','DISCHARGED','TRANSFERRED')),
    device_label TEXT
);

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
    admission_source TEXT CHECK (admission_source IS NULL OR admission_source IN ('manual','barcode','central_station','emergency','transfer')),
    admission_status TEXT DEFAULT 'DISCHARGED' CHECK (admission_status IN ('ADMITTED','DISCHARGED','TRANSFERRED')),
    device_label TEXT
);

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
    status TEXT NOT NULL,
    error_message TEXT
);

CREATE TABLE IF NOT EXISTS vitals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT,
    timestamp INTEGER NOT NULL,
    timestamp_iso TEXT,
    patient_id TEXT,
    patient_mrn TEXT NOT NULL,
    device_id TEXT,
    device_label TEXT,
    heart_rate REAL,
    spo2 REAL,
    respiration_rate REAL,
    signal_quality INTEGER,
    sample_rate_hz REAL,
    source TEXT,
    is_synced BOOLEAN DEFAULT 0,
    batch_id TEXT
);

CREATE TABLE IF NOT EXISTS alarms (
    alarm_id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id TEXT,
    patient_mrn TEXT NOT NULL,
    start_time INTEGER NOT NULL,
    end_time INTEGER,
    alarm_type TEXT NOT NULL,
    priority TEXT NOT NULL,
    status TEXT
);

CREATE TABLE IF NOT EXISTS admission_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    patient_mrn TEXT NOT NULL,
    device_label TEXT
);

CREATE TABLE IF NOT EXISTS action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp_ms INTEGER NOT NULL,
    timestamp_iso TEXT NOT NULL,
    user_id TEXT,
    action_type TEXT NOT NULL,
    target_type TEXT,
    target_id TEXT,
    result TEXT NOT NULL,
    device_id TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    role TEXT NOT NULL,
    pin_hash TEXT NOT NULL,
    created_at INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS security_audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    event_category TEXT,
    severity TEXT NOT NULL,
    device_id TEXT
);
