# Data Migration Workflow

This document defines the complete migration workflow for Z Monitor, covering schema migrations, data migrations, rollback procedures, testing strategies, and deployment workflows.

> **📊 Migration Workflow Diagram:**  
> [View Migration Workflow (Mermaid)](./34_DATA_MIGRATION_WORKFLOW.mmd)  
> [View Migration Workflow (SVG)](./34_DATA_MIGRATION_WORKFLOW.svg)

---

## 1. Overview

### **Types of Migrations**

| Type | Description | When to Use | Risk Level |
|------|-------------|-------------|------------|
| **Schema Migration** | Changes to table structure (DDL) | Adding/removing columns, tables, indices | ⚠️ Medium |
| **Data Migration** | Transform existing data | Data format changes, normalization | 🔴 High |
| **Seed Migration** | Insert initial/reference data | Default settings, lookup tables | 🟢 Low |
| **Rollback Migration** | Undo previous migration | Production issues, failed deployments | 🔴 High |

---

## 2. Migration File Structure

### **Directory Organization**

```
z-monitor/schema/
├── database.yaml                    # Schema definition (source of truth)
├── migrations/
│   ├── schema/                      # DDL migrations (CREATE, ALTER, DROP)
│   │   ├── 0001_initial_schema.sql
│   │   ├── 0002_add_adt_workflow.sql
│   │   ├── 0003_add_telemetry_metrics.sql
│   │   └── 0004_add_emergency_contact.sql
│   ├── data/                        # DML migrations (INSERT, UPDATE, DELETE)
│   │   ├── 0005_migrate_patient_ids_to_mrn.sql
│   │   ├── 0006_normalize_admission_data.sql
│   │   └── 0007_backfill_device_labels.sql
│   ├── seed/                        # Initial/reference data
│   │   ├── 0001_default_settings.sql
│   │   └── 0002_alarm_thresholds.sql
│   └── rollback/                    # Rollback scripts
│       ├── 0005_rollback.sql        # Rollback for migration 0005
│       └── 0006_rollback.sql        # Rollback for migration 0006
└── generated/
    └── ddl/                         # Auto-generated from YAML
        ├── create_tables.sql
        └── create_indices.sql
```

### **Migration File Naming Convention**

```
<number>_<type>_<description>.sql

Examples:
0001_schema_initial_schema.sql
0002_schema_add_adt_workflow.sql
0005_data_migrate_patient_ids_to_mrn.sql
0007_data_backfill_device_labels.sql
0010_seed_default_settings.sql
```

**Numbering:**
- Start at `0001`
- Increment sequentially
- **Never reuse or skip numbers**
- **Never modify an applied migration** (create new migration instead)

---

## 3. Schema Migrations (DDL)

### **3.1 Adding a Column**

**File:** `schema/migrations/schema/0004_add_emergency_contact.sql`

```sql
-- Migration: Add emergency_contact column to patients table
-- Version: 0004
-- Date: 2025-11-27
-- Author: Dev Team
-- Description: Add emergency contact field for patient records

-- Migration UP
BEGIN TRANSACTION;

-- Add new column
ALTER TABLE patients 
ADD COLUMN emergency_contact TEXT NULL;

-- Add comment (SQLite doesn't support column comments, document here)
-- emergency_contact: Emergency contact phone number (E.164 format)

-- Update schema version
INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (4, datetime('now'), 'Add emergency_contact column', 'schema');

COMMIT;
```

**Rollback File:** `schema/migrations/rollback/0004_rollback.sql`

```sql
-- Rollback: Remove emergency_contact column
-- Rolling back version: 0004

BEGIN TRANSACTION;

-- SQLite doesn't support DROP COLUMN directly
-- Need to recreate table without the column

-- 1. Create new table without emergency_contact
CREATE TABLE patients_new (
    mrn TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    dob TEXT NULL,
    sex TEXT NULL,
    bed_location TEXT NULL,
    admission_status TEXT DEFAULT 'DISCHARGED',
    admitted_at INTEGER NULL,
    discharged_at INTEGER NULL,
    admission_source TEXT NULL,
    last_lookup_at INTEGER NULL,
    lookup_source TEXT NULL
);

-- 2. Copy data (exclude emergency_contact)
INSERT INTO patients_new 
SELECT mrn, name, dob, sex, bed_location, admission_status, 
       admitted_at, discharged_at, admission_source, 
       last_lookup_at, lookup_source
FROM patients;

-- 3. Drop old table
DROP TABLE patients;

-- 4. Rename new table
ALTER TABLE patients_new RENAME TO patients;

-- 5. Recreate indices
CREATE UNIQUE INDEX idx_patients_mrn ON patients(mrn);
CREATE INDEX idx_patients_admission_status ON patients(admission_status);

-- 6. Update schema version (remove this version)
DELETE FROM schema_version WHERE version = 4;

COMMIT;
```

