# Database Schema Management and Code Generation

**Document ID:** DESIGN-033  
**Version:** 1.2  
**Status:** Approved  
**Last Updated:** 2025-11-27

> **📋 Related Documents:**
> - [34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md) – Complete migration workflow (schema + data migrations, rollback procedures) ⭐
> - [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md) – Query string management (works with SchemaInfo.h constants)
> - [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) – Error handling guidelines (when to return vs. log vs. emit errors)

---

This document defines the schema management strategy for Z Monitor, including schema definition files, code generation, migration workflow, and tooling.

**Key Workflows Covered:**
- ✅ **Adding new tables/columns** (Section 12.1, 12.2)
- ✅ **Deprecating columns/tables** (Section 12.3, 12.4) - 3-phase process with 6-month deprecation period
- ✅ **Removing deprecated items** (Section 12.3, 12.4) - After deprecation period
- ✅ **Best practices and checklists** (Section 12.5, 12.6, 12.7)

> **📊 Schema Management Workflow Diagram:**  
> [View Schema Management Workflow (Mermaid)](./33_SCHEMA_MANAGEMENT.mmd)  
> [View Schema Management Workflow (SVG)](./33_SCHEMA_MANAGEMENT.svg)

---

## 1. The Problem: Hardcoded Column Names

**Current Anti-Pattern:**

```cpp
// ❌ BAD: Hardcoded column names everywhere
PatientIdentity identity(
    query.value("mrn").toString(),        // Typo-prone
    query.value("name").toString(),       // No autocomplete
    query.value("dob").toDate(),          // Hard to refactor
    query.value("sex").toString()         // No compile-time checking
);
```

**Problems:**
- ❌ No autocomplete (easy to make typos)
- ❌ No compile-time checking (typos discovered at runtime)
- ❌ Hard to find all usages (grep for string, not symbol)
- ❌ Hard to refactor (renaming column = find/replace strings)
- ❌ Schema defined in multiple places (SQL + code)
- ❌ Manual mapping code (error-prone, tedious)

---

## 2. Recommended Solution: Schema-First with Code Generation

### **Strategy Overview:**

```
┌─────────────────────────────────────────┐
│  1. YAML Schema Definition              │
│     (Single Source of Truth)            │
│     - Tables, columns, types            │
│     - Indices, constraints              │
│     - Migrations                        │
└─────────────┬───────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│  2. Code Generator (Python/C++)         │
│     Generates:                          │
│     - DDL SQL files                     │
│     - Column name constants             │
│     - Repository boilerplate            │
│     - DTO mappings                      │
└─────────────┬───────────────────────────┘
              │
              ├──────────────┬──────────────┬──────────────┐
              ▼              ▼              ▼              ▼
    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
    │ SchemaInfo.h│  │ DDL.sql     │  │ Repository  │  │ Migrations  │
    │ (constants) │  │ (CREATE)    │  │ Helpers     │  │ (versioned) │
    └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘
```

---

## 3. Schema Definition Format: YAML

### **Why YAML?**

✅ **Human-readable** (easy to edit and review)  
✅ **Machine-parseable** (can be processed by scripts)  
✅ **Version control friendly** (clear diffs)  
✅ **Industry standard** (many tools support it)  
✅ **Supports comments** (document design decisions)

### **Schema File Structure:**

**File:** `z-monitor/schema/database.yaml`

```yaml
# Z Monitor Database Schema Definition
# This is the single source of truth for all database tables and columns.
# DO NOT edit generated files directly - edit this file and regenerate.

version: "1.0.0"
database: zmonitor

tables:
  # ═══════════════════════════════════════════════════════════
  # PATIENTS TABLE
  # ═══════════════════════════════════════════════════════════
  patients:
    description: "Patient demographic and admission information"
    columns:
      mrn:
        type: TEXT
        primary_key: true
        not_null: true
        description: "Medical Record Number (unique patient identifier)"
        
      name:
        type: TEXT
        not_null: true
        description: "Patient full name"
        
      dob:
        type: TEXT
        nullable: true
        description: "Date of birth (ISO 8601 format: YYYY-MM-DD)"
        
      sex:
        type: TEXT
        nullable: true
        description: "Biological sex (M/F/O/U)"
        check: "sex IN ('M', 'F', 'O', 'U')"
        
      bed_location:
        type: TEXT
        nullable: true
        description: "Current bed location (e.g., ICU-04)"
        
      admission_status:
        type: TEXT
        default: "'DISCHARGED'"
        description: "Admission status (ADMITTED/DISCHARGED/TRANSFERRED)"
        check: "admission_status IN ('ADMITTED', 'DISCHARGED', 'TRANSFERRED')"
        
      admitted_at:
        type: INTEGER
        nullable: true
        description: "Unix milliseconds timestamp of admission"
        
      discharged_at:
        type: INTEGER
        nullable: true
        description: "Unix milliseconds timestamp of discharge"
        
      admission_source:
        type: TEXT
        nullable: true
        description: "Source of admission (Manual/Barcode/Central/Emergency)"
        
      last_lookup_at:
        type: INTEGER
        nullable: true
        description: "Unix milliseconds of last HIS lookup (for caching)"
        
      lookup_source:
        type: TEXT
        nullable: true
        description: "Source of last lookup (HIS/Mock/Manual)"
    
    indices:
      - name: idx_patients_mrn
        columns: [mrn]
        unique: true
        
      - name: idx_patients_admission_status
        columns: [admission_status]
        
      - name: idx_patients_bed_location
        columns: [bed_location]
        where: "bed_location IS NOT NULL"
    
    constraints:
      - name: chk_admission_dates
        check: "admitted_at IS NULL OR discharged_at IS NULL OR discharged_at >= admitted_at"
  
  # ═══════════════════════════════════════════════════════════
  # VITALS TABLE (Time-Series)
  # ═══════════════════════════════════════════════════════════
  vitals:
    description: "Vital signs time-series data"
    columns:
      id:
        type: INTEGER
        primary_key: true
        autoincrement: true
        
      uuid:
        type: TEXT
        nullable: true
        description: "UUID for distributed systems"
        
      timestamp:
        type: INTEGER
        not_null: true
        description: "Unix milliseconds when data was measured"
        
      timestamp_iso:
        type: TEXT
        nullable: true
        description: "ISO 8601 timestamp (for readability)"
        
      patient_id:
        type: TEXT
        not_null: true
        description: "DEPRECATED: Use patient_mrn instead"
        
      patient_mrn:
        type: TEXT
        not_null: true
        description: "Patient MRN (FK to patients.mrn)"
        foreign_key:
          table: patients
          column: mrn
          on_delete: CASCADE
          
      device_id:
        type: TEXT
        nullable: true
        description: "Device serial number"
        
      device_label:
        type: TEXT
        nullable: true
        description: "Device asset tag (e.g., ICU-MON-04)"
        
      heart_rate:
        type: REAL
        nullable: true
        description: "Heart rate in BPM"
        check: "heart_rate IS NULL OR (heart_rate >= 0 AND heart_rate <= 300)"
        
      spo2:
        type: REAL
        nullable: true
        description: "SpO2 oxygen saturation (0-100%)"
        check: "spo2 IS NULL OR (spo2 >= 0 AND spo2 <= 100)"
        
      respiration_rate:
        type: REAL
        nullable: true
        description: "Respiration rate in breaths per minute"
        check: "respiration_rate IS NULL OR (respiration_rate >= 0 AND respiration_rate <= 100)"
        
      signal_quality:
        type: INTEGER
        nullable: true
        description: "Signal quality (0-100)"
        check: "signal_quality IS NULL OR (signal_quality >= 0 AND signal_quality <= 100)"
        
      sample_rate_hz:
        type: REAL
        nullable: true
        description: "Sample rate in Hz"
        
      source:
        type: TEXT
        nullable: true
        description: "Data source (Simulator/Device/Test)"
        
      is_synced:
        type: BOOLEAN
        default: 0
        description: "Whether data has been synced to server"
        
      batch_id:
        type: TEXT
        nullable: true
        description: "Telemetry batch ID (FK to telemetry_metrics.batch_id)"
        foreign_key:
          table: telemetry_metrics
          column: batch_id
          on_delete: SET NULL
          on_update: CASCADE
    
    indices:
      - name: idx_vitals_timestamp
        columns: [timestamp]
        
      - name: idx_vitals_patient_mrn
        columns: [patient_mrn]
        
      - name: idx_vitals_patient_time
        columns: [patient_mrn, timestamp]
        
      - name: idx_vitals_unsynced
        columns: [is_synced]
        where: "is_synced = 0"
        
      - name: idx_vitals_batch_id
        columns: [batch_id]
        where: "batch_id IS NOT NULL"
    
    retention:
      policy: "DELETE data older than 7 days"
      column: timestamp
      days: 7

  # ═══════════════════════════════════════════════════════════
  # TELEMETRY METRICS TABLE
  # ═══════════════════════════════════════════════════════════
  telemetry_metrics:
    description: "Telemetry transmission timing and performance metrics"
    columns:
      id:
        type: INTEGER
        primary_key: true
        autoincrement: true
        
      batch_id:
        type: TEXT
        not_null: true
        unique: true
        description: "Unique batch identifier (UUID)"
        
      device_id:
        type: TEXT
        not_null: true
        description: "Device serial number"
        
      device_label:
        type: TEXT
        nullable: true
        description: "Device asset tag"
        
      patient_mrn:
        type: TEXT
        nullable: true
        description: "Patient MRN (NULL if no patient admitted)"
        
      data_created_at:
        type: INTEGER
        not_null: true
        description: "Timestamp of oldest data point in batch"
        
      batch_created_at:
        type: INTEGER
        not_null: true
        description: "Timestamp when batch object was created"
        
      signed_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when batch was digitally signed"
        
      queued_for_tx_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when batch was queued for transmission"
        
      transmitted_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when transmission started"
        
      server_received_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when server received batch"
        
      server_processed_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when server processed batch"
        
      server_ack_at:
        type: INTEGER
        nullable: true
        description: "Timestamp when server sent acknowledgment"
        
      batch_creation_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: batch_created_at - data_created_at"
        
      signing_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: signed_at - batch_created_at"
        
      queue_wait_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: transmitted_at - queued_for_tx_at"
        
      network_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: server_received_at - transmitted_at"
        
      server_processing_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: server_processed_at - server_received_at"
        
      end_to_end_latency_ms:
        type: INTEGER
        nullable: true
        description: "Computed: server_ack_at - data_created_at"
        
      record_count:
        type: INTEGER
        nullable: true
        description: "Number of records in batch"
        
      batch_size_bytes:
        type: INTEGER
        nullable: true
        description: "Size of raw payload in bytes"
        
      compressed_size_bytes:
        type: INTEGER
        nullable: true
        description: "Size after compression (if applicable)"
        
      status:
        type: TEXT
        not_null: true
        description: "Status (success/failed/timeout/retrying)"
        check: "status IN ('success', 'failed', 'timeout', 'retrying')"
        
      error_message:
        type: TEXT
        nullable: true
        description: "Error message if status is failed"
        
      retry_count:
        type: INTEGER
        default: 0
        description: "Number of retry attempts"
        
      latency_class:
        type: TEXT
        nullable: true
        description: "Latency classification (excellent/good/acceptable/slow)"
        check: "latency_class IS NULL OR latency_class IN ('excellent', 'good', 'acceptable', 'slow')"
        
      created_at:
        type: INTEGER
        not_null: true
        description: "Timestamp when metrics record was created"
    
    indices:
      - name: idx_telemetry_metrics_batch_id
        columns: [batch_id]
        unique: true
        
      - name: idx_telemetry_metrics_device_id
        columns: [device_id]
        
      - name: idx_telemetry_metrics_patient_mrn
        columns: [patient_mrn]
        where: "patient_mrn IS NOT NULL"
        
      - name: idx_telemetry_metrics_status
        columns: [status]
        
      - name: idx_telemetry_metrics_created_at
        columns: [created_at]
        
      - name: idx_telemetry_metrics_latency_class
        columns: [latency_class]
        where: "latency_class IS NOT NULL"
    
    retention:
      policy: "DELETE metrics older than 30 days"
      column: created_at
      days: 30

# Add more tables: alarms, admission_events, settings, certificates, etc.
```

