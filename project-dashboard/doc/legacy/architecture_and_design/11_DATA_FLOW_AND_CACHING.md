# Data Flow and Caching Strategy

**Document ID:** DESIGN-011  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document outlines the lifecycle of data within the Z Monitor, from generation to on-device caching and synchronization with the central server.

## 1. Data Flow Diagram

The following diagram illustrates the data flow.

[View the full Data Flow Diagram (interactive)](./11_DATA_FLOW_AND_CACHING.mmd)

## 2. Data Flow Explained

1.  **Generation:** The `DeviceSimulator` generates new `PatientData` objects at a high frequency.
2.  **In-Memory Buffering:** Data is first collected in a small, in-memory buffer for a short period (e.g., 1-2 seconds).
3.  **Batch Cache:** Periodically, the contents of the in-memory buffer are written to the `vitals` table in the local SQLite database in a single, efficient transaction. This database acts as the persistent on-device cache.
4.  **Synchronization:**
    -   The `NetworkManager` periodically attempts to sync data with the server.
    -   It calls `DatabaseManager::getUnsyncedData()` to retrieve a batch of records (including their unique `id`s) where the `is_synced` flag is `false`.
    -   This batch of records is packaged into a single JSON payload.
    -   The `NetworkManager` sends this payload to the server.
    -   **Server Response:** The server's `POST /api/telemetry` endpoint is expected to return a JSON response containing a list of `id`s for the records it successfully received and processed.
    -   **On Success:** The `NetworkManager` uses the list of `id`s from the server's response to call `DatabaseManager::markDataAsSynced()`, updating the `is_synced` flag to `true` only for those confirmed records.
    -   **On Failure:** If the network is down or the server returns an error (or an empty/invalid list of IDs), no changes are made to the local database for those records. The unsynced data remains and will be included in the next synchronization attempt, ensuring data robustness and preventing duplicate transmissions of unconfirmed data.

## 3. Server Payload Definition

The data sent to the server (`/api/telemetry`) is a JSON object with the following structure:

```json
{
  "deviceId": "DEVICE-SERIAL-001",
  "deviceLabel": "ICU-MON-04",
  "patientMrn": "12345",
  "patientName": "John Doe",
  "bedLocation": "ICU-4B",
  "timestamp": "2025-11-22T14:30:00Z",
  "vitals": [
    {
      "id": 101,
      "timestamp": "2025-11-22T14:29:58Z",
      "patientMrn": "12345",
      "heart_rate": 78,
      "spo2": 98.5
    },
    {
      "id": 102,
      "timestamp": "2025-11-22T14:29:59Z",
      "patientMrn": "12345",
      "heart_rate": 79,
      "spo2": 98.6
    }
  ],
  "alarms": [
    {
      "id": 201,
      "timestamp": "2025-11-22T14:30:00Z",
      "patientMrn": "12345",
      "alarmType": "high_heart_rate",
      "priority": "high"
    }
  ]
}
```

**Critical Requirements:**
- **Patient Association:** All telemetry data MUST include `patientMrn` (Medical Record Number) to associate data with the patient. This is required for:
  - Patient data correlation on the server
  - Compliance with healthcare data regulations
  - Proper patient record management
  - Audit trails and data integrity
- **Device Identity:** `deviceId` (for telemetry routing) and `deviceLabel` (human-readable asset tag) are both included
- **Bed Location:** `bedLocation` comes from the Patient object (not device settings) and reflects current patient assignment
- **Standby State:** If no patient is admitted (`patientMrn` is empty), the device should not send patient telemetry data. Only device health/status data may be sent (clearly marked as non-patient data)

## 4. Server Response for Telemetry Upload

Upon successful processing of the telemetry data, the server is expected to respond with a JSON object containing the IDs of the records it successfully received:

```json
{
  "status": "success",
  "processed_ids": [101, 102]
}
```
