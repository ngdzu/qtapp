-- Migration: 0003_adt_workflow.sql
-- Description: ADT (Admission, Discharge, Transfer) workflow data migration
-- Date: 2025-01-15
--
-- This migration handles data migrations for ADT workflow:
-- 1. Removes bedId from settings table (deprecated)
-- 2. Adds deviceLabel to settings (if not present)
-- 3. Updates existing patients to set discharged_at for non-admitted patients
--
-- Note: ADT schema columns (bed_location, admitted_at, etc.) are already
-- included in the initial schema (0001_initial.sql), so this migration
-- only handles data migrations and settings updates.

BEGIN TRANSACTION;

-- Step 1: Remove bedId from settings (deprecated, replaced by bed_location in patients table)
DELETE FROM settings WHERE key = 'bedId';

-- Step 2: Add deviceLabel to settings (if not present)
INSERT OR IGNORE INTO settings (key, value, updated_at, updated_by)
VALUES ('deviceLabel', 'ICU-MON-04', strftime('%s', 'now') * 1000, NULL);

-- Step 3: Update existing patients (if any) to set discharged_at for non-admitted patients
-- This is a data migration step - set discharged_at for patients that don't have admitted_at
UPDATE patients 
SET discharged_at = created_at 
WHERE admitted_at IS NULL AND discharged_at IS NULL;

-- Note: migrate.py script records this migration automatically

COMMIT;