---

## 4. Code Generation: SchemaInfo.h

### **Generated File:** `z-monitor/src/infrastructure/persistence/generated/SchemaInfo.h`

```cpp
/**
 * @file SchemaInfo.h
 * @brief Auto-generated database schema constants.
 * 
 * ⚠️ DO NOT EDIT THIS FILE MANUALLY ⚠️
 * 
 * This file is auto-generated from schema/database.yaml
 * To make changes:
 *   1. Edit schema/database.yaml
 *   2. Run: ./scripts/generate_schema.py
 *   3. Commit both files together
 * 
 * Generated: 2025-11-27 04:45:00 UTC
 * Schema Version: 1.0.0
 */

#ifndef SCHEMAINFO_H
#define SCHEMAINFO_H

#include <QString>

/**
 * @namespace Schema
 * @brief Database schema constants (table names, column names, constraints).
 * 
 * Use these constants instead of hardcoded strings to ensure type safety
 * and enable refactoring.
 */
namespace Schema {

/**
 * @namespace Tables
 * @brief Table name constants.
 */
namespace Tables {
    constexpr const char* PATIENTS = "patients";
    constexpr const char* VITALS = "vitals";
    constexpr const char* TELEMETRY_METRICS = "telemetry_metrics";
    constexpr const char* ALARMS = "alarms";
    constexpr const char* ADMISSION_EVENTS = "admission_events";
    constexpr const char* SETTINGS = "settings";
    constexpr const char* CERTIFICATES = "certificates";
    constexpr const char* SECURITY_AUDIT_LOG = "security_audit_log";
}

/**
 * @namespace Columns
 * @brief Column name constants organized by table.
 */
namespace Columns {
    
    /**
     * @namespace Patients
     * @brief Column names for patients table.
     */
    namespace Patients {
        constexpr const char* MRN = "mrn";
        constexpr const char* NAME = "name";
        constexpr const char* DOB = "dob";
        constexpr const char* SEX = "sex";
        constexpr const char* BED_LOCATION = "bed_location";
        constexpr const char* ADMISSION_STATUS = "admission_status";
        constexpr const char* ADMITTED_AT = "admitted_at";
        constexpr const char* DISCHARGED_AT = "discharged_at";
        constexpr const char* ADMISSION_SOURCE = "admission_source";
        constexpr const char* LAST_LOOKUP_AT = "last_lookup_at";
        constexpr const char* LOOKUP_SOURCE = "lookup_source";
    }
    
    /**
     * @namespace Vitals
     * @brief Column names for vitals table.
     */
    namespace Vitals {
        constexpr const char* ID = "id";
        constexpr const char* UUID = "uuid";
        constexpr const char* TIMESTAMP = "timestamp";
        constexpr const char* TIMESTAMP_ISO = "timestamp_iso";
        constexpr const char* PATIENT_ID = "patient_id";  // DEPRECATED
        constexpr const char* PATIENT_MRN = "patient_mrn";
        constexpr const char* DEVICE_ID = "device_id";
        constexpr const char* DEVICE_LABEL = "device_label";
        constexpr const char* HEART_RATE = "heart_rate";
        constexpr const char* SPO2 = "spo2";
        constexpr const char* RESPIRATION_RATE = "respiration_rate";
        constexpr const char* SIGNAL_QUALITY = "signal_quality";
        constexpr const char* SAMPLE_RATE_HZ = "sample_rate_hz";
        constexpr const char* SOURCE = "source";
        constexpr const char* IS_SYNCED = "is_synced";
        constexpr const char* BATCH_ID = "batch_id";
    }
    
    /**
     * @namespace TelemetryMetrics
     * @brief Column names for telemetry_metrics table.
     */
    namespace TelemetryMetrics {
        constexpr const char* ID = "id";
        constexpr const char* BATCH_ID = "batch_id";
        constexpr const char* DEVICE_ID = "device_id";
        constexpr const char* DEVICE_LABEL = "device_label";
        constexpr const char* PATIENT_MRN = "patient_mrn";
        constexpr const char* DATA_CREATED_AT = "data_created_at";
        constexpr const char* BATCH_CREATED_AT = "batch_created_at";
        constexpr const char* SIGNED_AT = "signed_at";
        constexpr const char* QUEUED_FOR_TX_AT = "queued_for_tx_at";
        constexpr const char* TRANSMITTED_AT = "transmitted_at";
        constexpr const char* SERVER_RECEIVED_AT = "server_received_at";
        constexpr const char* SERVER_PROCESSED_AT = "server_processed_at";
        constexpr const char* SERVER_ACK_AT = "server_ack_at";
        constexpr const char* BATCH_CREATION_LATENCY_MS = "batch_creation_latency_ms";
        constexpr const char* SIGNING_LATENCY_MS = "signing_latency_ms";
        constexpr const char* QUEUE_WAIT_LATENCY_MS = "queue_wait_latency_ms";
        constexpr const char* NETWORK_LATENCY_MS = "network_latency_ms";
        constexpr const char* SERVER_PROCESSING_LATENCY_MS = "server_processing_latency_ms";
        constexpr const char* END_TO_END_LATENCY_MS = "end_to_end_latency_ms";
        constexpr const char* RECORD_COUNT = "record_count";
        constexpr const char* BATCH_SIZE_BYTES = "batch_size_bytes";
        constexpr const char* COMPRESSED_SIZE_BYTES = "compressed_size_bytes";
        constexpr const char* STATUS = "status";
        constexpr const char* ERROR_MESSAGE = "error_message";
        constexpr const char* RETRY_COUNT = "retry_count";
        constexpr const char* LATENCY_CLASS = "latency_class";
        constexpr const char* CREATED_AT = "created_at";
    }
    
    // Add more table namespaces...
}

/**
 * @namespace Constraints
 * @brief Constraint definitions for validation.
 */
namespace Constraints {
    namespace Patients {
        constexpr const char* VALID_SEX_VALUES[] = {"M", "F", "O", "U"};
        constexpr const char* VALID_ADMISSION_STATUS[] = {"ADMITTED", "DISCHARGED", "TRANSFERRED"};
    }
    
    namespace Vitals {
        constexpr double MIN_HEART_RATE = 0.0;
        constexpr double MAX_HEART_RATE = 300.0;
        constexpr double MIN_SPO2 = 0.0;
        constexpr double MAX_SPO2 = 100.0;
        constexpr double MIN_RESPIRATION_RATE = 0.0;
        constexpr double MAX_RESPIRATION_RATE = 100.0;
        constexpr int MIN_SIGNAL_QUALITY = 0;
        constexpr int MAX_SIGNAL_QUALITY = 100;
    }
    
    namespace TelemetryMetrics {
        constexpr const char* VALID_STATUS_VALUES[] = {"success", "failed", "timeout", "retrying"};
        constexpr const char* VALID_LATENCY_CLASS[] = {"excellent", "good", "acceptable", "slow"};
    }
}

} // namespace Schema

#endif // SCHEMAINFO_H
```

