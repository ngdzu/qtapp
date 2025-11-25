# ADT (Admission, Discharge, Transfer) Workflow

This document describes the patient admission, discharge, and transfer workflow for the Z Monitor, which replaces the previous "Bed ID" configuration approach with a proper ADT workflow aligned with hospital information systems (HIS).

## 1. Overview

The Z Monitor implements an ADT workflow that properly separates device identity from patient assignment, following industry-standard hospital admission processes.

### Key Concepts

- **Device Identity:** Each monitor has a fixed Asset Tag or Device Label (e.g., "ICU-MON-04") that rarely changes. This is a technical identifier for the device itself.
- **Patient Identity:** Patients are identified by MRN (Medical Record Number), which is the primary identifier in hospital systems.
- **Association:** When a patient arrives, the clinician "Admits" the patient to the monitor, creating an association between the device and the patient.

## 2. Workflow

### 2.1. Patient Admission

When a patient needs to be monitored, the clinician admits the patient to the device using one of three methods:

#### Option A: Manual Entry
- Clinician types the patient's Name or MRN directly into the Admission Modal
- System looks up patient information from HIS/EHR
- Clinician confirms patient details and admits

#### Option B: Barcode Scan
- Clinician scans the patient's wristband barcode
- System extracts MRN from barcode
- System looks up patient information from HIS/EHR
- Clinician confirms patient details and admits

#### Option C: Central Station Push
- Central Station (HIS/EHR) sends admission notification to the device
- Device receives: "Patient John Doe (MRN: 12345) is now in Bed 4, and you are the monitor at Bed 4"
- Device automatically admits the patient (with optional clinician confirmation)

### 2.2. Patient Discharge

When a patient is discharged:

1. **Manual Discharge:** Clinician selects "Discharge Patient" from the Patient Banner or Settings
2. **Central Station Push:** Central Station sends discharge notification
3. **Automatic Discharge:** Device automatically discharges when patient is removed from bed (if bed sensor available)

After discharge:
- Patient data is preserved for audit (retention policy applies)
- Device enters "STANDBY" state
- Header displays "DISCHARGED / STANDBY"
- Device is ready for next patient admission

### 2.3. Patient Transfer

When a patient is transferred to a different bed/monitor:

1. **Source Device:** Receives transfer notification, discharges patient
2. **Target Device:** Receives admission notification, admits patient
3. **Data Continuity:** Patient data may be transferred or remain on source device (configurable)

## 3. Patient Object Model

### 3.1. Patient Data Structure

```cpp
struct Patient {
    QString mrn;              // Medical Record Number (primary identifier)
    QString name;             // Full name
    QDate dateOfBirth;        // Date of birth
    QString sex;              // "M", "F", or other
    QStringList allergies;    // List of known allergies
    QString bedLocation;      // Current bed/room assignment (e.g., "ICU-4B")
    QDateTime admittedAt;     // Timestamp of admission
    QDateTime dischargedAt;    // Timestamp of discharge (NULL if currently admitted)
    QString admissionSource;  // "manual", "barcode", "central_station"
};
```

### 3.2. Patient States

- **Not Admitted:** No patient associated with device (STANDBY state)
- **Admitted:** Patient is currently being monitored
- **Discharged:** Patient has been discharged but data retained for audit

## 4. UI Components

### 4.1. Admission Modal

**Location:** Accessible from Patient Banner (when no patient admitted) or Settings → Patient Management

**Components:**
- **Admission Method Selection:**
  - Radio buttons or tabs: "Manual Entry", "Barcode Scan", "Central Station"
- **Manual Entry:**
  - Text input for MRN or Patient Name
  - "Lookup" button to search HIS/EHR
- **Barcode Scanner:**
  - Camera view for barcode scanning
  - Automatic MRN extraction and lookup
- **Patient Preview:**
  - Displays retrieved patient information (Name, DOB, Sex, Allergies, Bed Location)
  - Editable fields for bed location override
- **Action Buttons:**
  - "Admit Patient" (confirms admission)
  - "Cancel" (closes modal without admitting)

**Behavior:**
- On successful lookup, patient information is displayed for confirmation
- Clinician can override bed location if different from HIS assignment
- Admission creates patient record and associates with device
- All admission events logged to `audit_log` and `security_audit_log`

