---
doc_id: DOC-ARCH-005
title: Data Flow and Caching Strategy
version: 1.0
category: Architecture
subcategory: Data Management
status: Draft
owner: Architecture Team
reviewers: [Infrastructure, Application]
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-001_software_architecture.md
  - DOC-ARCH-006_system_overview.md
  - DOC-ARCH-011_thread_model.md
  - DOC-COMP-026_data_caching_strategy.md
tags: [data-flow, caching, sync, telemetry, persistence]
source:
  path: project-dashboard/doc/z-monitor/architecture_and_design/11_DATA_FLOW_AND_CACHING.md
  original_id: DESIGN-011
  last_updated: 2025-11-27
---

# Purpose
Defines data lifecycle from generation through caching, persistence, and synchronization with central server. Covers in-memory buffering, batch writes, network sync protocol, and patient association requirements.

# Data Flow Path

```mermaid
sequenceDiagram
    participant Sim as DeviceSimulator
    participant Buffer as In-Memory Buffer
    participant DB as Local SQLite DB
    participant Net as NetworkManager
    participant Server as Central Server

    Note over Sim, Buffer: High Frequency (250Hz)
    Sim->>Buffer: 1. Generate PatientData
    
    Note over Buffer, DB: Periodic (1-2s)
    Buffer->>DB: 2. Batch Write (is_synced=false)
    
    Note over DB, Server: Sync Interval (e.g. 10s)
    Net->>DB: 3. Get Unsynced Data
    DB-->>Net: Return Records
    Net->>Server: 4. POST /api/telemetry (JSON)
    
    alt Success
        Server-->>Net: 200 OK (processed_ids)
        Net->>DB: 5. Mark as Synced (is_synced=true)
    else Failure
        Server-->>Net: 500 Error / Timeout
        Note right of Net: Retry next cycle
    end
```

1. **Generation**: DeviceSimulator produces PatientData at high frequency.
2. **In-Memory Buffering**: Short-term (1-2s) buffer before batch persistence.
3. **Batch Cache**: Periodic writes to `vitals` table (local SQLite/SQLCipher).
4. **Synchronization**:
   - NetworkManager retrieves unsynced data (`is_synced = false`).
   - Batch JSON payload sent to `/api/telemetry`.
   - Server responds with processed IDs.
   - Local DB updated (`is_synced = true`) only for confirmed records.
   - On failure: unsynced data retained for next attempt.

# Server Payload Structure
```json
{
  "deviceId": "DEVICE-SERIAL-001",
  "deviceLabel": "ICU-MON-04",
  "patientMrn": "12345",
  "patientName": "John Doe",
  "bedLocation": "ICU-4B",
  "timestamp": "2025-11-22T14:30:00Z",
  "vitals": [
    {"id": 101, "timestamp": "2025-11-22T14:29:58Z", "heart_rate": 78, "spo2": 98.5}
  ],
  "alarms": [
    {"id": 201, "timestamp": "2025-11-22T14:30:00Z", "alarmType": "high_heart_rate", "priority": "high"}
  ]
}
```

**Critical Requirements:**
- Device identity: `deviceId`, `deviceLabel`.
- Patient association: `patientMrn`, `patientName`, `bedLocation` (REQUIRED for all patient data).
- Standby state: no patient telemetry sent if `patientMrn` is empty.

# Server Response
- Success: `{"status": "success", "processed_ids": [101, 102]}`.
- Failure: empty/invalid ID list; local DB unchanged; retry on next sync.

# Patient Association Requirements
- All telemetry MUST include `patientMrn` for correlation, compliance (HIPAA, IEC 62304), audit trails.
- `bedLocation` from Patient object; reflects current assignment.

# Integration Points
- DeviceSimulator (data generation).
- DatabaseManager (getUnsyncedData, markDataAsSynced).
- NetworkManager (sync orchestration).
- See DOC-ARCH-011 for thread assignments.

# Verification
- Functional: simulate generation, buffer, batch write, sync success/failure scenarios.
- Code Quality: enforce patient association validation; no hardcoded sync intervals.
- Documentation: diagram data flow and sync state machine.
- Integration: end-to-end sync test with mock server; verify partial success handling.
- Tests: unit tests for batch logic, sync protocol; integration tests for network failures.

# Document Metadata
| Field          | Value        |
| -------------- | ------------ |
| Original Doc   | DESIGN-011   |
| Migration Date | 2025-12-01   |
| New Doc ID     | DOC-ARCH-005 |

# Revision History
- 1.0 (2025-12-01): Migrated from 11_DATA_FLOW_AND_CACHING.md; aligned with architecture and threading.