### **3.2 Adding a Table**

**File:** `schema/migrations/schema/0008_add_alarm_history.sql`

```sql
-- Migration: Add alarm_history table for long-term alarm storage
-- Version: 0008

BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS alarm_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    alarm_id INTEGER NOT NULL,              -- Original alarm ID
    timestamp INTEGER NOT NULL,             -- Unix milliseconds
    patient_mrn TEXT NOT NULL,
    alarm_type TEXT NOT NULL,
    severity TEXT NOT NULL,
    acknowledged_at INTEGER NULL,
    acknowledged_by TEXT NULL,
    resolved_at INTEGER NULL,
    archived_at INTEGER NOT NULL,           -- When moved to history
    
    FOREIGN KEY (patient_mrn) REFERENCES patients(mrn) ON DELETE CASCADE
);

CREATE INDEX idx_alarm_history_patient_mrn ON alarm_history(patient_mrn);
CREATE INDEX idx_alarm_history_timestamp ON alarm_history(timestamp);
CREATE INDEX idx_alarm_history_archived_at ON alarm_history(archived_at);

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (8, datetime('now'), 'Add alarm_history table', 'schema');

COMMIT;
```

### **3.3 Modifying a Column (SQLite Limitations)**

SQLite doesn't support `ALTER COLUMN` directly. Must recreate table.

**File:** `schema/migrations/schema/0009_change_mrn_length.sql`

```sql
-- Migration: Change MRN column to support longer IDs
-- Version: 0009
-- Note: SQLite requires table recreation for column type changes

BEGIN TRANSACTION;

-- 1. Create new table with modified column
CREATE TABLE patients_new (
    mrn TEXT PRIMARY KEY NOT NULL CHECK(length(mrn) <= 50),  -- NEW: increased limit
    name TEXT NOT NULL,
    dob TEXT NULL,
    sex TEXT NULL CHECK(sex IN ('M', 'F', 'O', 'U')),
    bed_location TEXT NULL,
    admission_status TEXT DEFAULT 'DISCHARGED',
    admitted_at INTEGER NULL,
    discharged_at INTEGER NULL,
    admission_source TEXT NULL,
    last_lookup_at INTEGER NULL,
    lookup_source TEXT NULL,
    emergency_contact TEXT NULL
);

-- 2. Copy all data
INSERT INTO patients_new SELECT * FROM patients;

-- 3. Drop old table
DROP TABLE patients;

-- 4. Rename new table
ALTER TABLE patients_new RENAME TO patients;

-- 5. Recreate all indices
CREATE UNIQUE INDEX idx_patients_mrn ON patients(mrn);
CREATE INDEX idx_patients_admission_status ON patients(admission_status);
CREATE INDEX idx_patients_bed_location ON patients(bed_location) 
    WHERE bed_location IS NOT NULL;

-- 6. Update schema version
INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (9, datetime('now'), 'Change MRN column length to 50', 'schema');

COMMIT;
```

---

## 4. Data Migrations (DML)

### **4.1 Data Transformation**

**File:** `schema/migrations/data/0005_migrate_patient_ids_to_mrn.sql`

```sql
-- Data Migration: Convert old patient_id to new patient_mrn format
-- Version: 0005
-- Risk: HIGH - touches all vitals records
-- Estimated time: 5-10 minutes for 1M records

BEGIN TRANSACTION;

-- Step 1: Verify data integrity before migration
-- Check if any vitals have NULL patient_id
SELECT COUNT(*) FROM vitals WHERE patient_id IS NULL;
-- If count > 0, ABORT and investigate

-- Step 2: Create temporary mapping table
CREATE TEMP TABLE patient_id_mrn_map AS
SELECT DISTINCT patient_id, patient_mrn 
FROM vitals 
WHERE patient_mrn IS NOT NULL AND patient_id IS NOT NULL;

-- Step 3: Update vitals table (batch update)
-- For SQLite, do this in chunks to avoid locking issues
UPDATE vitals
SET patient_mrn = (
    SELECT patient_mrn 
    FROM patients 
    WHERE patients.mrn = vitals.patient_id
)
WHERE patient_mrn IS NULL AND patient_id IS NOT NULL;

-- Step 4: Verify migration success
-- All vitals should now have patient_mrn
SELECT COUNT(*) FROM vitals WHERE patient_mrn IS NULL;
-- Expected: 0

-- Step 5: Update schema version
INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (5, datetime('now'), 'Migrate patient_id to patient_mrn', 'data');

COMMIT;

-- Step 6: Post-migration validation (run separately)
-- SELECT COUNT(*) FROM vitals WHERE patient_mrn IS NULL;
-- SELECT COUNT(*) FROM vitals WHERE patient_id != patient_mrn;
```

