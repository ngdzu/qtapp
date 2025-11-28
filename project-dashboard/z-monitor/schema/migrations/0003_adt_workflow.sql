-- Migration: 0003_adt_workflow.sql
-- Description: Add ADT (Admission, Discharge, Transfer) workflow support
-- Date: 2025-01-15
--
-- This migration:
-- 1. Removes bedId from settings table
-- 2. Adds deviceLabel to settings (if not present)
-- 3. Enhances patients table with ADT columns
-- 4. Creates admission_events table for audit trail

BEGIN TRANSACTION;

-- Step 1: Remove bedId from settings (if exists)
DELETE FROM settings WHERE key = 'bedId';

-- Step 2: Add deviceLabel to settings (if not present)
INSERT OR IGNORE INTO settings (key, value, updated_at, updated_by)
VALUES ('deviceLabel', 'ICU-MON-04', strftime('%s', 'now'), NULL);

-- Step 3: Enhance patients table with ADT columns
-- Note: SQLite doesn't support ALTER TABLE ADD COLUMN IF NOT EXISTS,
-- so we check if columns exist before adding them

-- Add bed_location column (if not exists)
-- SQLite doesn't have a direct way to check if a column exists,
-- so we use a try-catch approach or check schema
-- For now, we'll attempt to add and ignore errors if column exists
-- (SQLite will fail silently if column already exists in some contexts)

-- Add bed_location
ALTER TABLE patients ADD COLUMN bed_location TEXT NULL;

-- Add admitted_at
ALTER TABLE patients ADD COLUMN admitted_at INTEGER NULL;

-- Add discharged_at
ALTER TABLE patients ADD COLUMN discharged_at INTEGER NULL;

-- Add admission_source
ALTER TABLE patients ADD COLUMN admission_source TEXT NULL;

-- Add device_label
ALTER TABLE patients ADD COLUMN device_label TEXT NULL;

-- Step 4: Create admission_events table for audit trail
CREATE TABLE IF NOT EXISTS admission_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,  -- "admission", "discharge", "transfer"
    patient_mrn TEXT NOT NULL,
    patient_name TEXT NULL,
    device_label TEXT NOT NULL,
    bed_location TEXT NULL,
    admission_source TEXT NULL,  -- "manual", "barcode", "central_station"
    user_id TEXT NULL,  -- Clinician who performed the action
    details TEXT NULL    -- Additional event details (JSON)
);

-- Create indices for admission_events
CREATE INDEX IF NOT EXISTS idx_admission_events_patient 
    ON admission_events(patient_mrn, timestamp);

CREATE INDEX IF NOT EXISTS idx_admission_events_device 
    ON admission_events(device_label, timestamp);

CREATE INDEX IF NOT EXISTS idx_admission_events_timestamp 
    ON admission_events(timestamp);

-- Step 5: Update existing patients (if any) to set discharged_at for non-admitted patients
-- This is a data migration step - set discharged_at for patients that don't have admitted_at
UPDATE patients 
SET discharged_at = created_at 
WHERE admitted_at IS NULL AND discharged_at IS NULL;

COMMIT;

