# Database Schema Management and Code Generation

This document defines the schema management strategy for Z Monitor, including schema definition files, code generation, migration workflow, and tooling.

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

| Problem | Solution | Benefit |
|---------|----------|---------|
| Hardcoded column names | Schema constants | ✅ Autocomplete, compile-time checking |
| Schema in multiple places | YAML single source | ✅ One place to edit |
| Manual DDL writing | Generated SQL | ✅ No typos, consistent |
| Hard to find column usages | Named constants | ✅ Ctrl+Click finds all |
| Schema drift | Pre-commit hook | ✅ Always synchronized |
| Manual migrations | Migration runner | ✅ Automated, tracked |
| Hard to refactor columns | Constants + generator | ✅ Rename in YAML → updates everywhere |

---

## 12. Implementation Checklist

- [ ] Create `schema/database.yaml` with complete schema
- [ ] Create `scripts/generate_schema.py` (Python generator)
- [ ] Create `scripts/migrate.py` (migration runner)
- [ ] Update CMakeLists.txt to run generator
- [ ] Add pre-commit hook for schema sync
- [ ] Update all repositories to use `Schema::` constants
- [ ] Remove all hardcoded column name strings
- [ ] Create initial migration (`0001_initial_schema.sql`)
- [ ] Document workflow in this file
- [ ] Add to `ZTODO.md`

---

## 13. References

- `doc/32_QUERY_REGISTRY.md` – Query string management (similar pattern)
- `doc/30_DATABASE_ACCESS_STRATEGY.md` – Repository pattern
- `doc/10_DATABASE_DESIGN.md` – Database schema
- YAML Specification: https://yaml.org/spec/1.2.2/
- SQLite Documentation: https://www.sqlite.org/lang.html

---

*This document defines the schema management strategy for Z Monitor. All schema changes must go through YAML → code generation → migration workflow.*