**Rollback File:** `schema/migrations/rollback/0005_rollback.sql`

```sql
-- Rollback: Restore patient_id values
-- Note: Only works if patient_id column still exists

BEGIN TRANSACTION;

-- Restore patient_id from patient_mrn
UPDATE vitals
SET patient_id = patient_mrn
WHERE patient_id IS NULL AND patient_mrn IS NOT NULL;

-- Remove version
DELETE FROM schema_version WHERE version = 5;

COMMIT;
```

### **4.2 Data Normalization**

**File:** `schema/migrations/data/0006_normalize_admission_data.sql`

```sql
-- Data Migration: Normalize admission data into admission_events table
-- Version: 0006
-- Risk: MEDIUM - creates new records, doesn't modify existing

BEGIN TRANSACTION;

-- Insert historical admission events from patients table
INSERT INTO admission_events (
    event_type,
    timestamp,
    patient_mrn,
    device_id,
    device_label,
    user_id,
    bed_location,
    source,
    notes,
    created_at
)
SELECT 
    'ADMISSION' as event_type,
    admitted_at as timestamp,
    mrn as patient_mrn,
    NULL as device_id,      -- Historical data may not have this
    NULL as device_label,
    NULL as user_id,        -- Historical data may not have this
    bed_location,
    admission_source as source,
    'Migrated from patients table' as notes,
    datetime('now') as created_at
FROM patients
WHERE admitted_at IS NOT NULL
  AND admission_status = 'ADMITTED'
  AND NOT EXISTS (
      -- Don't duplicate if already migrated
      SELECT 1 FROM admission_events ae 
      WHERE ae.patient_mrn = patients.mrn 
        AND ae.timestamp = patients.admitted_at
  );

-- Insert discharge events for patients who were discharged
INSERT INTO admission_events (
    event_type,
    timestamp,
    patient_mrn,
    device_id,
    device_label,
    user_id,
    bed_location,
    source,
    notes,
    created_at
)
SELECT 
    'DISCHARGE' as event_type,
    discharged_at as timestamp,
    mrn as patient_mrn,
    NULL as device_id,
    NULL as device_label,
    NULL as user_id,
    bed_location,
    'Manual' as source,
    'Migrated from patients table' as notes,
    datetime('now') as created_at
FROM patients
WHERE discharged_at IS NOT NULL
  AND admission_status = 'DISCHARGED'
  AND NOT EXISTS (
      SELECT 1 FROM admission_events ae 
      WHERE ae.patient_mrn = patients.mrn 
        AND ae.timestamp = patients.discharged_at
  );

-- Verify migration
SELECT event_type, COUNT(*) 
FROM admission_events 
WHERE notes LIKE '%Migrated%'
GROUP BY event_type;

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (6, datetime('now'), 'Normalize admission data into admission_events', 'data');

COMMIT;
```

### **4.3 Backfill Data**

**File:** `schema/migrations/data/0007_backfill_device_labels.sql`

```sql
-- Data Migration: Backfill device_label from device_id
-- Version: 0007
-- Risk: LOW - updates NULL values only

BEGIN TRANSACTION;

-- Backfill device_label in vitals table
-- Use device_id as fallback if no mapping exists
UPDATE vitals
SET device_label = (
    CASE 
        WHEN device_id LIKE 'SERIAL-%' THEN 'DEVICE-' || substr(device_id, 8, 3)
        ELSE 'UNKNOWN-' || device_id
    END
)
WHERE device_label IS NULL 
  AND device_id IS NOT NULL;

-- Backfill device_label in telemetry_metrics table
UPDATE telemetry_metrics
SET device_label = (
    SELECT device_label 
    FROM vitals 
    WHERE vitals.device_id = telemetry_metrics.device_id
    LIMIT 1
)
WHERE device_label IS NULL 
  AND device_id IS NOT NULL;

-- Verify backfill
SELECT 
    COUNT(*) as total_vitals,
    SUM(CASE WHEN device_label IS NULL THEN 1 ELSE 0 END) as null_labels
FROM vitals;

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (7, datetime('now'), 'Backfill device_label from device_id', 'data');

COMMIT;
```

---

## 5. Seed Migrations (Reference Data)

### **File:** `schema/migrations/seed/0001_default_settings.sql`