---

## 5. Usage: Type-Safe Column Names

### **Before (Hardcoded Strings):**

```cpp
// ❌ BAD: Hardcoded column names
PatientIdentity identity(
    query.value("mrn").toString(),
    query.value("name").toString(),
    query.value("dob").toDate(),
    query.value("sex").toString()
);
```

### **After (Schema Constants):**

```cpp
// ✅ GOOD: Type-safe constants with autocomplete
#include "generated/SchemaInfo.h"

PatientIdentity identity(
    query.value(Schema::Columns::Patients::MRN).toString(),
    query.value(Schema::Columns::Patients::NAME).toString(),
    query.value(Schema::Columns::Patients::DOB).toDate(),
    query.value(Schema::Columns::Patients::SEX).toString()
);
```

**Benefits:**
- ✅ **Autocomplete:** `Schema::Columns::Patients::` shows all columns
- ✅ **Compile-time checking:** Typos caught at compile time
- ✅ **Refactoring:** Rename constant updates everywhere
- ✅ **Findability:** Ctrl+Click to find all usages

---

## 6. Code Generator: Python Script

### **File:** `z-monitor/scripts/generate_schema.py`

```python
#!/usr/bin/env python3
"""
Database Schema Code Generator

Generates C++ header files and SQL DDL from YAML schema definition.

Usage:
    python3 scripts/generate_schema.py
    
Input:
    schema/database.yaml
    
Output:
    src/infrastructure/persistence/generated/SchemaInfo.h
    schema/generated/ddl/create_tables.sql
    schema/generated/ddl/create_indices.sql
"""

import yaml
import os
from datetime import datetime
from typing import Dict, List, Any

def load_schema(schema_file: str) -> Dict[str, Any]:
    """Load YAML schema file."""
    with open(schema_file, 'r') as f:
        return yaml.safe_load(f)

def to_cpp_constant(name: str) -> str:
    """Convert database column name to C++ constant name."""
    return name.upper().replace('.', '_')

def generate_schema_header(schema: Dict[str, Any], output_file: str):
    """Generate SchemaInfo.h C++ header file."""
    
    header = f"""/**
 * @file SchemaInfo.h
 * @brief Auto-generated database schema constants.
 * 
 * ⚠️ DO NOT EDIT THIS FILE MANUALLY ⚠️
 * 
 * This file is auto-generated from schema/database.yaml
 * To make changes:
 *   1. Edit schema/database.yaml
 *   2. Run: ./scripts/generate_schema.py
 *   3. Commit both files together
 * 
 * Generated: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')}
 * Schema Version: {schema['version']}
 */

#ifndef SCHEMAINFO_H
#define SCHEMAINFO_H

#include <QString>

namespace Schema {{

// Table names
namespace Tables {{
"""
    
    # Generate table name constants
    for table_name in schema['tables'].keys():
        const_name = to_cpp_constant(table_name)
        header += f'    constexpr const char* {const_name} = "{table_name}";\n'
    
    header += "}\n\n// Column names\nnamespace Columns {\n"
    
    # Generate column name constants for each table
    for table_name, table_def in schema['tables'].items():
        namespace_name = table_name.replace('_', ' ').title().replace(' ', '')
        header += f"\n    namespace {namespace_name} {{\n"
        
        for col_name in table_def['columns'].keys():
            const_name = to_cpp_constant(col_name)
            header += f'        constexpr const char* {const_name} = "{col_name}";\n'
        
        header += "    }\n"
    
    header += "}\n\n} // namespace Schema\n\n#endif // SCHEMAINFO_H\n"
    
    # Write to file
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    with open(output_file, 'w') as f:
        f.write(header)
    
    print(f"✅ Generated: {output_file}")

def generate_ddl(schema: Dict[str, Any], output_dir: str):
    """Generate SQL DDL files."""
    
    create_tables_sql = f"""-- Auto-generated DDL for table creation
-- Generated: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')}
-- Schema Version: {schema['version']}
-- ⚠️ DO NOT EDIT MANUALLY - Edit schema/database.yaml and regenerate

"""
    
    for table_name, table_def in schema['tables'].items():
        create_tables_sql += f"\n-- {table_def.get('description', table_name)}\n"
        create_tables_sql += f"CREATE TABLE IF NOT EXISTS {table_name} (\n"
        
        columns = []
        for col_name, col_def in table_def['columns'].items():
            col_sql = f"    {col_name} {col_def['type']}"
            
            if col_def.get('primary_key'):
                col_sql += " PRIMARY KEY"
            if col_def.get('autoincrement'):
                col_sql += " AUTOINCREMENT"
            if col_def.get('not_null'):
                col_sql += " NOT NULL"
            if col_def.get('unique'):
                col_sql += " UNIQUE"
            if 'default' in col_def:
                col_sql += f" DEFAULT {col_def['default']}"
            if 'check' in col_def:
                col_sql += f" CHECK ({col_def['check']})"
            
            # Add comment
            if 'description' in col_def:
                col_sql += f"  -- {col_def['description']}"
            
            columns.append(col_sql)
        
        # Add foreign keys
        for col_name, col_def in table_def['columns'].items():
            if 'foreign_key' in col_def:
                fk = col_def['foreign_key']
                fk_sql = f"    FOREIGN KEY ({col_name}) REFERENCES {fk['table']}({fk['column']})"
                if 'on_delete' in fk:
                    fk_sql += f" ON DELETE {fk['on_delete']}"
                if 'on_update' in fk:
                    fk_sql += f" ON UPDATE {fk['on_update']}"
                columns.append(fk_sql)
        
        # Add table constraints
        if 'constraints' in table_def:
            for constraint in table_def['constraints']:
                columns.append(f"    CONSTRAINT {constraint['name']} CHECK ({constraint['check']})")
        
        create_tables_sql += ",\n".join(columns)
        create_tables_sql += "\n);\n"
    
    # Write CREATE TABLES
    os.makedirs(output_dir, exist_ok=True)
    create_tables_file = os.path.join(output_dir, "create_tables.sql")
    with open(create_tables_file, 'w') as f:
        f.write(create_tables_sql)
    print(f"✅ Generated: {create_tables_file}")
    
    # Generate CREATE INDICES
    create_indices_sql = f"""-- Auto-generated DDL for index creation
-- Generated: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')}
-- ⚠️ DO NOT EDIT MANUALLY - Edit schema/database.yaml and regenerate

"""
    
    for table_name, table_def in schema['tables'].items():
        if 'indices' in table_def:
            for index in table_def['indices']:
                unique = "UNIQUE " if index.get('unique') else ""
                columns_str = ", ".join(index['columns'])
                create_indices_sql += f"CREATE {unique}INDEX IF NOT EXISTS {index['name']} "
                create_indices_sql += f"ON {table_name} ({columns_str})"
                if 'where' in index:
                    create_indices_sql += f" WHERE {index['where']}"
                create_indices_sql += ";\n"
            create_indices_sql += "\n"
    
    create_indices_file = os.path.join(output_dir, "create_indices.sql")
    with open(create_indices_file, 'w') as f:
        f.write(create_indices_sql)
    print(f"✅ Generated: {create_indices_file}")

def main():
    schema_file = "schema/database.yaml"
    header_output = "src/infrastructure/persistence/generated/SchemaInfo.h"
    ddl_output_dir = "schema/generated/ddl"
    
    print("📊 Z Monitor Schema Code Generator")
    print(f"📂 Loading schema: {schema_file}")
    
    schema = load_schema(schema_file)
    
    print(f"🔨 Generating C++ header...")
    generate_schema_header(schema, header_output)
    
    print(f"🔨 Generating SQL DDL...")
    generate_ddl(schema, ddl_output_dir)
    
    print("✅ Schema generation complete!")
    print("\n📝 Next steps:")
    print("  1. Review generated files")
    print("  2. Commit schema/database.yaml + generated files together")
    print("  3. Run: cmake --build build")

if __name__ == "__main__":
    main()
```

