# Database Design

This document outlines the design of the local SQLite database used by the Medical Device Dashboard application.

## 1. Entity Relationship Diagram (ERD)

The following diagram illustrates the tables, columns, and relationships within the database.

[View the full Database Schema Diagram (interactive)](./10_DATABASE_DESIGN.mmd)

## 2. Table Descriptions

### `patients`
Stores patient demographic information.

| Column | Type | Constraints | Description |
|---|---|---|---|
| `patient_id` | TEXT | PRIMARY KEY | Unique identifier for the patient (e.g., MRN). |
| `name` | TEXT | NOT NULL | Patient's full name. |
| `age` | INTEGER | | Patient's age. |
| `allergies` | TEXT | | JSON array or comma-separated list of known allergies. |

### `vitals`
Stores time-series physiological data. This is expected to be the largest table.

| Column | Type | Constraints | Description |
|---|---|---|---|
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique identifier for the vital record. |
| `timestamp` | INTEGER | NOT NULL | Unix epoch timestamp of the reading. |
| `patient_id` | TEXT | NOT NULL, FOREIGN KEY(patients) | Links to the patient record. |
| `heart_rate` | REAL | | Heart rate in BPM. |
| `spo2` | REAL | | Oxygen saturation in %. |
| `respiration_rate` | REAL | | Breaths per minute. |
| `is_synced` | BOOLEAN | DEFAULT 0 | Flag to track if the record has been sent to the server. |
| `...` | REAL | | Other vital signs. |

### `alarms`
Stores a history of all alarm events.

| Column | Type | Constraints | Description |
|---|---|---|---|
| `alarm_id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique ID for the alarm event. |
| `patient_id` | TEXT | NOT NULL, FOREIGN KEY(patients) | Links to the patient record. |
| `start_time` | INTEGER | NOT NULL | Unix epoch timestamp when the alarm was triggered. |
| `end_time` | INTEGER | | Unix epoch timestamp when the alarm condition cleared. |
| `alarm_type` | TEXT | NOT NULL | The type of alarm (e.g., "HR_HIGH", "SPO2_LOW"). |
| `priority` | TEXT | NOT NULL | The priority level (e.g., "CRITICAL", "HIGH"). |
| `status` | TEXT | | The final status (e.g., "Acknowledged", "Silenced"). |

### `snapshots`
Stores captured waveform data.

| Column | Type | Constraints | Description |
|---|---|---|---|
| `snapshot_id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique ID for the snapshot. |
| `patient_id` | TEXT | NOT NULL, FOREIGN KEY(patients) | Links to the patient record. |
| `capture_time` | INTEGER | NOT NULL | Unix epoch timestamp when the snapshot was taken. |
| `waveform_type` | TEXT | NOT NULL | Type of waveform (e.g., "ECG_II", "PLETH"). |
| `data` | BLOB | NOT NULL | Raw waveform data, likely stored as a serialized list of points. |

### `annotations`
Stores user-added notes for specific snapshots.

| Column | Type | Constraints | Description |
|---|---|---|---|
| `annotation_id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique ID for the annotation. |
| `snapshot_id` | INTEGER | NOT NULL, FOREIGN KEY(snapshots) | Links to the specific snapshot. |
| `author` | TEXT | | The user who created the annotation. |
| `note` | TEXT | NOT NULL | The content of the annotation. |
| `creation_time` | INTEGER | NOT NULL | Unix epoch timestamp. |