```sql
-- Seed Migration: Insert default application settings
-- Version: seed-0001

BEGIN TRANSACTION;

-- Insert default settings (only if not exists)
INSERT OR IGNORE INTO settings (key, value, updated_at, updated_by)
VALUES 
    ('deviceLabel', 'DEVICE-001', datetime('now'), 'system'),
    ('measurementUnit', 'metric', datetime('now'), 'system'),
    ('alarmVolume', '80', datetime('now'), 'system'),
    ('screenBrightness', '75', datetime('now'), 'system'),
    ('autoLogoutMinutes', '30', datetime('now'), 'system'),
    ('telemetryBatchSize', '100', datetime('now'), 'system'),
    ('telemetryIntervalMs', '5000', datetime('now'), 'system');

-- Insert default alarm thresholds
INSERT OR IGNORE INTO alarm_thresholds (
    parameter, 
    low_critical, low_warning, 
    high_warning, high_critical,
    created_at
)
VALUES 
    ('heart_rate', 40, 50, 120, 150, datetime('now')),
    ('spo2', 88, 92, NULL, NULL, datetime('now')),
    ('respiration_rate', 8, 10, 25, 30, datetime('now'));

COMMIT;
```

---

## 6. Migration Runner Implementation

### **File:** `z-monitor/scripts/migrate.py`