### 4.2. Patient Banner (Updated)

**Location:** Header, top-left section

**Display States:**

**When Patient Admitted:**
- **Patient Name:** Prominently displayed (large, bold font)
- **MRN:** Displayed below name (smaller font)
- **Bed Location:** Displayed as badge or secondary text
- **Allergies:** Displayed if present (highlighted in red/orange)
- **Admission Time:** Displayed (e.g., "Admitted: 2h 15m ago")

**When No Patient Admitted:**
- **Status Text:** "DISCHARGED / STANDBY" (prominently displayed)
- **Action:** Tappable to open Admission Modal

**Interaction:**
- Tap/click opens Patient Details view or Admission Modal (depending on state)
- Long press opens quick actions menu (Discharge, Transfer, View History)

### 4.3. Settings View Updates

**Device Configuration Section:**
- **Device ID:** Renamed from "Device ID" to "Device Label" (static technical identifier)
- **Device Label:** Read-only or editable only by Technician role (e.g., "ICU-MON-04")
- **Removed:** "Bed ID" field (no longer in Settings, now part of Patient object)

**Patient Management Section (New):**
- **Current Patient:** Display current patient information (if admitted)
- **Admission Actions:**
  - "Admit Patient" button (opens Admission Modal)
  - "Discharge Patient" button (if patient admitted)
  - "Transfer Patient" button (if patient admitted)
- **Patient History:** Link to view admission/discharge history

## 5. Database Schema Updates

### 5.1. Patients Table Enhancement

Add columns to track admission/discharge:

```sql
ALTER TABLE patients ADD COLUMN bed_location TEXT NULL;
ALTER TABLE patients ADD COLUMN admitted_at INTEGER NULL;
ALTER TABLE patients ADD COLUMN discharged_at INTEGER NULL;
ALTER TABLE patients ADD COLUMN admission_source TEXT NULL;  -- "manual", "barcode", "central_station"
ALTER TABLE patients ADD COLUMN device_label TEXT NULL;      -- Device that admitted this patient
```

### 5.2. Settings Table Update

Remove `bedId` setting, add `deviceLabel`:

```sql
-- Remove bedId (no longer needed)
DELETE FROM settings WHERE key = 'bedId';

-- Add deviceLabel (static device identifier)
INSERT INTO settings (key, value, updated_at) 
VALUES ('deviceLabel', 'ICU-MON-04', strftime('%s', 'now'));
```

### 5.3. Admission Events Table (New)

Track admission/discharge events for audit:

```sql
CREATE TABLE IF NOT EXISTS admission_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    event_type TEXT NOT NULL,  -- "admission", "discharge", "transfer"
    patient_mrn TEXT NOT NULL,
    patient_name TEXT NULL,
    device_label TEXT NOT NULL,
    bed_location TEXT NULL,
    admission_source TEXT NULL,
    user_id TEXT NULL,  -- Clinician who performed the action
    details TEXT NULL    -- Additional event details (JSON)
);

CREATE INDEX IF NOT EXISTS idx_admission_events_patient ON admission_events(patient_mrn, timestamp);
CREATE INDEX IF NOT EXISTS idx_admission_events_device ON admission_events(device_label, timestamp);
```

## 6. API Integration

### 6.1. Central Station Push (Option C)

**Endpoint:** Device receives admission/discharge notifications via secure channel

**Admission Notification:**
```json
{
  "event": "admission",
  "patient": {
    "mrn": "12345",
    "name": "John Doe",
    "dob": "1980-01-15",
    "sex": "M",
    "allergies": ["Penicillin"],
    "bedLocation": "ICU-4B"
  },
  "deviceLabel": "ICU-MON-04",
  "timestamp": "2024-11-25T14:30:00Z",
  "source": "central_station"
}
```

**Discharge Notification:**
```json
{
  "event": "discharge",
  "patient": {
    "mrn": "12345"
  },
  "deviceLabel": "ICU-MON-04",
  "timestamp": "2024-11-25T18:45:00Z",
  "source": "central_station"
}
```

### 6.2. Patient Lookup (HIS/EHR Integration)

**Interface:** `IPatientLookupService`