---

## 7. Migration Workflow

> **📋 Note:** This section covers basic schema migrations. For complete migration workflow including data migrations, rollback procedures, backup strategies, and testing, see [34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md).

### **7.1 Migration Files**

**Directory Structure:**

```
z-monitor/schema/
├── database.yaml                    # Source of truth
├── migrations/
│   ├── 0001_initial_schema.sql     # Initial schema
│   ├── 0002_add_adt_workflow.sql   # ADT workflow
│   ├── 0003_add_telemetry_metrics.sql
│   └── README.md                    # Migration guide
└── generated/
    └── ddl/
        ├── create_tables.sql        # Generated DDL
        └── create_indices.sql       # Generated indices
```

### **7.2 Migration Script**

**File:** `z-monitor/scripts/migrate.py`

```python
#!/usr/bin/env python3
"""
Database Migration Runner

Applies numbered migration files in order.

Usage:
    python3 scripts/migrate.py --db /path/to/database.db
"""

import sqlite3
import os
import sys
import argparse
from pathlib import Path

def get_current_version(conn: sqlite3.Connection) -> int:
    """Get current schema version from database."""
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS schema_version (
            version INTEGER PRIMARY KEY,
            applied_at TEXT NOT NULL,
            description TEXT
        )
    """)
    
    cursor.execute("SELECT MAX(version) FROM schema_version")
    result = cursor.fetchone()
    return result[0] if result[0] is not None else 0

def apply_migration(conn: sqlite3.Connection, migration_file: Path, version: int):
    """Apply a single migration file."""
    print(f"📦 Applying migration {version}: {migration_file.name}")
    
    try:
        with open(migration_file, 'r') as f:
            sql = f.read()
        
        cursor = conn.cursor()
        cursor.executescript(sql)
        
        cursor.execute("""
            INSERT INTO schema_version (version, applied_at, description)
            VALUES (?, datetime('now'), ?)
        """, (version, migration_file.stem))
        
        conn.commit()
        print(f"✅ Migration {version} applied successfully")
    except sqlite3.Error as e:
        conn.rollback()
        print(f"❌ Migration {version} FAILED: {e}")
        raise  # Re-raise to stop migration process
    except Exception as e:
        conn.rollback()
        print(f"❌ Unexpected error in migration {version}: {e}")
        raise

def main():
    parser = argparse.ArgumentParser(description='Database Migration Runner')
    parser.add_argument('--db', required=True, help='Path to database file')
    args = parser.parse_args()
    
    migrations_dir = Path("schema/migrations")
    conn = sqlite3.connect(args.db)
    
    current_version = get_current_version(conn)
    print(f"📊 Current schema version: {current_version}")
    
    # Get all migration files
    migration_files = sorted(migrations_dir.glob("*.sql"))
    
    for migration_file in migration_files:
        # Extract version number from filename (e.g., 0001_initial_schema.sql)
        version = int(migration_file.stem.split('_')[0])
        
        if version > current_version:
            apply_migration(conn, migration_file, version)
    
    final_version = get_current_version(conn)
    print(f"✅ Database is up to date (version {final_version})")
    
    conn.close()

if __name__ == "__main__":
    main()
```

### **7.3 Transaction Handling in Migrations**

**⚠️ IMPORTANT: DatabaseManager uses programmatic transactions, not SQL transaction commands.**

**Migration File Rules:**

✅ **DO NOT** include explicit `BEGIN TRANSACTION` or `COMMIT` statements in migration SQL files  
✅ DatabaseManager automatically wraps each migration file in a programmatic transaction  
✅ All statements in a migration file execute atomically (all-or-nothing)  
✅ If any statement fails, the entire migration is rolled back  

**Correct Migration File Format:**

```sql
-- schema/migrations/0002_add_indices.sql
-- Note: Transactions are managed programmatically by DatabaseManager
-- Do not add explicit BEGIN TRANSACTION or COMMIT statements

CREATE INDEX IF NOT EXISTS idx_patients_mrn ON patients (mrn);
CREATE INDEX IF NOT EXISTS idx_vitals_timestamp ON vitals (timestamp);
CREATE INDEX IF NOT EXISTS idx_alarms_patient_priority ON alarms (patient_mrn, priority, start_time);

-- Note: migrate.py script records this migration automatically
```

**Incorrect Migration File Format (DO NOT USE):**

```sql
-- ❌ WRONG: Explicit transaction commands are ignored
BEGIN TRANSACTION;

CREATE INDEX idx_patients_mrn ON patients (mrn);

COMMIT;
```

**How DatabaseManager Handles Transactions:**

```cpp
// From DatabaseManager::executeMigrations()
for (const QString &migrationPath : migrations)
{
    // Load migration SQL from file
    QFile migrationFile(migrationPath);
    QString sql = migrationFile.readAll();
    
    // Split into individual statements
    QStringList statements = sql.split(";", Qt::SkipEmptyParts);
    
    // Wrap ENTIRE migration in programmatic transaction
    if (!m_writeDb.transaction()) {
        // Handle transaction begin failure
    }
    
    // Execute each statement
    for (const QString &stmt : statements) {
        // Skip explicit transaction commands (ignored)
        if (stmt.contains(QRegularExpression("^(BEGIN|COMMIT|ROLLBACK)"))) {
            continue;  // Transaction managed programmatically
        }
        
        if (!query.exec(stmt)) {
            migrationSuccess = false;
            break;
        }
    }
    
    // Commit or rollback based on success
    if (migrationSuccess) {
        m_writeDb.commit();   // All statements succeeded
    } else {
        m_writeDb.rollback(); // Any statement failed → rollback all
    }
}
```

**Benefits of Programmatic Transactions:**

✅ **Atomic migrations** - All statements in a migration file execute or none do  
✅ **Automatic rollback** - If any statement fails, database remains in consistent state  
✅ **Simpler SQL files** - Migration files only contain DDL/DML, no transaction boilerplate  
✅ **Easier testing** - Migration logic in C++ is testable with unit/integration tests  
✅ **Better error handling** - C++ code can provide detailed error messages and logging  

**Migration Best Practices:**

1. **Keep migrations atomic** - Each migration file should represent one logical change
2. **Test migrations** - Run migration tests against copy of production database schema
3. **Idempotent DDL** - Use `CREATE TABLE IF NOT EXISTS` and `CREATE INDEX IF NOT EXISTS`
4. **Document changes** - Add comment header explaining what the migration does
5. **No data in schema migrations** - Use separate data migration files if needed (see 34_DATA_MIGRATION_WORKFLOW.md)

---

## 8. CMake Integration

### **File:** `z-monitor/CMakeLists.txt`

```cmake
# Add custom target to generate schema
add_custom_target(generate_schema
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/generate_schema.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating schema constants from YAML"
    BYPRODUCTS
        ${CMAKE_SOURCE_DIR}/src/infrastructure/persistence/generated/SchemaInfo.h
        ${CMAKE_SOURCE_DIR}/schema/generated/ddl/create_tables.sql
        ${CMAKE_SOURCE_DIR}/schema/generated/ddl/create_indices.sql
)

# Include generated header in infrastructure sources
set(INFRASTRUCTURE_SOURCES
    src/infrastructure/persistence/DatabaseManager.cpp
    src/infrastructure/persistence/SQLitePatientRepository.cpp
    src/infrastructure/persistence/generated/SchemaInfo.h  # Generated
    # ...
)

# Ensure schema is generated before building
add_dependencies(z-monitor generate_schema)
```

---

## 9. Pre-commit Hook Integration

### **File:** `.pre-commit-config.yaml`