```python
#!/usr/bin/env python3
"""
Database Migration Runner

Applies schema, data, and seed migrations in order.
Tracks applied migrations in schema_version table.

Usage:
    python3 scripts/migrate.py --db /path/to/database.db
    python3 scripts/migrate.py --db /path/to/database.db --dry-run
    python3 scripts/migrate.py --db /path/to/database.db --rollback 5
"""

import sqlite3
import os
import argparse
import hashlib
from pathlib import Path
from datetime import datetime
from typing import List, Tuple

class MigrationRunner:
    def __init__(self, db_path: str):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self.migrations_dir = Path("schema/migrations")
        
    def initialize_schema_version_table(self):
        """Create schema_version table if it doesn't exist."""
        self.conn.execute("""
            CREATE TABLE IF NOT EXISTS schema_version (
                version INTEGER PRIMARY KEY,
                applied_at TEXT NOT NULL,
                description TEXT,
                migration_type TEXT,  -- 'schema', 'data', 'seed'
                file_hash TEXT,       -- SHA256 of migration file
                execution_time_ms INTEGER,
                applied_by TEXT DEFAULT 'migrate.py'
            )
        """)
        self.conn.commit()
        
    def get_current_version(self) -> int:
        """Get the highest applied migration version."""
        cursor = self.conn.cursor()
        cursor.execute("SELECT MAX(version) FROM schema_version")
        result = cursor.fetchone()
        return result[0] if result[0] is not None else 0
    
    def get_applied_migrations(self) -> List[int]:
        """Get list of all applied migration versions."""
        cursor = self.conn.cursor()
        cursor.execute("SELECT version FROM schema_version ORDER BY version")
        return [row[0] for row in cursor.fetchall()]
    
    def get_pending_migrations(self) -> List[Tuple[int, Path, str]]:
        """Get list of pending migrations.
        
        Returns:
            List of (version, file_path, migration_type) tuples
        """
        applied = set(self.get_applied_migrations())
        pending = []
        
        # Scan all migration subdirectories
        for migration_type in ['schema', 'data', 'seed']:
            migration_subdir = self.migrations_dir / migration_type
            if not migration_subdir.exists():
                continue
                
            for migration_file in sorted(migration_subdir.glob("*.sql")):
                # Extract version from filename (e.g., 0005_migrate_data.sql -> 5)
                try:
                    version_str = migration_file.stem.split('_')[0]
                    version = int(version_str)
                except (ValueError, IndexError):
                    print(f"⚠️  Skipping invalid migration filename: {migration_file.name}")
                    continue
                
                if version not in applied:
                    pending.append((version, migration_file, migration_type))
        
        # Sort by version number
        pending.sort(key=lambda x: x[0])
        return pending
    
    def calculate_file_hash(self, file_path: Path) -> str:
        """Calculate SHA256 hash of migration file."""
        sha256 = hashlib.sha256()
        with open(file_path, 'rb') as f:
            sha256.update(f.read())
        return sha256.hexdigest()
    
    def apply_migration(self, version: int, file_path: Path, migration_type: str, dry_run: bool = False):
        """Apply a single migration file."""
        print(f"\n{'[DRY RUN] ' if dry_run else ''}📦 Applying {migration_type} migration {version}: {file_path.name}")
        
        # Read migration file
        with open(file_path, 'r') as f:
            sql = f.read()
        
        if dry_run:
            print(f"   SQL Preview (first 200 chars):")
            print(f"   {sql[:200]}...")
            return
        
        # Calculate file hash
        file_hash = self.calculate_file_hash(file_path)
        
        # Execute migration
        start_time = datetime.now()
        try:
            cursor = self.conn.cursor()
            cursor.executescript(sql)
            
            execution_time_ms = int((datetime.now() - start_time).total_seconds() * 1000)
            
            # Record migration (if not already recorded by the migration itself)
            cursor.execute("""
                INSERT OR REPLACE INTO schema_version 
                (version, applied_at, description, migration_type, file_hash, execution_time_ms)
                VALUES (?, datetime('now'), ?, ?, ?, ?)
            """, (version, file_path.stem, migration_type, file_hash, execution_time_ms))
            
            self.conn.commit()
            print(f"   ✅ Migration {version} applied successfully ({execution_time_ms}ms)")
            
        except sqlite3.Error as e:
            self.conn.rollback()
            print(f"   ❌ Migration {version} failed: {e}")
            raise
    
    def rollback_migration(self, version: int, dry_run: bool = False):
        """Rollback a specific migration."""
        rollback_file = self.migrations_dir / "rollback" / f"{version:04d}_rollback.sql"
        
        if not rollback_file.exists():
            print(f"❌ Rollback file not found: {rollback_file}")
            return False
        
        print(f"\n{'[DRY RUN] ' if dry_run else ''}🔄 Rolling back migration {version}: {rollback_file.name}")
        
        with open(rollback_file, 'r') as f:
            sql = f.read()
        
        if dry_run:
            print(f"   SQL Preview (first 200 chars):")
            print(f"   {sql[:200]}...")
            return True
        
        try:
            cursor = self.conn.cursor()
            cursor.executescript(sql)
            self.conn.commit()
            print(f"   ✅ Migration {version} rolled back successfully")
            return True
            
        except sqlite3.Error as e:
            self.conn.rollback()
            print(f"   ❌ Rollback {version} failed: {e}")
            return False
    
    def run(self, dry_run: bool = False, target_version: int = None):
        """Run all pending migrations up to target version."""
        self.initialize_schema_version_table()
        
        current_version = self.get_current_version()
        print(f"📊 Current schema version: {current_version}")
        
        pending = self.get_pending_migrations()
        
        if target_version:
            pending = [(v, p, t) for v, p, t in pending if v <= target_version]
        
        if not pending:
            print("✅ Database is up to date!")
            return
        
        print(f"📋 Found {len(pending)} pending migration(s)")
        
        for version, file_path, migration_type in pending:
            self.apply_migration(version, file_path, migration_type, dry_run)
        
        if not dry_run:
            final_version = self.get_current_version()
            print(f"\n✅ Migration complete! Database version: {final_version}")
    
    def status(self):
        """Show migration status."""
        self.initialize_schema_version_table()
        
        current_version = self.get_current_version()
        applied = self.get_applied_migrations()
        pending = self.get_pending_migrations()
        
        print(f"📊 Migration Status")
        print(f"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        print(f"Current version: {current_version}")
        print(f"Applied migrations: {len(applied)}")
        print(f"Pending migrations: {len(pending)}")
        print()
        
        if applied:
            print("✅ Applied Migrations:")
            cursor = self.conn.cursor()
            cursor.execute("""
                SELECT version, applied_at, description, migration_type, execution_time_ms
                FROM schema_version 
                ORDER BY version
            """)
            for row in cursor.fetchall():
                version, applied_at, desc, mig_type, exec_time = row
                print(f"   {version:04d} - {mig_type:8s} - {desc:50s} ({exec_time}ms) - {applied_at}")
        
        if pending:
            print("\n⏳ Pending Migrations:")
            for version, file_path, migration_type in pending:
                print(f"   {version:04d} - {migration_type:8s} - {file_path.name}")
    
    def close(self):
        """Close database connection."""
        self.conn.close()

def main():
    parser = argparse.ArgumentParser(description='Database Migration Runner')
    parser.add_argument('--db', required=True, help='Path to database file')
    parser.add_argument('--dry-run', action='store_true', help='Show what would be executed without applying')
    parser.add_argument('--rollback', type=int, help='Rollback specific migration version')
    parser.add_argument('--status', action='store_true', help='Show migration status')
    parser.add_argument('--target', type=int, help='Migrate up to specific version')
    args = parser.parse_args()
    
    runner = MigrationRunner(args.db)
    
    try:
        if args.status:
            runner.status()
        elif args.rollback:
            runner.rollback_migration(args.rollback, args.dry_run)
        else:
            runner.run(args.dry_run, args.target)
    finally:
        runner.close()

if __name__ == "__main__":
    main()
```