**Lookup by MRN:**
- Device queries HIS/EHR with MRN
- Returns patient demographics, allergies, current bed assignment
- Device caches result in local `patients` table

## 7. Implementation Components

### 7.1. PatientManager Updates

**New Methods:**
- `admitPatient(const PatientInfo& info, const QString& admissionSource)`: Admits patient to device
- `dischargePatient(const QString& mrn)`: Discharges patient from device
- `transferPatient(const QString& mrn, const QString& targetDevice)`: Transfers patient to another device
- `getCurrentAdmission()`: Returns current admission information
- `isPatientAdmitted()`: Returns whether a patient is currently admitted
- `getAdmissionHistory(const QString& mrn)`: Returns admission/discharge history for patient

**Updated Properties:**
- `currentPatient`: Patient object (includes bedLocation)
- `admissionState`: Current admission state (NotAdmitted, Admitted, Discharged)

### 7.2. PatientController Updates

**New Properties (Q_PROPERTY):**
- `admissionState`: Current admission state (enum)
- `bedLocation`: Current patient's bed location (QString, read-only)
- `admittedAt`: Timestamp of admission (QDateTime, read-only)
- `isAdmitted`: Whether patient is currently admitted (bool, read-only)

**New Q_INVOKABLE Methods:**
- `openAdmissionModal()`: Opens admission modal
- `admitPatient(const QString& mrn, const QString& admissionSource)`: Admits patient
- `dischargePatient()`: Discharges current patient
- `scanBarcode()`: Initiates barcode scanning for admission

### 7.3. SettingsManager Updates

**Removed:**
- `bedId` setting (no longer exists)

**Added:**
- `deviceLabel` setting (static device identifier, e.g., "ICU-MON-04")

**Updated:**
- `deviceId` remains for telemetry/device identification
- `deviceLabel` is the human-readable asset tag/label

## 8. Security and Audit

### 8.1. Audit Logging

All admission/discharge events are logged:

- **Admission Events:** Logged to `admission_events` table and `audit_log`
- **Discharge Events:** Logged to `admission_events` table and `audit_log`
- **Transfer Events:** Logged to `admission_events` table and `audit_log`
- **Security Events:** Admission/discharge from Central Station logged to `security_audit_log`

### 8.2. Access Control

- **Admission:** Requires Clinician or Technician role
- **Discharge:** Requires Clinician or Technician role
- **Transfer:** Requires Technician role (higher privilege)
- **Device Label Change:** Requires Technician role

## 9. Migration from Bed ID

### 9.1. Data Migration

For existing devices with `bedId` setting:

1. **Extract bedId:** Read current `bedId` from settings
2. **Create deviceLabel:** Use `bedId` as initial `deviceLabel` or prompt for new label
3. **Clear bedId:** Remove `bedId` from settings
4. **Update existing patients:** If any patients have `bedId` in their record, migrate to `bedLocation` in patient object

### 9.2. UI Migration

- Remove "Bed ID" input from Settings View
- Add "Device Label" display (read-only or Technician-editable)
- Update Patient Banner to show patient name prominently
- Add "DISCHARGED / STANDBY" state to header
- Add Admission Modal component

## 10. Testing Requirements

### 10.1. Unit Tests

- PatientManager admission/discharge logic
- Patient object creation and validation
- Bed location assignment and override
- Admission source tracking

### 10.2. Integration Tests

- Admission via manual entry
- Admission via barcode scan
- Admission via Central Station push
- Discharge workflow
- Transfer workflow
- Patient lookup from HIS/EHR

### 10.3. E2E Tests

- Complete admission workflow (lookup → confirm → admit)
- Discharge workflow
- Header state changes (admitted → discharged)
- Settings view updates
- Audit log verification

## 11. Benefits

### 11.1. Alignment with Hospital Workflows

- Matches standard ADT processes used in hospitals
- Integrates with HIS/EHR systems
- Supports barcode scanning (common in hospitals)

### 11.2. Improved Data Model

- Separates device identity from patient assignment
- Proper patient lifecycle management
- Better audit trail for patient assignments

### 11.3. Enhanced User Experience

- Clear visual indication of patient state
- Prominent patient name display
- Streamlined admission process
- Support for multiple admission methods