```yaml
repos:
  - repo: local
    hooks:
      - id: schema-sync-check
        name: Check schema is synchronized
        entry: scripts/pre-commit-schema-check.sh
        language: script
        files: ^schema/database\.yaml$
```

### **File:** `scripts/pre-commit-schema-check.sh`

```bash
#!/bin/bash
# Pre-commit hook: Verify schema is synchronized

if git diff --cached --name-only | grep -q "schema/database.yaml"; then
    echo "⚠️  schema/database.yaml modified - regenerating..."
    python3 scripts/generate_schema.py
    
    # Add generated files to commit
    git add src/infrastructure/persistence/generated/SchemaInfo.h
    git add schema/generated/ddl/*.sql
    
    echo "✅ Schema regenerated and staged"
fi
```

---

## 10. Complete Workflow Example

### **Scenario: Add new column to patients table**

**Step 1: Edit schema YAML**

```yaml
# schema/database.yaml
patients:
  columns:
    # ... existing columns ...
    emergency_contact:  # NEW COLUMN
      type: TEXT
      nullable: true
      description: "Emergency contact phone number"
```

**Step 2: Regenerate code**

```bash
python3 scripts/generate_schema.py
```

**Output:**
```
📊 Z Monitor Schema Code Generator
📂 Loading schema: schema/database.yaml
🔨 Generating C++ header...
✅ Generated: src/infrastructure/persistence/generated/SchemaInfo.h
🔨 Generating SQL DDL...
✅ Generated: schema/generated/ddl/create_tables.sql
✅ Generated: schema/generated/ddl/create_indices.sql
✅ Schema generation complete!
```

**Step 3: Create migration SQL**

```sql
-- schema/migrations/0004_add_emergency_contact.sql
ALTER TABLE patients ADD COLUMN emergency_contact TEXT NULL;
```

**Step 4: Use new constant in code**

```cpp
// ✅ New constant auto-generated and available
QString emergency = query.value(Schema::Columns::Patients::EMERGENCY_CONTACT).toString();
```

**Step 5: Apply migration**

```bash
python3 scripts/migrate.py --db data/zmonitor.db
```

**Step 6: Commit**

```bash
git add schema/database.yaml
git add schema/migrations/0004_add_emergency_contact.sql
git add src/infrastructure/persistence/generated/SchemaInfo.h
git add schema/generated/ddl/*.sql
git commit -m "Add emergency_contact column to patients table"
```

---

## 11. Benefits Summary

| Problem                    | Solution              | Benefit                               |
| -------------------------- | --------------------- | ------------------------------------- |
| Hardcoded column names     | Schema constants      | ✅ Autocomplete, compile-time checking |
| Schema in multiple places  | YAML single source    | ✅ One place to edit                   |
| Manual DDL writing         | Generated SQL         | ✅ No typos, consistent                |
| Hard to find column usages | Named constants       | ✅ Ctrl+Click finds all                |
| Schema drift               | Pre-commit hook       | ✅ Always synchronized                 |
| Manual migrations          | Migration runner      | ✅ Automated, tracked                  |
| Hard to refactor columns   | Constants + generator | ✅ Rename in YAML → updates everywhere |

---

## 12. Schema Change Workflows

### 12.1. Adding New Tables

**Workflow for adding a new table:**

**Step 1: Update Schema YAML**
```yaml
# schema/database.yaml
tables:
  # ... existing tables ...
  
  # NEW TABLE
  notifications:
    description: "User notifications and alerts"
    columns:
      id:
        type: INTEGER
        primary_key: true
        autoincrement: true
      message:
        type: TEXT
        not_null: true
      severity:
        type: TEXT
        not_null: true
        check: "severity IN ('INFO', 'WARNING', 'ERROR')"
      created_at:
        type: INTEGER
        not_null: true
    indices:
      - name: idx_notifications_created_at
        columns: [created_at]
```

**Step 2: Regenerate Schema**
```bash
python3 scripts/generate_schema.py
```

**Step 3: Create Migration SQL**
```sql
-- schema/migrations/schema/0010_create_notifications_table.sql
BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS notifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    message TEXT NOT NULL,
    severity TEXT NOT NULL CHECK(severity IN ('INFO', 'WARNING', 'ERROR')),
    created_at INTEGER NOT NULL
);

CREATE INDEX idx_notifications_created_at ON notifications(created_at);

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (10, datetime('now'), 'Create notifications table', 'schema');

COMMIT;
```

**Step 4: Update QueryRegistry (if needed)**
```cpp
// QueryRegistry.h - Add query IDs for new table
namespace QueryId {
    namespace Notifications {
        constexpr const char* INSERT = "notifications.insert";
        constexpr const char* FIND_ALL = "notifications.find_all";
        constexpr const char* DELETE = "notifications.delete";
    }
}

// QueryCatalog.cpp - Add SQL queries
queries[QueryId::Notifications::INSERT] = {
    .id = QueryId::Notifications::INSERT,
    .sql = R"(
        INSERT INTO notifications (message, severity, created_at)
        VALUES (:message, :severity, :created_at)
    )",
    .description = "Insert notification",
    .parameters = {":message", ":severity", ":created_at"},
    .isReadOnly = false
};
```

**Step 5: Apply Migration**
```bash
python3 scripts/migrate.py --db data/zmonitor.db
```

**Step 6: Use in Code**
```cpp
// ✅ Use new constants
#include "generated/SchemaInfo.h"
#include "QueryRegistry.h"

QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Notifications::INSERT);
query.bindValue(":message", "Patient admitted");
query.bindValue(":severity", "INFO");
query.bindValue(":created_at", QDateTime::currentMSecsSinceEpoch());
query.exec();
```

**Step 7: Commit**
```bash
git add schema/database.yaml
git add schema/migrations/schema/0010_create_notifications_table.sql
git add src/infrastructure/persistence/generated/SchemaInfo.h
git add src/infrastructure/persistence/QueryRegistry.h
git add src/infrastructure/persistence/QueryCatalog.cpp
git commit -m "Add notifications table"
```

### 12.2. Adding New Columns

**Workflow for adding a new column to existing table:**

**Step 1: Update Schema YAML**
```yaml
# schema/database.yaml
patients:
  columns:
    # ... existing columns ...
    emergency_contact:  # NEW COLUMN
      type: TEXT
      nullable: true
      description: "Emergency contact phone number"
```

**Step 2: Regenerate Schema**
```bash
python3 scripts/generate_schema.py
```

**Step 3: Create Migration SQL**
```sql
-- schema/migrations/schema/0011_add_emergency_contact.sql
BEGIN TRANSACTION;

ALTER TABLE patients ADD COLUMN emergency_contact TEXT NULL;

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (11, datetime('now'), 'Add emergency_contact column to patients', 'schema');

COMMIT;
```

**Step 4: Update Queries (if needed)**
```cpp
// QueryCatalog.cpp - Update SELECT queries to include new column
queries[QueryId::Patient::FIND_BY_MRN] = {
    .sql = R"(
        SELECT mrn, name, dob, sex, bed_location, emergency_contact
        FROM patients
        WHERE mrn = :mrn
    )",
    // ... updated parameters
};
```

**Step 5: Apply Migration**
```bash
python3 scripts/migrate.py --db data/zmonitor.db
```

**Step 6: Use in Code**
```cpp
// ✅ New constant available
QString emergency = query.value(Schema::Columns::Patients::EMERGENCY_CONTACT).toString();
```

**Step 7: Commit**
```bash
git add schema/database.yaml
git add schema/migrations/schema/0011_add_emergency_contact.sql
git add src/infrastructure/persistence/generated/SchemaInfo.h
git add src/infrastructure/persistence/QueryCatalog.cpp
git commit -m "Add emergency_contact column to patients table"
```

### 12.3. Deprecating Columns or Tables

**⚠️ CRITICAL: Deprecation must follow a multi-phase process to avoid breaking existing code.**

**Deprecation Policy:**
1. **Phase 1: Mark as Deprecated** (6 months before removal)
2. **Phase 2: Remove from Active Use** (update all code to stop using)
3. **Phase 3: Final Removal** (after deprecation period)

**Workflow for Deprecating a Column:**

**Phase 1: Mark as Deprecated in Schema YAML**

```yaml
# schema/database.yaml
patients:
  columns:
    patient_id:  # DEPRECATED
      type: TEXT
      nullable: true
      description: "DEPRECATED: Use patient_mrn instead. Will be removed in v2.0.0"
      deprecated: true
      deprecated_since: "2025-11-27"
      removal_version: "2.0.0"
      replacement: "patient_mrn"
```

**Step 2: Regenerate Schema (Constant Still Generated, But Marked)**