---

## 7. Migration Testing Strategy

### **7.1 Pre-Migration Checks**

**File:** `z-monitor/scripts/pre_migration_check.py`

```python
#!/usr/bin/env python3
"""
Pre-Migration Validation

Checks database integrity before applying migrations.
"""

import sqlite3
import sys

def check_database(db_path: str) -> bool:
    """Run pre-migration checks."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    checks_passed = True
    
    print("🔍 Running Pre-Migration Checks...")
    
    # Check 1: Foreign key integrity
    print("\n1. Checking foreign key integrity...")
    cursor.execute("PRAGMA foreign_keys = ON")
    cursor.execute("PRAGMA foreign_key_check")
    fk_violations = cursor.fetchall()
    
    if fk_violations:
        print(f"   ❌ Found {len(fk_violations)} foreign key violations!")
        for violation in fk_violations:
            print(f"      {violation}")
        checks_passed = False
    else:
        print("   ✅ No foreign key violations")
    
    # Check 2: Database integrity
    print("\n2. Checking database integrity...")
    cursor.execute("PRAGMA integrity_check")
    result = cursor.fetchone()
    
    if result[0] != 'ok':
        print(f"   ❌ Database integrity check failed: {result[0]}")
        checks_passed = False
    else:
        print("   ✅ Database integrity OK")
    
    # Check 3: Disk space (if needed)
    print("\n3. Checking database size...")
    cursor.execute("SELECT page_count * page_size as size FROM pragma_page_count(), pragma_page_size()")
    size_bytes = cursor.fetchone()[0]
    size_mb = size_bytes / (1024 * 1024)
    print(f"   ℹ️  Database size: {size_mb:.2f} MB")
    
    # Check 4: Active connections/locks
    print("\n4. Checking for active transactions...")
    cursor.execute("SELECT COUNT(*) FROM pragma_database_list WHERE name != 'temp'")
    # Note: SQLite doesn't provide direct lock info, but we can check WAL
    
    conn.close()
    
    if checks_passed:
        print("\n✅ All pre-migration checks passed!")
    else:
        print("\n❌ Some checks failed. Review before migrating.")
    
    return checks_passed

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 pre_migration_check.py <database_path>")
        sys.exit(1)
    
    passed = check_database(sys.argv[1])
    sys.exit(0 if passed else 1)
```

### **7.2 Post-Migration Validation**

**File:** `z-monitor/scripts/post_migration_check.py`

```python
#!/usr/bin/env python3
"""
Post-Migration Validation

Validates database after migrations.
"""

import sqlite3
import sys

def validate_migration(db_path: str, expected_version: int) -> bool:
    """Validate migration was applied correctly."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    validation_passed = True
    
    print(f"🔍 Running Post-Migration Validation (Expected: v{expected_version})...")
    
    # Check 1: Version matches
    print("\n1. Checking schema version...")
    cursor.execute("SELECT MAX(version) FROM schema_version")
    actual_version = cursor.fetchone()[0]
    
    if actual_version == expected_version:
        print(f"   ✅ Schema version correct: v{actual_version}")
    else:
        print(f"   ❌ Version mismatch! Expected: v{expected_version}, Got: v{actual_version}")
        validation_passed = False
    
    # Check 2: No NULL patient_mrn (example for specific migration)
    print("\n2. Checking data integrity...")
    cursor.execute("SELECT COUNT(*) FROM vitals WHERE patient_mrn IS NULL")
    null_count = cursor.fetchone()[0]
    
    if null_count == 0:
        print(f"   ✅ No NULL patient_mrn values")
    else:
        print(f"   ⚠️  Found {null_count} NULL patient_mrn values")
        # This might be OK depending on the migration
    
    # Check 3: Row counts match expectations
    print("\n3. Checking row counts...")
    cursor.execute("SELECT COUNT(*) FROM patients")
    patient_count = cursor.fetchone()[0]
    print(f"   ℹ️  Patients: {patient_count}")
    
    cursor.execute("SELECT COUNT(*) FROM vitals")
    vitals_count = cursor.fetchone()[0]
    print(f"   ℹ️  Vitals: {vitals_count}")
    
    conn.close()
    
    if validation_passed:
        print("\n✅ Post-migration validation passed!")
    else:
        print("\n❌ Validation failed! Review database.")
    
    return validation_passed

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 post_migration_check.py <database_path> <expected_version>")
        sys.exit(1)
    
    passed = validate_migration(sys.argv[1], int(sys.argv[2]))
    sys.exit(0 if passed else 1)
```

---

## 8. Migration Workflow (Step-by-Step)

### **Development Workflow**

