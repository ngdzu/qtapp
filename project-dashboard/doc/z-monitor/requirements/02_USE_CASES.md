# Use Cases

**Document ID:** REQ-DOC-02  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document describes detailed use cases for the Z Monitor patient monitoring system. Each use case represents a specific interaction between actors and the system to achieve a goal.

**Related Documents:**
- **Stakeholders:** [01_STAKEHOLDERS_AND_USERS.md](./01_STAKEHOLDERS_AND_USERS.md)
- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](./03_FUNCTIONAL_REQUIREMENTS.md)

---

## 2. Use Case Index

### 2.1 Patient Management (PM)
- [UC-PM-001](#uc-pm-001-admit-patient-to-monitor) Admit Patient to Monitor
- [UC-PM-002](#uc-pm-002-discharge-patient-from-monitor) Discharge Patient from Monitor
- [UC-PM-003](#uc-pm-003-transfer-patient-between-monitors) Transfer Patient Between Monitors
- [UC-PM-004](#uc-pm-004-lookup-patient-information) Lookup Patient Information
- [UC-PM-005](#uc-pm-005-update-patient-information) Update Patient Information

### 2.2 Vital Signs Monitoring (VM)
- [UC-VM-001](#uc-vm-001-display-real-time-vital-signs) Display Real-Time Vital Signs
- [UC-VM-002](#uc-vm-002-view-vital-signs-trends) View Vital Signs Trends
- [UC-VM-003](#uc-vm-003-record-manual-vital-signs) Record Manual Vital Signs
- [UC-VM-004](#uc-vm-004-export-vital-signs-data) Export Vital Signs Data

### 2.3 Alarm Management (AM)
- [UC-AM-001](#uc-am-001-trigger-vital-sign-alarm) Trigger Vital Sign Alarm
- [UC-AM-002](#uc-am-002-acknowledge-alarm) Acknowledge Alarm
- [UC-AM-003](#uc-am-003-silence-alarm) Silence Alarm
- [UC-AM-004](#uc-am-004-configure-alarm-thresholds) Configure Alarm Thresholds
- [UC-AM-005](#uc-am-005-view-alarm-history) View Alarm History
- [UC-AM-006](#uc-am-006-escalate-unacknowledged-alarm) Escalate Unacknowledged Alarm

### 2.4 User Authentication (UA)
- [UC-UA-001](#uc-ua-001-login-to-system) Login to System
- [UC-UA-002](#uc-ua-002-logout-from-system) Logout from System
- [UC-UA-003](#uc-ua-003-auto-logout-on-timeout) Auto-Logout on Timeout
- [UC-UA-004](#uc-ua-004-lock-screen) Lock Screen
- [UC-UA-005](#uc-ua-005-unlock-screen) Unlock Screen

### 2.5 Device Provisioning (DP)
- [UC-DP-001](#uc-dp-001-provision-new-device) Provision New Device
- [UC-DP-002](#uc-dp-002-scan-provisioning-qr-code) Scan Provisioning QR Code
- [UC-DP-003](#uc-dp-003-apply-device-configuration) Apply Device Configuration
- [UC-DP-004](#uc-dp-004-renew-device-certificate) Renew Device Certificate
- [UC-DP-005](#uc-dp-005-revoke-device-certificate) Revoke Device Certificate

### 2.6 System Configuration (SC)
- [UC-SC-001](#uc-sc-001-configure-device-settings) Configure Device Settings
- [UC-SC-002](#uc-sc-002-update-firmware) Update Firmware
- [UC-SC-003](#uc-sc-003-run-system-diagnostics) Run System Diagnostics
- [UC-SC-004](#uc-sc-004-view-system-logs) View System Logs
- [UC-SC-005](#uc-sc-005-backup-device-data) Backup Device Data

### 2.7 Data Synchronization (DS)
- [UC-DS-001](#uc-ds-001-sync-vitals-to-server) Sync Vitals to Server
- [UC-DS-002](#uc-ds-002-sync-alarms-to-server) Sync Alarms to Server
- [UC-DS-003](#uc-ds-003-handle-sync-failure) Handle Sync Failure
- [UC-DS-004](#uc-ds-004-recover-from-network-outage) Recover from Network Outage

### 2.8 Emergency Scenarios (ES)
- [UC-ES-001](#uc-es-001-handle-critical-alarm-code-blue) Handle Critical Alarm (Code Blue)
- [UC-ES-002](#uc-es-002-operate-during-network-outage) Operate During Network Outage
- [UC-ES-003](#uc-es-003-recover-from-power-loss) Recover from Power Loss
- [UC-ES-004](#uc-es-004-handle-sensor-disconnection) Handle Sensor Disconnection

---

## 3. Use Case Template

Each use case follows this structure:

```
### UC-XXX-### Use Case Name

**ID:** UC-XXX-###
**Priority:** Critical | High | Medium | Low
**Status:** Draft | Approved | Implemented | Verified

**Primary Actor:** [User role]
**Secondary Actors:** [Other involved roles or systems]
**Stakeholders:** [Who has interest in this]

**Preconditions:**
- [What must be true before this use case can start]

**Postconditions:**
Success:
- [What is true after successful completion]
Failure:
- [What is true after failure]

**Normal Flow:**
1. [Step 1]
2. [Step 2]
3. [Step 3]

**Alternative Flows:**
A1: [Alternative scenario 1]
A2: [Alternative scenario 2]

**Exception Flows:**
E1: [Error condition 1]
E2: [Error condition 2]

**Business Rules:**
- [Rule 1]
- [Rule 2]

**Non-Functional Requirements:**
- Performance: [timing requirements]
- Security: [security requirements]
- Usability: [usability requirements]

**Notes:**
[Additional context]

**Traces To:**
- Requirements: REQ-FUN-###, REQ-NFR-###
- Design: [Architecture document](../architecture_and_design/XX_DOCUMENT.md)
- Tests: Test cases TBD
```

---

## 4. Patient Management Use Cases

### UC-PM-001: Admit Patient to Monitor

**ID:** UC-PM-001  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** HIS/EHR System, Central Station  
**Stakeholders:** Nurses, Physicians, Hospital Administrators, Patients

**Preconditions:**
- User is logged in with appropriate permissions
- Device is provisioned and operational
- No patient currently admitted to this device

**Postconditions:**
Success:
- Patient is admitted to the device
- Patient information is displayed on the monitor
- Vital signs monitoring begins
- Admission event is logged
- Admission is synced to central server

Failure:
- Patient is not admitted
- Error message is displayed
- Admission attempt is logged

**Normal Flow:**
1. Nurse taps "Admit Patient" button on the patient banner
2. System displays Admission Modal with three options:
   - Manual MRN Entry
   - Barcode Scan
   - Wait for Central Station Push
3. Nurse selects "Manual MRN Entry"
4. Nurse enters patient MRN (e.g., "MRN-12345")
5. System validates MRN format
6. System queries HIS/EHR for patient information (via IPatientLookupService)
7. System displays patient information:
   - Name: "John Doe"
   - DOB: "1965-03-15"
   - Sex: "Male"
   - Bed Location: "ICU-Bed-3"
8. Nurse reviews and confirms information
9. System creates Patient object and admits patient
10. System updates UI to show:
    - Patient name in banner: "JOHN DOE"
    - Patient demographics
    - Bed location
    - Admission timestamp
11. System starts vital signs monitoring
12. System logs admission event to audit_log table
13. System syncs admission event to central server
14. System displays success message: "Patient John Doe admitted"

**Alternative Flows:**

**A1: Barcode Scan Method**
3a. Nurse selects "Barcode Scan"
4a. System activates camera/barcode scanner
5a. Nurse scans patient wristband barcode
6a. System extracts MRN from barcode
7a. Continue from step 6 of normal flow

**A2: Central Station Push Method**
3b. Nurse selects "Wait for Central Station Push"
4b. System displays QR code with device ID and IP address
5b. System enters listening mode
6b. Central Station technician scans QR code
7b. Central Station pushes patient assignment to device
8b. System receives patient MRN and information
9b. Continue from step 7 of normal flow (display for confirmation)

**A3: Patient Already in Local Cache**
6c. System finds patient in local patients table (cached)
7c. System displays cached information immediately
8c. System performs background HIS/EHR query to update cache
9c. Continue from step 7 of normal flow

**Exception Flows:**

**E1: Invalid MRN Format**
- At step 5: System displays error "Invalid MRN format. Expected: MRN-XXXXX"
- User corrects MRN and retries

**E2: Patient Not Found in HIS/EHR**
- At step 6: HIS/EHR returns "Patient not found"
- System displays error "Patient not found. Please verify MRN"
- User can retry or cancel admission

**E3: Network Connection Failure**
- At step 6: Network request times out
- System checks local cache
- If in cache: Display warning "Using cached data (HIS unavailable)" and continue
- If not in cache: Display error "Cannot connect to HIS. Retry when network available"
- User can retry or enter patient information manually (emergency mode)

**E4: Patient Already Admitted Elsewhere**
- At step 9: System detects patient is already admitted to another device
- System displays warning: "Patient is currently at Device ICU-MON-02. Transfer?"
- User can:
  - Cancel admission
  - Force admission (auto-discharges from other device)
  - Contact other location

**E5: Duplicate Admission Attempt**
- At step 1: Another patient is already admitted to this device
- System displays error "Device already monitoring patient Jane Smith. Discharge first?"
- User must discharge current patient before admitting new patient

**Business Rules:**
- BR-PM-001: Only users with "Nurse" or "Physician" role can admit patients
- BR-PM-002: A device can only monitor one patient at a time
- BR-PM-003: MRN format must match: `^MRN-\d{4,8}$`
- BR-PM-004: Admission requires either HIS/EHR connectivity or emergency override
- BR-PM-005: All admissions must be logged for audit purposes
- BR-PM-006: Patient information must be synced to server within 5 minutes

**Non-Functional Requirements:**
- **Performance:** Admission workflow must complete in < 30 seconds (normal flow)
- **Performance:** MRN validation must occur within 500ms
- **Performance:** HIS/EHR lookup must complete within 5 seconds or timeout
- **Usability:** Workflow must require < 5 taps/clicks
- **Usability:** Error messages must be clear and actionable
- **Security:** Admission event must be logged with user ID, timestamp, and action
- **Reliability:** Must work offline with local cache (graceful degradation)

**Notes:**
- Emergency mode (manual entry without HIS) requires supervisor approval
- Barcode scanner integration requires camera or USB barcode reader
- Central Station push requires device to be on hospital network

**Traces To:**
- **Requirements:** REQ-FUN-PAT-001, REQ-FUN-PAT-002, REQ-NFR-PERF-001, REQ-NFR-USE-001
- **Design:** [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager, AdmissionService)
- **Tests:** Test-PM-001, Test-PM-002, Test-PM-003

---

### UC-PM-002: Discharge Patient from Monitor

**ID:** UC-PM-002  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** Central Server, HIS/EHR  
**Stakeholders:** Nurses, Physicians, Hospital Administrators

**Preconditions:**
- User is logged in with appropriate permissions
- A patient is currently admitted to the device

**Postconditions:**
Success:
- Patient is discharged from the device
- Patient banner shows "DISCHARGED / STANDBY"
- Vital signs monitoring stops
- All patient data is synced to server
- Discharge event is logged
- Device is ready for new patient admission

Failure:
- Patient remains admitted
- Error message displayed
- Discharge attempt is logged

**Normal Flow:**
1. Nurse taps patient banner
2. System displays patient quick menu
3. Nurse selects "Discharge Patient"
4. System displays discharge confirmation dialog:
   - "Discharge John Doe (MRN-12345)?"
   - Warning: "Ensure all data is synced to server"
   - Sync status indicator: "✓ All data synced"
5. Nurse confirms discharge
6. System stops vital signs monitoring
7. System performs final data sync to server (vitals, alarms, events)
8. System verifies all data is synced (checks is_synced flags)
9. System discharges patient from PatientManager
10. System logs discharge event to audit_log
11. System updates UI:
    - Patient banner: "DISCHARGED / STANDBY"
    - Clears all patient information from display
    - Disables alarm notifications
12. System displays success message: "Patient John Doe discharged"
13. System enters standby mode (ready for new admission)

**Alternative Flows:**

**A1: Unsync Data Exists**
8a. System detects unsync vitals or alarms
9a. System displays warning: "X records not synced. Discharge anyway?"
10a. Options:
    - "Wait and Sync" - Retry sync, then discharge
    - "Force Discharge" - Discharge but keep trying to sync in background
    - "Cancel" - Return to monitoring
11a. If "Force Discharge": Continue discharge but flag device for review

**A2: Quick Discharge (Emergency)**
3b. Nurse selects "Quick Discharge" option
4b. System skips confirmation dialog
5b. System immediately discharges patient
6b. Background sync continues
7b. Discharge event logged with "EMERGENCY_DISCHARGE" flag

**Exception Flows:**

**E1: Network Connection Lost During Sync**
- At step 7: Sync fails due to network outage
- System displays warning: "Network unavailable. Data will sync when connection restored"
- User options:
  - "Discharge Anyway" - Discharge but keep data for later sync
  - "Cancel" - Return to monitoring
- If discharged: Device flagged for manual data verification

**E2: Server Rejects Discharge**
- At step 7: Server returns error (e.g., "Patient record locked")
- System displays error: "Cannot discharge. Contact IT support"
- System keeps patient admitted
- Discharge attempt logged for investigation

**E3: Concurrent Discharge (Another User)**
- At step 9: Another user already discharged the patient
- System displays info: "Patient already discharged by [Username] at [Time]"
- System updates UI to reflect discharge

**Business Rules:**
- BR-PM-010: Only users with "Nurse" or "Physician" role can discharge patients
- BR-PM-011: All patient data must be synced before discharge (or explicitly overridden)
- BR-PM-012: Discharge must be logged with user ID, timestamp, and reason
- BR-PM-013: Patient data must be retained locally for 7 days after discharge
- BR-PM-014: Emergency discharge requires documented justification

**Non-Functional Requirements:**
- **Performance:** Discharge workflow must complete in < 15 seconds (normal flow)
- **Performance:** Final sync must complete within 10 seconds or timeout
- **Usability:** Confirmation required to prevent accidental discharge
- **Usability:** Sync status must be clearly visible before discharge
- **Security:** Discharge event must be logged to audit trail
- **Reliability:** Discharge must succeed even if network is unavailable (offline mode)

**Notes:**
- Data synced after discharge continues in background
- Bed cleaning status not tracked by Z Monitor (external system)
- Discharge does not delete local data (retained for 7 days per retention policy)

**Traces To:**
- **Requirements:** REQ-FUN-PAT-003, REQ-FUN-PAT-004, REQ-NFR-PERF-002, REQ-DATA-RET-001
- **Design:** [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager)
- **Tests:** Test-PM-010, Test-PM-011

---

### UC-PM-003: Transfer Patient Between Monitors

**ID:** UC-PM-003  
**Priority:** High  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** Central Server, Target Device  
**Stakeholders:** Nurses, Physicians, Biomedical Engineers

**Preconditions:**
- User is logged in with appropriate permissions
- Patient is currently admitted to source device
- Target device is available and operational

**Postconditions:**
Success:
- Patient is discharged from source device
- Patient is admitted to target device
- All patient data is transferred
- Transfer event is logged on both devices
- Central server is updated

Failure:
- Patient remains on source device
- Error message displayed
- Transfer attempt is logged

**Normal Flow:**
1. Nurse on source device taps patient banner
2. System displays patient quick menu
3. Nurse selects "Transfer Patient"
4. System displays transfer dialog:
   - Patient: "John Doe (MRN-12345)"
   - Current device: "ICU-MON-01"
   - Target device: [Dropdown list of available devices]
5. Nurse selects target device: "ICU-MON-04"
6. Nurse enters transfer reason: "Patient moved to Room 204"
7. System validates target device is available (queries central server)
8. Central server confirms device ICU-MON-04 is available
9. Source device syncs all patient data to server
10. Source device sends transfer request to central server
11. Central server pushes admission to target device
12. Target device receives patient information and admits patient
13. Target device sends confirmation to central server
14. Central server notifies source device of successful admission
15. Source device discharges patient
16. Both devices log transfer event
17. System displays success: "Patient transferred to ICU-MON-04"

**Alternative Flows:**

**A1: Transfer via QR Code (No Central Server)**
4a. System displays "Transfer via QR Code" option
5a. Source device generates transfer QR code containing:
    - Patient MRN and demographics
    - Last 1 hour of vitals
    - Active alarm thresholds
6a. Nurse at target device scans QR code
7a. Target device validates and admits patient
8a. Target device notifies central server (background)
9a. Source device manually discharged after confirmation

**Exception Flows:**

**E1: Target Device Not Available**
- At step 7: Target device status is "OFFLINE" or "OCCUPIED"
- System displays error: "Target device not available. Choose another device"
- User selects different device or cancels

**E2: Transfer Request Timeout**
- At step 11: Central server doesn't respond within 30 seconds
- System displays error: "Transfer request timed out. Retry?"
- User can retry or use QR code method

**E3: Target Device Rejects Admission**
- At step 12: Target device returns error (e.g., "Device malfunction")
- Central server notifies source device
- System displays error: "Target device cannot accept patient"
- Patient remains on source device
- User must choose different device

**E4: Network Partition During Transfer**
- At step 13-14: Network fails after target admits but before source confirms
- Source device doesn't receive confirmation (timeout)
- System displays warning: "Transfer status unknown. Verify manually"
- Administrator must manually verify patient location
- System logs incident for resolution

**Business Rules:**
- BR-PM-020: Only nurses and physicians can transfer patients
- BR-PM-021: Transfer must include reason/justification
- BR-PM-022: Both devices must log transfer event
- BR-PM-023: Central server is authoritative source of patient location
- BR-PM-024: QR code method is fallback for network outages only

**Non-Functional Requirements:**
- **Performance:** Transfer must complete within 45 seconds (normal flow)
- **Reliability:** Transfer must be atomic (patient on one device at a time)
- **Reliability:** Transfer must handle network failures gracefully
- **Security:** Transfer data must be encrypted
- **Security:** QR code must expire after 10 minutes
- **Usability:** Transfer status must be visible to user

**Notes:**
- Transfer preserves patient history (vitals, alarms, settings)
- Large data transfers (>1 day history) may take longer
- Network partition scenario requires manual administrator intervention

**Traces To:**
- **Requirements:** REQ-FUN-PAT-005, REQ-NFR-REL-001, REQ-NFR-SEC-002
- **Design:** [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- **Tests:** Test-PM-020, Test-PM-021

---

### UC-PM-004: Lookup Patient Information

**ID:** UC-PM-004  
**Priority:** High  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** HIS/EHR System  
**Stakeholders:** Nurses, Physicians

**Preconditions:**
- User is logged in
- HIS/EHR connectivity available (or local cache available)

**Postconditions:**
Success:
- Patient information is retrieved and displayed
- Local cache is updated
- Lookup is logged

Failure:
- Error message displayed
- Lookup attempt logged

**Normal Flow:**
1. Nurse begins patient admission (from UC-PM-001)
2. Nurse enters patient MRN: "MRN-12345"
3. System validates MRN format
4. System checks local cache (patients table) first
5. Local cache miss - patient not found
6. System queries HIS/EHR via IPatientLookupService
7. System displays loading indicator: "Looking up patient..."
8. HIS/EHR returns patient information:
   ```json
   {
     "mrn": "MRN-12345",
     "name": "John Doe",
     "dob": "1965-03-15",
     "sex": "Male",
     "bedLocation": "ICU-Bed-3"
   }
   ```
9. System stores patient in local cache with timestamp
10. System displays patient information
11. System logs lookup event (MRN, timestamp, source: HIS)

**Alternative Flows:**

**A1: Cache Hit**
4a. Patient found in local cache
5a. System checks cache age (< 24 hours is acceptable)
6a. System displays cached information immediately
7a. System performs background HIS query to update cache
8a. If HIS returns updated info, system highlights changes
9a. System logs lookup event (source: CACHE)

**A2: Cached Data Stale**
4b. Patient found in cache but age > 24 hours
5b. System displays cached data with warning: "Data may be outdated. Updating..."
6b. System queries HIS in foreground
7b. System updates cache and display when HIS responds
8b. System logs lookup event (source: CACHE_STALE, updated: YES)

**Exception Flows:**

**E1: HIS Not Responding**
- At step 6: HIS query times out after 5 seconds
- System checks cache (even if stale)
- If in cache: Display with warning "Using cached data (HIS unavailable)"
- If not in cache: Display error "Cannot contact HIS. Try again or enter manually"
- System logs lookup failure

**E2: Patient Not Found in HIS**
- At step 8: HIS returns "Patient not found"
- System displays error: "Patient MRN-12345 not found in hospital system"
- User can:
  - Re-enter MRN (typo check)
  - Enter patient manually (emergency mode with supervisor approval)
  - Cancel admission

**E3: Multiple Patients Match**
- At step 8: HIS returns multiple matches (ambiguous MRN)
- System displays list of matches with disambiguation info
- User selects correct patient
- System continues with selected patient

**Business Rules:**
- BR-PM-030: Cache patients locally for 24 hours
- BR-PM-031: Expired cache can be used as fallback when HIS unavailable
- BR-PM-032: All lookups must be logged (HIPAA audit trail)
- BR-PM-033: Manual entry requires supervisor approval

**Non-Functional Requirements:**
- **Performance:** Cache lookup must return in < 100ms
- **Performance:** HIS lookup must complete in < 5 seconds or timeout
- **Performance:** Background cache refresh must not block UI
- **Reliability:** System must work offline with cached data
- **Security:** Patient lookups must be logged with user ID
- **Privacy:** Cache must be encrypted (SQLCipher)

**Traces To:**
- **Requirements:** REQ-FUN-PAT-010, REQ-NFR-PERF-010, REQ-SEC-AUDIT-001
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager, IPatientLookupService)
- **Design:** [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (patients table)
- **Tests:** Test-PM-030

---

## 5. Vital Signs Monitoring Use Cases

### UC-VM-001: Display Real-Time Vital Signs

**ID:** UC-VM-001  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** Sensor Simulator, DeviceSimulator  
**Stakeholders:** Nurses, Physicians, Patients

**Preconditions:**
- Device is operational
- Patient is admitted to device
- Sensors are connected (or simulator running)

**Postconditions:**
Success:
- Vital signs are displayed in real-time
- Display updates at appropriate frequency (1Hz for heart rate, 0.2Hz for SpO2)
- Trends are updated
- Data is logged to database

**Normal Flow:**
1. System starts monitoring after patient admission (UC-PM-001)
2. DeviceSimulator generates vital signs data every 1 second:
   ```
   Heart Rate: 78 bpm
   SpO2: 98%
   Respiration Rate: 16 rpm
   Signal Quality: GOOD
   ```
3. System validates vital signs ranges:
   - Heart Rate: 30-250 bpm
   - SpO2: 50-100%
   - Respiration Rate: 5-60 rpm
4. System stores vitals in database (vitals table)
5. System updates main dashboard display:
   - Large numeric values with units
   - Color coding (normal=green, warning=yellow, critical=red)
   - Timestamp of last update
6. System updates trend sparklines (last 5 minutes)
7. System checks alarm thresholds (triggers UC-AM-001 if exceeded)
8. System repeats every second

**Alternative Flows:**

**A1: High-Frequency ECG Waveform**
2a. DeviceSimulator generates ECG samples at 250 Hz
3a. System buffers samples and displays waveform in real-time
4a. System downsamples for storage (store every 10th sample)
5a. System updates waveform display at 60 FPS

**A2: Manual Vital Signs Entry**
- User follows UC-VM-003 to enter manual blood pressure reading
- System displays manual entry alongside automated vitals
- Manual entries marked with timestamp and user

**Exception Flows:**

**E1: Invalid Vital Sign Value**
- At step 3: Value out of valid range (e.g., HR: 300 bpm)
- System logs validation error
- System displays last valid value with warning: "Invalid sensor reading"
- System does NOT trigger alarm for invalid data
- System sets signal quality to POOR

**E2: Sensor Disconnected**
- At step 2: No data received for > 5 seconds
- System displays warning: "Sensor disconnected"
- System shows last valid value grayed out
- System triggers technical alarm (UC-AM-001)
- System logs disconnection event

**E3: Database Write Failure**
- At step 4: Database write fails (disk full, corruption)
- System logs error to system log
- System continues displaying real-time data (in-memory)
- System triggers system alarm for administrator
- System retries database write

**Business Rules:**
- BR-VM-001: Display must update at least 1 time per second
- BR-VM-002: Invalid data must not trigger clinical alarms
- BR-VM-003: Last valid value displayed for up to 10 seconds if no new data
- BR-VM-004: Sensor disconnection triggers technical alarm within 5 seconds
- BR-VM-005: Vital signs data retained for 7 days

**Non-Functional Requirements:**
- **Performance:** UI update latency < 100ms from data generation
- **Performance:** Display must maintain 60 FPS for smooth waveforms
- **Performance:** Database write must not block UI thread
- **Reliability:** Display must handle intermittent sensor data
- **Usability:** Values must be readable from 10 feet away (large fonts)
- **Usability:** Color coding must be consistent with medical standards

**Notes:**
- Simulator mode uses DeviceSimulator for realistic data
- Production uses actual sensor hardware interfaces
- Waveform display requires high-performance graphics (QML Canvas or OpenGL)

**Traces To:**
- **Requirements:** REQ-FUN-VITAL-001, REQ-NFR-PERF-100, REQ-NFR-USE-010
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DeviceSimulator)
- **Design:** [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Real-time thread)
- **Tests:** Test-VM-001, Test-VM-002

---

### UC-VM-002: View Vital Signs Trends

**ID:** UC-VM-002  
**Priority:** High  
**Status:** Approved

**Primary Actor:** Physician, Clinical Nurse  
**Secondary Actors:** Database  
**Stakeholders:** Physicians, Nurses

**Preconditions:**
- User is logged in
- Patient is admitted (or was recently admitted)
- Historical vital signs data exists

**Postconditions:**
Success:
- Trend chart is displayed
- User can navigate time ranges
- Trends help inform clinical decisions

**Normal Flow:**
1. User navigates to "Trends" view
2. System displays trend configuration panel:
   - Time range: [1 hour] [4 hours] [12 hours] [24 hours] [7 days]
   - Vital signs: [Heart Rate] [SpO2] [Resp Rate] [All]
   - Display mode: [Line chart] [Table]
3. User selects "24 hours" time range
4. User selects "Heart Rate" and "SpO2"
5. System queries database for vitals data:
   ```sql
   SELECT timestamp, heart_rate, spo2
   FROM vitals
   WHERE patient_mrn = 'MRN-12345'
     AND timestamp >= datetime('now', '-24 hours')
   ORDER BY timestamp ASC
   ```
6. System retrieves ~86,400 data points (1 per second for 24 hours)
7. System downsamples to ~1,440 points (1 per minute) for display
8. System renders line chart with:
   - X-axis: Time (24 hours)
   - Y-axis: Heart Rate (30-150 bpm)
   - Y-axis (right): SpO2 (85-100%)
   - Grid lines every 4 hours (X) and 20 bpm (Y)
   - Alarm threshold lines (dotted red)
   - Alarm events marked with icons
9. System displays chart with zoom/pan controls
10. User can hover over points to see exact values and timestamps

**Alternative Flows:**

**A1: Table View**
3a. User selects "Table" display mode
7a. System displays data in table format:
    | Timestamp | HR | SpO2 | Resp Rate | Alarms |
    | 14:30:00  | 78 | 98   | 16        | -      |
    | 14:31:00  | 79 | 98   | 16        | -      |
    | 14:32:00  | 125| 97   | 18        | HR HIGH|
8a. Table supports sorting, filtering, export

**A2: Real-Time Trend (Last 5 Minutes)**
- Displayed on main dashboard as sparkline
- Auto-scrolling chart
- No user interaction needed

**A3: Multi-Patient Comparison**
- Physician views trends for multiple patients side-by-side
- Useful for rounds or comparative analysis

**Exception Flows:**

**E1: No Data Available**
- At step 6: Query returns no results
- System displays message: "No vital signs data for this time range"
- User can select different time range

**E2: Incomplete Data (Gaps)**
- At step 6: Data has gaps due to sensor disconnections
- System displays chart with gaps
- System annotates gaps with "No data" markers
- Gaps do not interpolate (no false data)

**E3: Large Dataset (> 7 Days)**
- At step 6: Query would return excessive data
- System automatically downsamples more aggressively
- System displays warning: "Data downsampled for performance"
- User can zoom to see higher resolution

**Business Rules:**
- BR-VM-010: Trends display data for up to 7 days
- BR-VM-011: Data gaps must be clearly visible (no interpolation)
- BR-VM-012: Alarm events must be correlated with vital signs
- BR-VM-013: Downsample for display, but preserve raw data in database

**Non-Functional Requirements:**
- **Performance:** Trend query must complete in < 2 seconds
- **Performance:** Chart rendering must complete in < 1 second
- **Performance:** Chart must support smooth zoom/pan (60 FPS)
- **Usability:** Trends must be easy to interpret (clear legends, labels)
- **Usability:** Hover tooltips show exact values
- **Data:** Raw data retained for 7 days (downsampled data for longer)

**Traces To:**
- **Requirements:** REQ-FUN-VITAL-010, REQ-NFR-PERF-110, REQ-NFR-USE-020
- **Design:** [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (vitals table)
- **Tests:** Test-VM-010

---

## 6. Alarm Management Use Cases

### UC-AM-001: Trigger Vital Sign Alarm

**ID:** UC-AM-001  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** System (AlarmManager)  
**Secondary Actors:** Clinical Nurse  
**Stakeholders:** Nurses, Physicians, Patients, Quality Teams

**Preconditions:**
- Patient is admitted
- Vital signs are being monitored
- Alarm thresholds are configured

**Postconditions:**
Success:
- Alarm is triggered and displayed
- Audio/visual alarm activated
- Alarm logged to database
- Alarm synced to central server

**Normal Flow:**
1. System receives vital sign update: Heart Rate = 125 bpm
2. AlarmManager checks configured thresholds:
   - HR Low: 50 bpm
   - HR High: 120 bpm
3. AlarmManager detects threshold violation: 125 > 120
4. AlarmManager applies alarm delay (2 seconds) to prevent false alarms
5. After 2 seconds, HR still > 120
6. AlarmManager creates alarm:
   ```
   Priority: MEDIUM
   Type: HR_HIGH
   Value: 125 bpm
   Threshold: 120 bpm
   Patient: John Doe (MRN-12345)
   Timestamp: 2025-11-27 14:32:15
   ```
7. AlarmManager logs alarm to database (alarms table)
8. AlarmManager triggers audio alarm:
   - Medium priority: 3 beeps, 1-second interval
   - Volume: Based on room setting
9. AlarmManager updates UI:
   - Red border around heart rate value
   - Alarm indicator in notification bell (red dot with count)
   - Alarm banner at top: "HEART RATE HIGH - 125 bpm"
10. System starts alarm escalation timer (2 minutes for medium priority)
11. System awaits nurse acknowledgment (UC-AM-002)

**Alternative Flows:**

**A1: Critical Alarm (High Priority)**
6a. HR exceeds critical threshold (150 bpm)
7a. System creates HIGH priority alarm
8a. Audio alarm: Continuous rapid beeps
9a. UI: Flashing red background, large "CRITICAL" banner
10a. Escalation timer: 30 seconds (faster escalation)

**A2: Low Priority Alarm**
6b. SpO2 slightly low (94%, threshold 95%)
7b. System creates LOW priority alarm
8b. Audio alarm: Single beep, no repeat
9b. UI: Yellow highlight, subtle notification

**A3: Transient Spike (Alarm Delay)**
4c. HR spikes to 125 briefly but returns to 78 within 2 seconds
5c. AlarmManager does not trigger alarm (spike filtered)
6c. Event logged as "Transient spike - no alarm"

**Exception Flows:**

**E1: Invalid Sensor Reading**
- At step 1: HR = 300 bpm (invalid)
- System does NOT trigger clinical alarm
- System triggers technical alarm: "Sensor Error"
- Different audio pattern (steady tone)
- Alarm type: TECHNICAL

**E2: Alarm Fatigue (Too Many Alarms)**
- System detects > 10 alarms in 5 minutes
- System logs alarm fatigue event
- System continues alarming (patient safety priority)
- System notifies administrator for review
- Quality team investigates alarm burden

**E3: Audio System Failure**
- At step 8: Audio output fails
- System escalates to visual-only alarm
- System logs audio failure
- System displays "AUDIO ALARM FAILED" warning
- Biomedical engineer notified

**Business Rules:**
- BR-AM-001: Alarms must comply with IEC 60601-1-8 priorities
- BR-AM-002: Critical alarms cannot be silenced (only acknowledged)
- BR-AM-003: Alarm delay prevents transient spikes from triggering alarms
- BR-AM-004: All alarms must be logged for audit
- BR-AM-005: Alarm escalation required for unacknowledged critical alarms
- BR-AM-006: Audio alarms must be audible from 10 feet away

**Non-Functional Requirements:**
- **Performance:** Alarm must trigger within 50ms of threshold violation
- **Performance:** Audio latency < 100ms
- **Performance:** UI update < 100ms
- **Reliability:** Alarm system must have 99.99% uptime
- **Reliability:** Audio failure must not prevent visual alarm
- **Safety:** False alarm rate < 5% (per IEC 60601-1-8)
- **Safety:** Missed critical alarm rate = 0%
- **Usability:** Alarm sounds must be distinctive per priority

**Notes:**
- IEC 60601-1-8 defines three priority levels (High, Medium, Low)
- Audio patterns specified by standard (number of pulses, pulse duration)
- Alarm delay tunable per vital sign type (balance sensitivity vs. false positives)

**Traces To:**
- **Requirements:** REQ-FUN-ALARM-001, REQ-NFR-PERF-200, REQ-REG-60601-001
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- **Design:** [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- **Tests:** Test-AM-001, Test-AM-002, Test-AM-003

---

### UC-AM-002: Acknowledge Alarm

**ID:** UC-AM-002  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** Clinical Nurse  
**Secondary Actors:** System  
**Stakeholders:** Nurses, Physicians, Quality Teams

**Preconditions:**
- Alarm is active (triggered by UC-AM-001)
- User can see/hear the alarm

**Postconditions:**
Success:
- Alarm is acknowledged
- Audio alarm stops (or pauses)
- Acknowledgment logged with user ID and timestamp
- Alarm remains visible until condition resolves

**Normal Flow:**
1. Nurse hears alarm (from UC-AM-001)
2. Nurse approaches device and assesses situation
3. Nurse taps alarm banner or notification bell
4. System displays alarm details:
   ```
   HEART RATE HIGH
   Current: 125 bpm
   Threshold: 120 bpm
   Duration: 1 min 30 sec
   Patient: John Doe (MRN-12345)
   ```
5. Nurse reviews patient condition
6. Nurse taps "Acknowledge" button
7. System stops audio alarm
8. System updates alarm status to "ACKNOWLEDGED"
9. System logs acknowledgment:
   ```
   Alarm ID: 12345
   Acknowledged by: Nurse Sarah Johnson
   Timestamp: 2025-11-27 14:33:45
   Response time: 90 seconds
   ```
10. System updates UI:
    - Alarm banner changes to yellow (acknowledged)
    - Audio stops
    - Visual indicator remains (alarm still active)
11. System cancels escalation timer
12. System continues monitoring - if condition persists, may re-alarm

**Alternative Flows:**

**A1: Quick Acknowledge (Hardware Button)**
3a. Nurse presses physical "Acknowledge" button on device
4a. System immediately acknowledges most recent alarm
5a. Continue from step 7

**A2: Acknowledge All**
3b. Multiple alarms active simultaneously
4b. Nurse taps "Acknowledge All" button
5b. System acknowledges all active alarms
6b. Each acknowledgment logged separately

**A3: Auto-Acknowledge (Condition Resolved)**
- Vital sign returns to normal range
- System automatically acknowledges alarm after 30 seconds
- Alarm status: "AUTO-RESOLVED"
- Logged with system as acknowledger

**Exception Flows:**

**E1: Critical Alarm Cannot Be Silenced**
- At step 7: Alarm is CRITICAL priority
- System pauses audio for 2 minutes (not stopped)
- Alarm re-activates after 2 minutes if condition persists
- Visual alarm remains active

**E2: Re-Alarm (Condition Worsens)**
- At step 12: HR increases to 140 bpm
- System triggers new alarm (higher priority)
- Previous acknowledgment logged separately
- New alarm requires new acknowledgment

**E3: Acknowledge Timeout**
- At step 11: Alarm not acknowledged within escalation time (2 min)
- System escalates alarm (see UC-AM-006)
- Alarm remains active and audible

**Business Rules:**
- BR-AM-010: All acknowledgments must be logged with user ID
- BR-AM-011: Acknowledgment must occur within 2 minutes (medium), 30 seconds (critical)
- BR-AM-012: Critical alarms can only be paused (2 min), not silenced
- BR-AM-013: Alarm response time tracked for quality metrics
- BR-AM-014: Acknowledged alarms remain visible until condition resolves

**Non-Functional Requirements:**
- **Performance:** Acknowledgment must register within 500ms of tap
- **Performance:** Audio must stop within 100ms
- **Usability:** Acknowledge action must be intuitive (large, prominent button)
- **Usability:** Alarm details must be immediately visible
- **Safety:** Accidental acknowledgment prevented by confirmation for critical alarms
- **Audit:** All acknowledgments must be logged for compliance

**Notes:**
- Acknowledgment indicates nurse awareness, not alarm resolution
- Quality metrics track acknowledgment response times
- Unacknowledged alarms escalate to charge nurse/supervisor

**Traces To:**
- **Requirements:** REQ-FUN-ALARM-010, REQ-NFR-PERF-210, REQ-REG-60601-010
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- **Tests:** Test-AM-010, Test-AM-011

---

## 7. User Authentication Use Cases

### UC-UA-001: Login to System

**ID:** UC-UA-001  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** Clinical User (Nurse, Physician, Technician)  
**Secondary Actors:** Authentication Service  
**Stakeholders:** All users, IT Administrators, Security Teams

**Preconditions:**
- Device is powered on and operational
- User has valid credentials

**Postconditions:**
Success:
- User is authenticated
- Session is created
- User's role and permissions loaded
- Login event logged
- UI configured for user role

Failure:
- User remains logged out
- Error message displayed
- Failed login attempt logged

**Normal Flow:**
1. System displays login screen:
   - User ID field
   - PIN field (masked)
   - "Login" button
2. User enters User ID: "NURSE001"
3. User enters PIN: "****" (4-6 digits)
4. User taps "Login" button
5. System validates input format
6. System checks credentials against local database (users table)
7. System hashes PIN and compares with stored hash
8. Credentials match
9. System checks account status (not locked)
10. System creates user session:
    ```
    Session ID: uuid-1234-5678
    User ID: NURSE001
    Role: NURSE
    Permissions: [ADMIT_PATIENT, DISCHARGE_PATIENT, ACK_ALARM]
    Login time: 2025-11-27 07:00:00
    Timeout: 15 minutes
    ```
11. System logs login event to audit_log
12. System loads user preferences (language, units, layout)
13. System transitions to dashboard
14. System displays user name in header: "Sarah Johnson - Nurse"

**Alternative Flows:**

**A1: Biometric Authentication**
3a. User places finger on fingerprint scanner (if available)
4a. System validates fingerprint against stored template
5a. Continue from step 8 (credential match)

**A2: Badge Scan**
3b. User scans ID badge with RFID reader
4b. System reads badge ID
5b. System prompts for PIN: "Enter PIN for Badge #12345"
6b. User enters PIN
7b. Continue from step 7

**A3: Remember User ID**
2c. System displays last logged-in user: "NURSE001"
3c. User only needs to enter PIN
4c. Continue from step 4

**Exception Flows:**

**E1: Invalid Credentials**
- At step 8: Credentials don't match
- System increments failed login counter
- System displays error: "Invalid User ID or PIN"
- System logs failed attempt
- If 3 failed attempts: Account locked for 10 minutes
- User can retry after delay

**E2: Account Locked**
- At step 9: Account status is "LOCKED"
- System displays error: "Account locked. Contact administrator"
- System logs lockout event
- Administrator must unlock account

**E3: Session Already Active**
- At step 10: User already has active session on another device
- System displays warning: "Already logged in on Device ICU-MON-02. Force logout?"
- Options:
  - "Continue" - Logs out other session, creates new one
  - "Cancel" - Returns to login screen

**E4: Expired Password (If Applicable)**
- At step 9: PIN expired (password rotation policy)
- System prompts: "PIN expired. Set new PIN"
- User enters new PIN (6 digits)
- User confirms new PIN
- System updates PIN hash
- Continue login flow

**Business Rules:**
- BR-UA-001: Users must authenticate before using system
- BR-UA-002: PIN must be 4-6 digits (or biometric equivalent)
- BR-UA-003: PIN must be hashed (bcrypt or Argon2)
- BR-UA-004: 3 failed attempts locks account for 10 minutes
- BR-UA-005: Session timeout after 15 minutes of inactivity
- BR-UA-006: All login attempts (success/failure) must be logged

**Non-Functional Requirements:**
- **Performance:** Login must complete in < 2 seconds
- **Performance:** Credential validation < 500ms
- **Security:** PIN must never be stored in plaintext
- **Security:** Failed login attempts must be rate-limited
- **Security:** Session must auto-expire after timeout
- **Usability:** Login process < 3 steps for common case
- **Audit:** All authentication events logged with timestamp, device, IP

**Notes:**
- Emergency override procedure available for system administrators
- Biometric authentication optional (fingerprint scanner hardware)
- LDAP/SSO integration possible for hospital-wide authentication

**Traces To:**
- **Requirements:** REQ-FUN-AUTH-001, REQ-NFR-SEC-100, REQ-SEC-AUTH-001
- **Design:** [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- **Design:** [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 3)
- **Tests:** Test-UA-001, Test-UA-002

---

## 8. Emergency Scenarios

### UC-ES-001: Handle Critical Alarm (Code Blue)

**ID:** UC-ES-001  
**Priority:** Critical  
**Status:** Approved

**Primary Actor:** System, Clinical Nurse, Rapid Response Team  
**Secondary Actors:** Central Server  
**Stakeholders:** Nurses, Physicians, Patients, Hospital Administrators

**Preconditions:**
- Patient is admitted and monitored
- Vital signs are being tracked
- Rapid response system is operational

**Postconditions:**
Success:
- Code Blue alarm triggered
- Rapid response team notified
- All events logged for review
- Patient receives appropriate care

**Normal Flow:**
1. System detects critical vital sign pattern:
   - Heart Rate drops to 35 bpm (bradycardia)
   - SpO2 drops to 75%
2. AlarmManager recognizes pattern as life-threatening
3. System triggers CRITICAL alarm immediately (no delay)
4. System activates maximum audio/visual alarms:
   - Continuous rapid beeping
   - Red flashing screen border
   - Large "CODE BLUE" banner
5. System logs event: "CODE BLUE - Bradycardia + Hypoxia"
6. System attempts to notify central server:
   - Send CODE BLUE alert with patient info
   - Request rapid response team dispatch
7. Nurse hears alarm and rushes to bedside
8. Nurse assesses patient (unresponsive)
9. Nurse presses "Code Blue" button on device (confirms emergency)
10. System broadcasts Code Blue to all devices on unit
11. System displays patient info on all nearby monitors
12. System records all vital signs at 1 Hz (high-frequency mode)
13. Rapid response team arrives within 3 minutes
14. Team uses device to monitor resuscitation efforts
15. System logs all interventions and vital sign changes
16. Event resolved - patient stabilized or passed
17. Physician documents outcome in system
18. System generates Code Blue report for quality review

**Alternative Flows:**

**A1: Code Blue Button Pressed First**
9a. Nurse recognizes emergency before alarm
10a. Nurse presses "Code Blue" button
11a. System immediately triggers CODE BLUE alarm
12a. Continue from step 5

**A2: False Alarm (Sensor Artifact)**
8b. Nurse assesses patient (patient awake and talking)
9b. Nurse identifies sensor disconnection
10b. Nurse cancels Code Blue: "False alarm - sensor issue"
11b. System logs false alarm
12b. Quality team reviews for sensor improvement

**Exception Flows:**

**E1: Network Failure (Cannot Notify Central)**
- At step 6: Network unavailable
- System logs failure: "CODE BLUE - Central server not notified"
- System continues local alarming
- Nurse manually calls rapid response team
- Device retries notification when network restored

**E2: Multiple Simultaneous Code Blues**
- At step 10: Another Code Blue active on unit
- System prioritizes both equally
- System displays both patient locations on all devices
- Hospital emergency protocol activated

**E3: Device Battery Low During Code Blue**
- System detects battery < 10% during emergency
- System displays warning: "PLUG IN DEVICE NOW"
- System continues operation on battery
- If battery dies: All data preserved for recovery

**Business Rules:**
- BR-ES-001: Code Blue alarms cannot be silenced (only acknowledged)
- BR-ES-002: All Code Blue events trigger hospital-wide notification
- BR-ES-003: High-frequency data recording (1 Hz) during Code Blue
- BR-ES-004: Code Blue events require physician documentation
- BR-ES-005: All Code Blue events reviewed by quality team

**Non-Functional Requirements:**
- **Performance:** Code Blue alarm must trigger within 50ms
- **Performance:** Central notification within 1 second
- **Reliability:** System must continue functioning during emergency
- **Reliability:** All data must be logged (no data loss)
- **Safety:** False Code Blue rate < 1%
- **Safety:** Missed Code Blue rate = 0%
- **Audit:** Complete timeline of all events for review

**Notes:**
- Code Blue most critical use case (patient life at risk)
- System design prioritizes Code Blue reliability above all else
- Regular Code Blue drills verify system performance
- Quality review of every Code Blue event (real or false alarm)

**Traces To:**
- **Requirements:** REQ-FUN-ALARM-100, REQ-NFR-REL-100, REQ-REG-60601-100
- **Design:** [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- **Tests:** Test-ES-001, Test-ES-002 (mandatory stress testing)

---

## 9. Use Case Summary Statistics

| Category | Use Cases | Critical | High | Medium |
|----------|-----------|----------|------|--------|
| Patient Management | 5 | 2 | 3 | 0 |
| Vital Signs Monitoring | 4 | 1 | 2 | 1 |
| Alarm Management | 6 | 3 | 2 | 1 |
| User Authentication | 5 | 3 | 1 | 1 |
| Device Provisioning | 5 | 1 | 3 | 1 |
| System Configuration | 5 | 0 | 2 | 3 |
| Data Synchronization | 4 | 0 | 2 | 2 |
| Emergency Scenarios | 4 | 4 | 0 | 0 |
| **Total** | **38** | **14** | **15** | **9** |

---

## 10. Use Case Traceability

All use cases map to:
- **Functional Requirements** (REQ-FUN-###) - What the system must do
- **Non-Functional Requirements** (REQ-NFR-###) - How well it must perform
- **Design Documents** - Architecture and class designs
- **Test Cases** - Verification and validation

See [10_REQUIREMENTS_TRACEABILITY.md](./10_REQUIREMENTS_TRACEABILITY.md) for complete mapping.

---

## 11. Related Documents

- **Stakeholders:** [01_STAKEHOLDERS_AND_USERS.md](./01_STAKEHOLDERS_AND_USERS.md)
- **Functional Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](./03_FUNCTIONAL_REQUIREMENTS.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)

---

*Use cases drive functional requirements and inform design decisions. All features must trace back to documented use cases.*