```cpp
// SchemaInfo.h - Constant still exists but marked deprecated
namespace Schema::Columns::Patients {
    constexpr const char* PATIENT_ID = "patient_id";  // DEPRECATED: Use PATIENT_MRN instead
    constexpr const char* PATIENT_MRN = "patient_mrn";
}
```

**Step 3: Add Deprecation Warnings in Code**

```cpp
// ✅ GOOD: Log warning when deprecated column is accessed
std::optional<PatientAggregate> SQLitePatientRepository::findByMrn(const QString& mrn) {
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        return std::nullopt;
    }
    
    if (query.next()) {
        // ✅ Check if deprecated column is being used
        if (query.value(Schema::Columns::Patients::PATIENT_ID).isValid()) {
            m_logService->warning("Using deprecated column patient_id", {
                {"mrn", mrn},
                {"deprecated_since", "2025-11-27"},
                {"removal_version", "2.0.0"},
                {"replacement", "patient_mrn"}
            });
        }
        
        // ✅ Use new column instead
        return mapToAggregate(query);  // Uses PATIENT_MRN, not PATIENT_ID
    }
    
    return std::nullopt;
}
```

**Step 4: Update All Code to Stop Using Deprecated Column**

```cpp
// ❌ BAD: Still using deprecated column
QString oldId = query.value(Schema::Columns::Patients::PATIENT_ID).toString();

// ✅ GOOD: Use replacement column
QString mrn = query.value(Schema::Columns::Patients::PATIENT_MRN).toString();
```

**Phase 2: Remove from Active Use (After 6 Months)**

**Step 1: Verify No Usages**
```bash
# Search for all usages of deprecated constant
grep -r "PATIENT_ID" src/
# Should return zero results (or only in deprecation warnings)
```

**Step 2: Update Schema YAML (Mark for Removal)**
```yaml
# schema/database.yaml
patients:
  columns:
    patient_id:  # REMOVED IN v2.0.0
      type: TEXT
      nullable: true
      description: "REMOVED: Use patient_mrn instead"
      removed: true
      removed_in: "2.0.0"
      replacement: "patient_mrn"
```

**Phase 3: Final Removal (After Deprecation Period)**

**Step 1: Create Migration to Remove Column**

```sql
-- schema/migrations/schema/0020_remove_deprecated_patient_id.sql
-- ⚠️ WARNING: This migration removes a deprecated column
-- Only run after all code has been updated and deprecation period has passed

BEGIN TRANSACTION;

-- SQLite doesn't support DROP COLUMN directly, must recreate table
-- 1. Create new table without deprecated column
CREATE TABLE patients_new (
    mrn TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    dob TEXT NULL,
    sex TEXT NULL,
    patient_mrn TEXT NOT NULL,  -- ✅ Using replacement column
    -- patient_id removed  -- ❌ Deprecated column removed
    bed_location TEXT NULL,
    -- ... other columns ...
);

-- 2. Copy data (excluding deprecated column)
INSERT INTO patients_new (mrn, name, dob, sex, patient_mrn, bed_location, ...)
SELECT mrn, name, dob, sex, patient_mrn, bed_location, ...
FROM patients;

-- 3. Drop old table
DROP TABLE patients;

-- 4. Rename new table
ALTER TABLE patients_new RENAME TO patients;

-- 5. Recreate indices
CREATE UNIQUE INDEX idx_patients_mrn ON patients(mrn);
-- ... other indices ...

-- 6. Update schema version
INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (20, datetime('now'), 'Remove deprecated patient_id column', 'schema');

COMMIT;
```

**Step 2: Remove from Schema YAML**
```yaml
# schema/database.yaml
patients:
  columns:
    # patient_id removed - no longer in schema
    patient_mrn:
      type: TEXT
      not_null: true
      description: "Patient MRN (replaced deprecated patient_id)"
```

**Step 3: Regenerate Schema**
```bash
python3 scripts/generate_schema.py
```

**Output:**
```cpp
// SchemaInfo.h - Deprecated constant removed
namespace Schema::Columns::Patients {
    // PATIENT_ID constant removed → compile error if still used!
    constexpr const char* PATIENT_MRN = "patient_mrn";
}
```

**Step 4: Build Will Fail if Deprecated Column Still Used**
```bash
cmake --build build

# ❌ Compile error if deprecated constant still used:
# error: 'PATIENT_ID' is not a member of 'Schema::Columns::Patients'
#     query.value(Schema::Columns::Patients::PATIENT_ID)
#                                            ^~~~~~~~~~~
```

**Step 5: Fix All Compile Errors**
```cpp
// ✅ Update all code to use replacement
QString mrn = query.value(Schema::Columns::Patients::PATIENT_MRN).toString();
```

**Step 6: Apply Migration**
```bash
python3 scripts/migrate.py --db data/zmonitor.db
```

**Step 7: Commit**
```bash
git add schema/database.yaml
git add schema/migrations/schema/0020_remove_deprecated_patient_id.sql
git add src/infrastructure/persistence/generated/SchemaInfo.h
git add src/  # All code updates
git commit -m "Remove deprecated patient_id column (replaced by patient_mrn)"
```

### 12.4. Deprecating Tables

**Workflow for Deprecating an Entire Table:**

**Phase 1: Mark Table as Deprecated**

```yaml
# schema/database.yaml
old_telemetry:  # DEPRECATED TABLE
  description: "DEPRECATED: Use telemetry_metrics instead. Will be removed in v2.0.0"
  deprecated: true
  deprecated_since: "2025-11-27"
  removal_version: "2.0.0"
  replacement: "telemetry_metrics"
  columns:
    # ... existing columns ...
```

**Phase 2: Update All Code to Use Replacement Table**

```cpp
// ❌ BAD: Using deprecated table
QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::OldTelemetry::FIND_ALL);

// ✅ GOOD: Use replacement table
QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::TelemetryMetrics::FIND_ALL);
```

**Phase 3: Create Data Migration (if needed)**

```sql
-- schema/migrations/data/0021_migrate_old_telemetry_to_new.sql
-- Migrate data from deprecated table to replacement table

BEGIN TRANSACTION;

INSERT INTO telemetry_metrics (
    batch_id, device_id, patient_mrn, data_created_at, ...
)
SELECT 
    batch_id, device_id, patient_mrn, data_created_at, ...
FROM old_telemetry;

-- Verify migration
SELECT COUNT(*) FROM old_telemetry;  -- Should match count in telemetry_metrics

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (21, datetime('now'), 'Migrate data from deprecated old_telemetry to telemetry_metrics', 'data');

COMMIT;
```

**Phase 4: Final Removal**

```sql
-- schema/migrations/schema/0022_drop_deprecated_old_telemetry.sql
-- ⚠️ WARNING: This migration drops a deprecated table
-- Only run after data migration and deprecation period has passed

BEGIN TRANSACTION;

-- Verify data has been migrated
SELECT COUNT(*) FROM old_telemetry;  -- Should be 0 or verify manually

-- Drop deprecated table
DROP TABLE IF EXISTS old_telemetry;

-- Drop related indices
DROP INDEX IF EXISTS idx_old_telemetry_batch_id;

INSERT INTO schema_version (version, applied_at, description, migration_type)
VALUES (22, datetime('now'), 'Drop deprecated old_telemetry table', 'schema');

COMMIT;
```

**Step 5: Remove from Schema YAML**
```yaml
# schema/database.yaml
# old_telemetry table removed - no longer in schema
telemetry_metrics:
  description: "Telemetry transmission timing and performance metrics"
  # ... columns ...
```

**Step 6: Remove from QueryRegistry**
```cpp
// QueryRegistry.h - Remove deprecated namespace
namespace QueryId {
    // OldTelemetry namespace removed
    namespace TelemetryMetrics {
        // ... queries ...
    }
}
```

**Step 7: Regenerate and Apply**
```bash
python3 scripts/generate_schema.py
python3 scripts/migrate.py --db data/zmonitor.db
```

### 12.5. Deprecation Checklist

**Before Deprecating:**
- [ ] Identify replacement (column/table)
- [ ] Document deprecation reason
- [ ] Set deprecation date and removal version
- [ ] Update schema YAML with deprecation metadata
- [ ] Regenerate SchemaInfo.h

**During Deprecation Period (6 months):**
- [ ] Add deprecation warnings in code
- [ ] Update all code to use replacement
- [ ] Remove deprecated constant/table from active use
- [ ] Verify no new code uses deprecated item
- [ ] Document migration path for existing data (if needed)

**Before Final Removal:**
- [ ] Verify deprecation period has passed (6 months minimum)
- [ ] Search codebase for all usages (should be zero)
- [ ] Create data migration (if data needs to be preserved)
- [ ] Test migration on staging database
- [ ] Create removal migration SQL
- [ ] Update schema YAML (remove deprecated item)
- [ ] Regenerate SchemaInfo.h
- [ ] Verify build succeeds (no compile errors)
- [ ] Apply migration to production