```bash
# 1. Edit schema definition
vim schema/database.yaml

# 2. Generate updated code
python3 scripts/generate_schema.py

# 3. Create migration file
cat > schema/migrations/schema/0010_add_new_field.sql << 'EOF'
BEGIN TRANSACTION;
ALTER TABLE patients ADD COLUMN notes TEXT NULL;
INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (10, datetime('now'), 'Add notes field to patients', 'schema');
COMMIT;
EOF

# 4. Create rollback file
cat > schema/migrations/rollback/0010_rollback.sql << 'EOF'
-- Rollback script for version 10
-- (recreate table without notes column)
EOF

# 5. Test migration on copy of database
cp data/zmonitor.db data/zmonitor_test.db
python3 scripts/migrate.py --db data/zmonitor_test.db --dry-run
python3 scripts/migrate.py --db data/zmonitor_test.db

# 6. Validate
python3 scripts/post_migration_check.py data/zmonitor_test.db 10

# 7. Test rollback
python3 scripts/migrate.py --db data/zmonitor_test.db --rollback 10

# 8. If all tests pass, commit
git add schema/database.yaml
git add schema/migrations/schema/0010_*.sql
git add schema/migrations/rollback/0010_*.sql
git add src/infrastructure/persistence/generated/SchemaInfo.h
git commit -m "feat: Add notes field to patients table (migration 0010)"
```

### **Production Deployment Workflow**

```bash
# 1. Backup production database
cp /data/prod/zmonitor.db /data/backups/zmonitor_$(date +%Y%m%d_%H%M%S).db

# 2. Run pre-migration checks
python3 scripts/pre_migration_check.py /data/prod/zmonitor.db
# Exit if checks fail

# 3. Show migration status
python3 scripts/migrate.py --db /data/prod/zmonitor.db --status

# 4. Dry run (preview changes)
python3 scripts/migrate.py --db /data/prod/zmonitor.db --dry-run

# 5. Apply migrations
python3 scripts/migrate.py --db /data/prod/zmonitor.db

# 6. Post-migration validation
python3 scripts/post_migration_check.py /data/prod/zmonitor.db 10

# 7. If validation fails, rollback
python3 scripts/migrate.py --db /data/prod/zmonitor.db --rollback 10

# 8. Monitor application
# Check logs, monitor metrics, verify functionality
```

---

## 9. Best Practices

### **DO:**

✅ **Always backup before migrations** (especially in production)  
✅ **Test migrations on copy of production data**  
✅ **Write rollback scripts for every migration**  
✅ **Use transactions** (BEGIN/COMMIT)  
✅ **Add comments** explaining why the migration is needed  
✅ **Version migrations sequentially** (never skip numbers)  
✅ **Validate data integrity** before and after  
✅ **Time large data migrations** (estimate production time)  
✅ **Use dry-run mode** before applying to production  
✅ **Document migration risks** in migration files

### **DON'T:**

❌ **Don't modify applied migrations** (create new one instead)  
❌ **Don't skip pre-migration checks**  
❌ **Don't run migrations without backups**  
❌ **Don't forget rollback scripts**  
❌ **Don't ignore migration failures** (investigate before retrying)  
❌ **Don't mix schema and data changes** in same migration  
❌ **Don't deploy without testing rollback**  
❌ **Don't assume migrations are instant** (large tables take time)

---

## 10. Rollback Strategy

### **When to Rollback**

- Migration fails mid-execution
- Post-migration validation fails
- Application fails after deployment
- Data corruption detected
- Performance degradation

### **Rollback Types**

| Type | Description | Risk |
|------|-------------|------|
| **Automatic** | Transaction rollback (migration failed) | 🟢 Safe |
| **Manual** | Execute rollback script | ⚠️ Medium |
| **Database Restore** | Restore from backup | 🔴 Data loss risk |

### **Rollback Procedure**

```bash
# 1. Stop application (prevent new data)
systemctl stop z-monitor

# 2. Backup current state (before rollback)
cp /data/prod/zmonitor.db /data/backups/zmonitor_before_rollback_$(date +%Y%m%d_%H%M%S).db

# 3. Run rollback script
python3 scripts/migrate.py --db /data/prod/zmonitor.db --rollback 10

# 4. Validate rollback
python3 scripts/post_migration_check.py /data/prod/zmonitor.db 9

# 5. Restart application
systemctl start z-monitor

# 6. Monitor for issues
tail -f /var/log/z-monitor/app.log
```

---

## 11. Large Data Migration Strategy

### **11.1 Batch Processing**

For migrations affecting millions of rows:

```sql
-- Data Migration: Process in batches to avoid long locks
-- Version: 0011
-- Estimated time: 30-60 minutes for 10M records

BEGIN TRANSACTION;

-- Create progress tracking table
CREATE TEMP TABLE migration_progress (
    batch_num INTEGER,
    processed_count INTEGER,
    timestamp TEXT
);

-- Process in batches of 10000 records
-- Batch 1
UPDATE vitals
SET patient_mrn = (
    SELECT mrn FROM patients WHERE patients.old_id = vitals.patient_id
)
WHERE id IN (
    SELECT id FROM vitals 
    WHERE patient_mrn IS NULL 
    LIMIT 10000
);

INSERT INTO migration_progress VALUES (1, changes(), datetime('now'));

-- Commit after each batch
COMMIT;

-- Repeat for next batch...
-- In production, use a script with a loop
```

### **11.2 Python Batch Migration Script**

```python
#!/usr/bin/env python3
"""
Large Data Migration with Progress Tracking
"""

import sqlite3
import time

def migrate_in_batches(db_path: str, batch_size: int = 10000):
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Count total records to migrate
    cursor.execute("SELECT COUNT(*) FROM vitals WHERE patient_mrn IS NULL")
    total = cursor.fetchone()[0]
    
    print(f"📊 Total records to migrate: {total:,}")
    
    batches = (total + batch_size - 1) // batch_size
    print(f"📦 Processing in {batches} batches of {batch_size:,}")
    
    for batch_num in range(batches):
        start_time = time.time()
        
        # Process batch
        cursor.execute("""
            UPDATE vitals
            SET patient_mrn = (
                SELECT mrn FROM patients WHERE patients.old_id = vitals.patient_id
            )
            WHERE id IN (
                SELECT id FROM vitals 
                WHERE patient_mrn IS NULL 
                LIMIT ?
            )
        """, (batch_size,))
        
        conn.commit()
        
        elapsed = time.time() - start_time
        processed = (batch_num + 1) * batch_size
        progress = min(100, (processed / total) * 100)
        
        print(f"   Batch {batch_num + 1}/{batches}: {processed:,}/{total:,} ({progress:.1f}%) - {elapsed:.2f}s")
    
    print("✅ Migration complete!")
    conn.close()

if __name__ == "__main__":
    migrate_in_batches("data/zmonitor.db", batch_size=10000)
```

---

## 12. CI/CD Integration

### **GitHub Actions Workflow**

**File:** `.github/workflows/migration-check.yml`

```yaml
name: Migration Check

on:
  pull_request:
    paths:
      - 'schema/**'
      - 'scripts/migrate.py'

jobs:
  check-migrations:
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.10'
      
      - name: Install dependencies
        run: |
          pip install pyyaml
      
      - name: Check migration numbering
        run: |
          python3 scripts/check_migration_sequence.py
      
      - name: Test migrations on empty database
        run: |
          # Create test database
          touch test.db
          
          # Run migrations
          python3 scripts/migrate.py --db test.db
          
          # Validate
          python3 scripts/post_migration_check.py test.db $(ls schema/migrations/schema/*.sql | wc -l)
      
      - name: Test migrations on sample data
        run: |
          # Load sample data
          sqlite3 test.db < tests/fixtures/sample_data.sql
          
          # Run migrations
          python3 scripts/migrate.py --db test.db
      
      - name: Test rollback
        run: |
          # Get latest version
          VERSION=$(python3 -c "import sqlite3; print(sqlite3.connect('test.db').execute('SELECT MAX(version) FROM schema_version').fetchone()[0])")
          
          # Rollback
          python3 scripts/migrate.py --db test.db --rollback $VERSION
```

---

## 13. Monitoring and Alerts

### **Migration Metrics to Track**

- **Migration execution time** (store in `schema_version.execution_time_ms`)
- **Row counts before/after** (verify data not lost)
- **Migration failures** (log and alert)
- **Rollback frequency** (indicates migration issues)
- **Database size growth** (monitor storage)

### **Alerts**

- Migration takes >5 minutes (may block application)
- Migration fails
- Post-migration validation fails
- Rollback executed in production

---

## 14. References

- `doc/33_SCHEMA_MANAGEMENT.md` – Schema definition and code generation
- `doc/32_QUERY_REGISTRY.md` – Query string management
- `doc/30_DATABASE_ACCESS_STRATEGY.md` – Repository pattern
- `doc/10_DATABASE_DESIGN.md` – Database schema
- SQLite ALTER TABLE: https://www.sqlite.org/lang_altertable.html
- SQLite PRAGMA: https://www.sqlite.org/pragma.html

---

*This document defines the complete migration workflow for Z Monitor. All schema and data changes must follow this workflow to ensure safety and reliability.*