**After Removal:**
- [ ] Verify migration applied successfully
- [ ] Monitor for errors (should be none if done correctly)
- [ ] Update documentation
- [ ] Remove deprecation warnings from code

### 12.6. Best Practices for Schema Changes

#### **Adding New Items:**
- ✅ **Always add, never remove** (during active development)
- ✅ **Use nullable columns** for new fields (allows gradual rollout)
- ✅ **Provide default values** when possible
- ✅ **Update QueryRegistry** if queries affected
- ✅ **Test migration** on copy of production data
- ✅ **Commit YAML + migration + generated files** together

#### **Deprecating Items:**
- ✅ **6-month minimum deprecation period** (gives time for code updates)
- ✅ **Document replacement** clearly in schema YAML
- ✅ **Log warnings** when deprecated items are accessed
- ✅ **Update all code** before removal
- ✅ **Verify zero usages** before final removal
- ✅ **Create data migration** if data needs preservation

#### **Removing Items:**
- ✅ **Only remove after deprecation period**
- ✅ **Backup database** before removal migration
- ✅ **Test migration** on staging first
- ✅ **Verify data migration** completed (if applicable)
- ✅ **Remove from YAML** and regenerate
- ✅ **Remove from QueryRegistry** (if applicable)
- ✅ **Verify build succeeds** (compile errors catch remaining usages)

### 12.7. Common Scenarios

#### **Scenario 1: Rename Column**

**Workflow:**
1. Add new column with new name
2. Create data migration to copy data from old to new
3. Update all code to use new column
4. Deprecate old column (6 months)
5. Remove old column

**Example:**
```yaml
# Step 1: Add new column
patients:
  columns:
    dob:  # OLD - will be deprecated
      deprecated: true
      replacement: "date_of_birth"
    date_of_birth:  # NEW
      type: TEXT
      nullable: true
```

```sql
-- Step 2: Data migration
UPDATE patients SET date_of_birth = dob WHERE date_of_birth IS NULL;
```

#### **Scenario 2: Split Column**

**Workflow:**
1. Add new columns (first_name, last_name)
2. Create data migration to split full name
3. Update all code to use new columns
4. Deprecate old column (name)
5. Remove old column

#### **Scenario 3: Merge Tables**

**Workflow:**
1. Add columns to target table
2. Create data migration to copy data
3. Update all code to use target table
4. Deprecate source table (6 months)
5. Remove source table

## 13. Implementation Checklist

- [ ] Create `schema/database.yaml` with complete schema
- [ ] Create `scripts/generate_schema.py` (Python generator)
- [ ] Create `scripts/migrate.py` (migration runner with error handling)
- [ ] Update CMakeLists.txt to run generator
- [ ] Add pre-commit hook for schema sync
- [ ] Update all repositories to use `Schema::` constants
- [ ] Remove all hardcoded column name strings
- [ ] Integrate with QueryRegistry (use SchemaInfo constants in queries)
- [ ] Create initial migration (`0001_initial_schema.sql`)
- [ ] Add error handling to migration script (rollback, logging)
- [ ] Document workflow in this file
- [ ] Add to `ZTODO.md`

---

## 13. Integration with ORM (QxOrm)

> **📋 Implementation Status:**  
> ✅ **ORM integration is implemented.** QxOrm can be enabled via `-DUSE_QXORM=ON` CMake option. ORM mappings use Schema constants for type safety. See ZTODO.md task "Implement QxOrm Integration (Hybrid ORM + Stored Procedures)" for implementation details.

### **13.1 ORM Uses Schema Constants**

When QxOrm is integrated, the ORM registration **must use schema constants** to maintain single source of truth:

**File:** `z-monitor/src/infrastructure/persistence/orm/PatientAggregateMapping.h`

```cpp
#include "domain/aggregates/PatientAggregate.h"
#include "generated/SchemaInfo.h"  // ✅ Include schema constants
#include <QxOrm.h>

QX_REGISTER_HPP_EXPORT(PatientAggregate, qx::trait::no_base_class_defined, 0)

namespace qx {
    template<> void register_class(QxClass<PatientAggregate>& t) {
        // ✅ Use schema constants (not hardcoded strings)
        using namespace Schema::Tables;
        using namespace Schema::Columns::Patients;
        
        t.setName(PATIENTS);  // Table name from schema
        t.id(&PatientAggregate::mrn, MRN);  // Column names from schema
        t.data(&PatientAggregate::name, NAME);
        t.data(&PatientAggregate::dateOfBirth, DOB);
        t.data(&PatientAggregate::sex, SEX);
        t.data(&PatientAggregate::bedLocation, BED_LOCATION);
        t.data(&PatientAggregate::admittedAt, ADMITTED_AT);
        t.data(&PatientAggregate::admissionSource, ADMISSION_SOURCE);
    }
}
```

**Benefits:**
- ✅ Schema changes propagate to ORM (compile error if mapping outdated)
- ✅ No duplication of table/column names
- ✅ Type-safe column names in ORM registration
- ✅ Autocomplete works for ORM mapping

### **13.2 Workflow: Schema Change Affects ORM**

**Scenario:** Rename column `dob` → `date_of_birth`

**Step 1: Update Schema YAML**
```yaml
# schema/database.yaml
patients:
  columns:
    date_of_birth:  # ✅ Renamed from 'dob'
      type: TEXT
      nullable: true
      description: "Date of birth (ISO 8601: YYYY-MM-DD)"
```

**Step 2: Regenerate Schema**
```bash
python3 scripts/generate_schema.py
```

**Output:**
```cpp
// SchemaInfo.h
namespace Schema::Columns::Patients {
    constexpr const char* DATE_OF_BIRTH = "date_of_birth";  // ✅ Updated constant
    // Old constant 'DOB' removed → compile error if still used!
}
```

**Step 3: Build Fails (Good!)**
```bash
cmake --build build

# ❌ Compile error in PatientAggregateMapping.h:
# error: 'DOB' is not a member of 'Schema::Columns::Patients'
#     t.data(&PatientAggregate::dateOfBirth, DOB);
#                                            ^~~
```

**Step 4: Fix ORM Mapping**
```cpp
// PatientAggregateMapping.h
t.data(&PatientAggregate::dateOfBirth, DATE_OF_BIRTH);  // ✅ Updated to new constant
```

**Step 5: Create Migration**
```sql
-- schema/migrations/0005_rename_dob_column.sql
ALTER TABLE patients RENAME COLUMN dob TO date_of_birth;
```

**Step 6: Build Success**
```bash
cmake --build build  # ✅ Compiles successfully
python3 scripts/migrate.py --db data/zmonitor.db  # Apply migration
```

**Result:** Schema, ORM, and database all synchronized automatically

### **13.3 Directory Structure with ORM**

```
z-monitor/
├── schema/
│   ├── database.yaml                    # ⭐ Single source of truth
│   ├── migrations/
│   │   ├── 0001_initial_schema.sql
│   │   ├── 0002_add_adt_workflow.sql
│   │   └── 0005_rename_dob_column.sql   # Schema changes
│   └── generated/
│       └── ddl/
│           ├── create_tables.sql         # ✅ Generated
│           └── create_indices.sql        # ✅ Generated
├── src/
│   ├── domain/
│   │   └── aggregates/
│   │       └── PatientAggregate.h       # POCO (no schema dependencies)
│   └── infrastructure/
│       └── persistence/
│           ├── generated/
│           │   └── SchemaInfo.h         # ✅ Generated from YAML
│           ├── orm/
│           │   ├── PatientAggregateMapping.h   # ✅ Uses SchemaInfo.h
│           │   ├── VitalRecordMapping.h
│           │   └── OrmRegistry.cpp
│           ├── repositories/
│           │   ├── SQLitePatientRepository.cpp # ✅ Uses QxOrm or Qt SQL
│           │   └── SQLiteTelemetryRepository.cpp
│           └── DatabaseManager.cpp
└── scripts/
    ├── generate_schema.py               # Code generator
    └── migrate.py                       # Migration runner
```

### **13.4 Validation Script (Optional)**

Create a script to validate ORM mappings match schema:

```python
# scripts/validate_orm_schema.py
"""
Validates that ORM mappings use schema constants correctly.
"""
import yaml
import re
from pathlib import Path

def validate_orm_mappings():
    # Load schema
    with open('schema/database.yaml', 'r') as f:
        schema = yaml.safe_load(f)
    
    # Get all expected constants
    expected_constants = set()
    for table_name, table_def in schema['tables'].items():
        for col_name in table_def['columns'].keys():
            const_name = col_name.upper().replace('.', '_')
            expected_constants.add(const_name)
    
    # Parse ORM mapping files
    orm_dir = Path('src/infrastructure/persistence/orm')
    errors = []
    
    for mapping_file in orm_dir.glob('*Mapping.h'):
        with open(mapping_file, 'r') as f:
            content = f.read()
        
        # Extract all t.data() calls
        for match in re.finditer(r't\.data\([^,]+,\s*([A-Z_]+)\)', content):
            constant = match.group(1)
            if constant not in expected_constants:
                errors.append(f"{mapping_file.name}: Unknown constant '{constant}'")
    
    if errors:
        print("❌ ORM mapping validation FAILED:")
        for error in errors:
            print(f"   {error}")
        return False
    
    print("✅ All ORM mappings validated successfully")
    return True

if __name__ == "__main__":
    import sys
    sys.exit(0 if validate_orm_mappings() else 1)
```

**Run in CI/CD:**
```yaml
# .github/workflows/validate.yml
- name: Validate ORM Mappings
  run: python3 scripts/validate_orm_schema.py
```

---

## 14. Integration with Query Registry

### 14.1 SchemaInfo.h vs. QueryRegistry.h

**Both patterns work together:**

| Pattern             | Purpose                     | Generated From              | Use Case                                    |
| ------------------- | --------------------------- | --------------------------- | ------------------------------------------- |
| **SchemaInfo.h**    | Column/table name constants | `schema/database.yaml`      | Type-safe column names in queries           |
| **QueryRegistry.h** | Query ID constants          | `QueryCatalog.cpp` (manual) | Type-safe query IDs for prepared statements |

**Example: Using Both Together**

```cpp
// ✅ GOOD: Use SchemaInfo for column names, QueryRegistry for query IDs
#include "generated/SchemaInfo.h"
#include "QueryRegistry.h"

std::optional<PatientAggregate> SQLitePatientRepository::findByMrn(const QString& mrn) {
    // Query ID from QueryRegistry (type-safe)
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    
    // Column names from SchemaInfo (type-safe)
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        m_logService->error("Query failed", {
            {"queryId", QueryId::Patient::FIND_BY_MRN},
            {"mrn", mrn},
            {"error", query.lastError().text()}
        });
        return std::nullopt;
    }
    
    if (!query.next()) {
        return std::nullopt;
    }
    
    // Use SchemaInfo constants for column access (type-safe)
    PatientIdentity identity(
        query.value(Schema::Columns::Patients::MRN).toString(),
        query.value(Schema::Columns::Patients::NAME).toString(),
        query.value(Schema::Columns::Patients::DOB).toDate(),
        query.value(Schema::Columns::Patients::SEX).toString()
    );
    
    return PatientAggregate(identity);
}
```

**Key Insight:**
- **SchemaInfo.h** = Column/table names (generated from YAML)
- **QueryRegistry.h** = Query IDs (manual constants in C++)
- **Both provide type safety** and autocomplete
- **Use together** for complete type-safe database access

### 14.2 Workflow: Schema Change Affects Both

**Scenario:** Add new column `emergency_contact` to `patients` table

**Step 1: Update Schema YAML**
```yaml
# schema/database.yaml
patients:
  columns:
    emergency_contact:
      type: TEXT
      nullable: true
```

**Step 2: Regenerate SchemaInfo.h**
```bash
python3 scripts/generate_schema.py
```

**Output:** New constant in `SchemaInfo.h`
```cpp
namespace Schema::Columns::Patients {
    constexpr const char* EMERGENCY_CONTACT = "emergency_contact";  // ✅ New constant
}
```

**Step 3: Update QueryRegistry (if needed)**
```cpp
// QueryRegistry.h - No change needed (query IDs unchanged)
// QueryCatalog.cpp - Update SQL if query needs new column
queries[QueryId::Patient::FIND_BY_MRN] = {
    .sql = R"(
        SELECT mrn, name, dob, sex, emergency_contact  -- ✅ Added new column
        FROM patients
        WHERE mrn = :mrn
    )",
    // ...
};
```

**Step 4: Use in Repository**
```cpp
// ✅ Use both constants
QString emergency = query.value(Schema::Columns::Patients::EMERGENCY_CONTACT).toString();
```

## 15. Error Handling in Migrations

### 15.1 Migration Error Handling

**Critical:** Migration failures must be handled carefully to prevent database corruption.

**Error Handling Strategy:**
- ✅ **Log all migration errors** (infrastructure failures need audit trail)
- ✅ **Rollback on failure** (transaction safety)
- ✅ **Stop migration process** (don't continue with partial migration)
- ✅ **Return error status** (caller needs to know migration failed)

**See:** [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) Section 4 for guidelines on when to return vs. log vs. emit errors.

**Example: Enhanced Migration Script with Error Handling**

```python
def apply_migration(conn: sqlite3.Connection, migration_file: Path, version: int) -> bool:
    """Apply a single migration file.
    
    Returns:
        True if successful, False if failed
    """
    print(f"📦 Applying migration {version}: {migration_file.name}")
    
    sql = None
    try:
        # Read migration file
        with open(migration_file, 'r') as f:
            sql = f.read()
        
        # Begin transaction
        conn.execute("BEGIN TRANSACTION")
        
        cursor = conn.cursor()
        cursor.executescript(sql)
        
        # Record migration in schema_version table
        cursor.execute("""
            INSERT INTO schema_version (version, applied_at, description)
            VALUES (?, datetime('now'), ?)
        """, (version, migration_file.stem))
        
        # Commit transaction
        conn.commit()
        print(f"✅ Migration {version} applied successfully")
        return True
        
    except sqlite3.Error as e:
        # ✅ Log database error (infrastructure failure)
        conn.rollback()
        print(f"❌ Migration {version} FAILED (database error): {e}")
        print(f"   File: {migration_file}")
        if sql:
            print(f"   SQL: {sql[:200]}...")  # First 200 chars for debugging
        return False  # ✅ Return error status
        
    except FileNotFoundError as e:
        # ✅ Log file error (infrastructure failure)
        print(f"❌ Migration {version} FAILED (file not found): {e}")
        return False
        
    except Exception as e:
        # ✅ Log unexpected error (infrastructure failure)
        conn.rollback()
        print(f"❌ Migration {version} FAILED (unexpected error): {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Database Migration Runner')
    parser.add_argument('--db', required=True, help='Path to database file')
    args = parser.parse_args()
    
    migrations_dir = Path("schema/migrations")
    conn = sqlite3.connect(args.db)
    
    current_version = get_current_version(conn)
    print(f"📊 Current schema version: {current_version}")
    
    # Get all migration files
    migration_files = sorted(migrations_dir.glob("*.sql"))
    
    for migration_file in migration_files:
        version = int(migration_file.stem.split('_')[0])
        
        if version > current_version:
            success = apply_migration(conn, migration_file, version)
            if not success:
                # ✅ Stop on first failure (don't continue with partial migration)
                print(f"❌ Migration process stopped at version {version}")
                conn.close()
                sys.exit(1)  # ✅ Return error status
    
    final_version = get_current_version(conn)
    print(f"✅ Database is up to date (version {final_version})")
    
    conn.close()
    sys.exit(0)
```

### 15.2 Migration Error Categories

| Error Type               | Handling       | Log?  | Return? | Rationale                            |
| ------------------------ | -------------- | ----- | ------- | ------------------------------------ |
| **SQL syntax error**     | Rollback, stop | ✅ Yes | ✅ Yes   | Infrastructure failure, needs audit  |
| **Constraint violation** | Rollback, stop | ✅ Yes | ✅ Yes   | Data integrity issue, needs audit    |
| **File not found**       | Stop           | ✅ Yes | ✅ Yes   | Infrastructure failure, needs audit  |
| **Database locked**      | Retry or stop  | ✅ Yes | ✅ Yes   | Infrastructure failure, needs audit  |
| **Disk full**            | Rollback, stop | ✅ Yes | ✅ Yes   | Critical system failure, needs audit |

**Key Principle:** All migration errors are infrastructure failures and should be logged and returned (see [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) Section 4.3).

## 16. References

- **[30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md)** – **Repository pattern and ORM integration** ⭐
- **[32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md)** – Query string management (works with SchemaInfo.h)
- **[20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md)** – Error handling guidelines (when to return vs. log vs. emit errors)
- **[34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md)** – Complete migration workflow (schema + data migrations)
- **[10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md)** – Database schema
- YAML Specification: https://yaml.org/spec/1.2.2/
- SQLite Documentation: https://www.sqlite.org/lang.html
- QxOrm Documentation: https://www.qxorm.com/

---

*This document defines the schema management strategy for Z Monitor. All schema changes must go through YAML → code generation → migration workflow.*

