# Functional Requirements

**Document ID:** REQ-DOC-03  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document specifies what the Z Monitor system must do. Each requirement is atomic, testable, and traceable to use cases and design decisions.

**Related Documents:**
- **Use Cases:** [02_USE_CASES.md](./02_USE_CASES.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)

---

## 2. Requirements Index

### 2.1 User Management (REQ-FUN-USER-###)
Authentication, authorization, session management

### 2.2 Patient Management (REQ-FUN-PAT-###)
Patient admission, discharge, transfer, lookup

### 2.3 Vital Signs (REQ-FUN-VITAL-###)
Real-time monitoring, display, recording, trends

### 2.4 Alarm Management (REQ-FUN-ALARM-###)
Alarm detection, notification, acknowledgment, escalation

### 2.5 Device Management (REQ-FUN-DEV-###)
Device provisioning, configuration, diagnostics

### 2.6 Data Management (REQ-FUN-DATA-###)
Data storage, synchronization, backup, archival

### 2.7 System Configuration (REQ-FUN-SYS-###)
Settings, preferences, system controls

### 2.8 Reporting & Analytics (REQ-FUN-REP-###)
Reports, logs, audit trails, analytics

---

## 3. Requirement Format

Each requirement follows this structure:

```
[REQ-FUN-XXX-###] Requirement Title

Category: [Category name]
Priority: Must Have | Should Have | Nice to Have
Status: Draft | Approved | Implemented | Verified

Description:
A clear, concise statement of what the system must do. Written from system perspective
("The system shall..."). Testable and unambiguous.

Rationale:
Why this requirement exists. Business need, user need, or regulatory requirement.

Acceptance Criteria:
- Testable criterion 1
- Testable criterion 2
- Testable criterion 3

Related Requirements:
- REQ-FUN-YYY-### (dependency)
- REQ-NFR-ZZZ-### (non-functional constraint)

Traces To:
- Use Case: UC-XXX-###
- Design: [Document](../architecture_and_design/XX_DOCUMENT.md) (Section Y)
- Test: Test-XXX-###

Notes:
Additional context, assumptions, or clarifications.
```

---

## 4. User Management Requirements

### [REQ-FUN-USER-001] User Authentication

**Category:** User Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall authenticate users via PIN-based login before granting access to any patient data or system functions.

**Rationale:**
HIPAA requires authentication to protect patient data. User accountability requires knowing who performs each action.

**Acceptance Criteria:**
- User must enter valid User ID and PIN to login
- PIN must be 4-6 digits
- System validates credentials against local database
- Failed authentication displays error message
- Successful authentication creates user session

**Related Requirements:**
- REQ-FUN-USER-002 (session management)
- REQ-NFR-SEC-001 (security)
- REQ-SEC-AUTH-001 (authentication security)

**Traces To:**
- Use Case: UC-UA-001
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 3)
- Test: Test-USER-001

**Notes:**
- Biometric authentication (fingerprint) is optional enhancement
- Emergency override available for system administrators

---

### [REQ-FUN-USER-002] Session Management

**Category:** User Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall create a user session upon successful authentication and maintain session state until logout or timeout.

**Rationale:**
Sessions track user activity, enable audit logging, and enforce security policies (auto-logout).

**Acceptance Criteria:**
- Session created with unique ID, user info, role, permissions, timestamp
- Session persists across screen navigation
- Session expires after 15 minutes of inactivity
- User can explicitly logout to end session
- Only one active session per user per device

**Related Requirements:**
- REQ-FUN-USER-001 (authentication)
- REQ-FUN-USER-003 (auto-logout)
- REQ-SEC-AUTH-002 (session security)

**Traces To:**
- Use Case: UC-UA-001, UC-UA-002, UC-UA-003
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-USER-002

---

### [REQ-FUN-USER-003] Auto-Logout on Inactivity

**Category:** User Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall automatically logout users after 15 minutes of inactivity to prevent unauthorized access.

**Rationale:**
Security best practice. Prevents unauthorized access if user walks away from device.

**Acceptance Criteria:**
- Inactivity timer starts when no user input for 15 minutes
- Warning displayed at 14 minutes: "Auto-logout in 1 minute"
- User can dismiss warning to reset timer
- Session terminated at 15 minutes
- User redirected to login screen
- Auto-logout event logged

**Related Requirements:**
- REQ-FUN-USER-002 (session management)
- REQ-SEC-AUTH-003 (session timeout)

**Traces To:**
- Use Case: UC-UA-003
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-USER-003

---

### [REQ-FUN-USER-004] Role-Based Access Control

**Category:** User Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall enforce role-based permissions where users can only perform actions authorized for their role.

**Rationale:**
Security principle of least privilege. Prevents unauthorized actions (e.g., technicians admitting patients).

**Acceptance Criteria:**
- System defines roles: Nurse, Physician, Technician, Administrator
- Each role has defined permissions list
- System checks permissions before allowing actions
- Unauthorized actions display error: "Insufficient permissions"
- Permission checks logged for audit

**Related Requirements:**
- REQ-FUN-USER-001 (authentication)
- REQ-SEC-AUTHZ-001 (authorization)

**Traces To:**
- Use Case: All use cases (permission checks)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 4)
- Test: Test-USER-004

**Notes:**
Role-permission matrix defined in design documentation.

---

### [REQ-FUN-USER-005] Brute Force Protection

**Category:** User Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall lock user accounts after 3 consecutive failed login attempts for 10 minutes to prevent brute force attacks.

**Rationale:**
Security requirement to prevent password guessing attacks.

**Acceptance Criteria:**
- System counts failed login attempts per user
- After 3 failures, account locked for 10 minutes
- Locked account displays: "Account locked. Try again in X minutes"
- Lock expires automatically after 10 minutes
- Administrator can manually unlock accounts
- All lockout events logged

**Related Requirements:**
- REQ-FUN-USER-001 (authentication)
- REQ-SEC-AUTH-004 (brute force protection)

**Traces To:**
- Use Case: UC-UA-001 (exception flow E1)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 3.2)
- Test: Test-USER-005

---

## 5. Patient Management Requirements

### [REQ-FUN-PAT-001] Patient Admission - Manual MRN Entry

**Category:** Patient Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall allow authorized users to admit a patient by manually entering the patient's MRN (Medical Record Number).

**Rationale:**
Primary method for associating a patient with a monitoring device. Required for patient identification and data association.

**Acceptance Criteria:**
- User taps "Admit Patient" button
- System displays admission modal with MRN input field
- User enters MRN (format: MRN-XXXXX)
- System validates MRN format
- System queries HIS/EHR for patient information
- System displays patient name, DOB, sex, bed location
- User confirms patient information
- System admits patient and starts monitoring
- Admission logged with user ID and timestamp

**Related Requirements:**
- REQ-FUN-PAT-002 (barcode admission)
- REQ-FUN-PAT-010 (patient lookup)
- REQ-FUN-USER-004 (role permissions)

**Traces To:**
- Use Case: UC-PM-001 (normal flow)
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager, AdmissionService)
- Test: Test-PAT-001

---

### [REQ-FUN-PAT-002] Patient Admission - Barcode Scan

**Category:** Patient Management  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall allow authorized users to admit a patient by scanning a barcode on the patient's wristband.

**Rationale:**
Faster and more accurate than manual entry. Reduces errors from typos. Industry standard practice.

**Acceptance Criteria:**
- User taps "Admit Patient" then "Barcode Scan"
- System activates camera or barcode scanner
- User scans patient wristband barcode
- System extracts MRN from barcode
- System performs HIS lookup (same as manual entry)
- System displays patient information for confirmation
- User confirms and patient is admitted

**Related Requirements:**
- REQ-FUN-PAT-001 (manual admission)
- REQ-FUN-PAT-010 (patient lookup)

**Traces To:**
- Use Case: UC-PM-001 (alternative flow A1)
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Test: Test-PAT-002

**Notes:**
- Requires barcode scanner hardware or camera
- Barcode format must be configurable (hospital-specific)

---

### [REQ-FUN-PAT-003] Patient Admission - Central Station Push

**Category:** Patient Management  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall accept patient admission pushed from the Central Station, allowing centralized patient assignment management.

**Rationale:**
Supports centralized workflow where central station manages all bed assignments. Reduces duplicate entry.

**Acceptance Criteria:**
- User selects "Wait for Central Station Push"
- System displays QR code with device ID and IP address
- System enters listening mode
- Central Station scans QR code and pushes patient MRN
- System receives patient information
- System displays information for nurse confirmation
- Nurse confirms and patient is admitted

**Related Requirements:**
- REQ-FUN-PAT-001 (manual admission)
- REQ-FUN-DEV-001 (device provisioning)
- REQ-INT-CENTRAL-001 (central station interface)

**Traces To:**
- Use Case: UC-PM-001 (alternative flow A2)
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Test: Test-PAT-003

**Notes:**
- Requires network connectivity
- Central Station must be authorized to push assignments

---

### [REQ-FUN-PAT-004] Patient Discharge

**Category:** Patient Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall allow authorized users to discharge a patient from the device, ending the monitoring session.

**Rationale:**
Required to free device for next patient. Ensures clean separation between patient records.

**Acceptance Criteria:**
- User selects "Discharge Patient" from patient menu
- System displays confirmation dialog with patient name
- System shows sync status (all data synced yes/no)
- User confirms discharge
- System stops vital signs monitoring
- System performs final data sync to server
- System clears patient information from display
- System updates banner to "DISCHARGED / STANDBY"
- Discharge logged with user ID and timestamp

**Related Requirements:**
- REQ-FUN-PAT-001 (admission)
- REQ-FUN-DATA-001 (data sync)
- REQ-FUN-VITAL-001 (monitoring)

**Traces To:**
- Use Case: UC-PM-002
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager)
- Test: Test-PAT-004

---

### [REQ-FUN-PAT-005] Patient Transfer

**Category:** Patient Management  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall allow authorized users to transfer a patient from one device to another, maintaining continuity of care.

**Rationale:**
Patients move between rooms/units. Transfer preserves patient history and settings.

**Acceptance Criteria:**
- User selects "Transfer Patient"
- System displays list of available target devices
- User selects target device and enters reason
- System syncs all patient data to central server
- Central server pushes admission to target device
- Target device confirms admission
- Source device discharges patient
- Transfer logged on both devices

**Related Requirements:**
- REQ-FUN-PAT-001 (admission)
- REQ-FUN-PAT-004 (discharge)
- REQ-FUN-DATA-001 (data sync)

**Traces To:**
- Use Case: UC-PM-003
- Design: [19_ADT_WORKFLOW.md](../architecture_and_design/19_ADT_WORKFLOW.md)
- Test: Test-PAT-005

---

### [REQ-FUN-PAT-010] Patient Information Lookup

**Category:** Patient Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall query the Hospital Information System (HIS/EHR) to retrieve patient demographics based on MRN.

**Rationale:**
Ensures patient information is accurate and up-to-date. Reduces manual data entry errors.

**Acceptance Criteria:**
- System accepts MRN as input
- System validates MRN format
- System queries HIS via IPatientLookupService interface
- System returns patient name, DOB, sex, bed location
- Lookup completes within 5 seconds or times out
- System caches results locally for offline access
- Lookup logged for audit

**Related Requirements:**
- REQ-FUN-PAT-001 (admission)
- REQ-FUN-PAT-011 (patient cache)
- REQ-INT-HIS-001 (HIS interface)

**Traces To:**
- Use Case: UC-PM-004
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (PatientManager, IPatientLookupService)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (patients table)
- Test: Test-PAT-010

---

### [REQ-FUN-PAT-011] Patient Information Caching

**Category:** Patient Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall cache patient information locally to enable offline operation when HIS is unavailable.

**Rationale:**
Network outages should not prevent patient care. Local cache enables graceful degradation.

**Acceptance Criteria:**
- System stores patient info in local database after successful HIS lookup
- Cache includes MRN, name, DOB, sex, bed location, lookup timestamp
- Cache entries valid for 24 hours
- System checks cache before HIS query
- Stale cache used with warning if HIS unavailable
- Cache encrypted with SQLCipher

**Related Requirements:**
- REQ-FUN-PAT-010 (patient lookup)
- REQ-DATA-SEC-001 (encryption at rest)
- REQ-NFR-REL-005 (offline operation)

**Traces To:**
- Use Case: UC-PM-004 (alternative flow A1, A2)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (patients table)
- Test: Test-PAT-011

---

## 6. Vital Signs Monitoring Requirements

### [REQ-FUN-VITAL-001] Real-Time Vital Signs Display

**Category:** Vital Signs Monitoring  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall display patient vital signs in real-time, updating at least once per second.

**Rationale:**
Core function of patient monitor. Clinicians need current vital signs to assess patient status.

**Acceptance Criteria:**
- System displays heart rate, SpO2, respiration rate
- Values update at 1 Hz minimum (every second)
- Values displayed in large, readable font
- Units displayed (bpm, %, rpm)
- Timestamp of last update shown
- Color coding: green (normal), yellow (warning), red (critical)

**Related Requirements:**
- REQ-FUN-VITAL-002 (vital signs recording)
- REQ-FUN-VITAL-010 (trend display)
- REQ-NFR-PERF-100 (display latency)

**Traces To:**
- Use Case: UC-VM-001
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DeviceSimulator)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Real-time thread)
- Test: Test-VITAL-001

---

### [REQ-FUN-VITAL-002] Vital Signs Data Recording

**Category:** Vital Signs Monitoring  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall record all vital signs data to local database for historical analysis and compliance.

**Rationale:**
Required for trend analysis, legal documentation, quality improvement. Retention policies mandate data storage.

**Acceptance Criteria:**
- System stores each vital sign reading in vitals table
- Record includes: timestamp, patient_mrn, device_id, heart_rate, spo2, respiration_rate, signal_quality
- Database write does not block UI updates
- Data retained for 7 days minimum
- Database protected with encryption (SQLCipher)

**Related Requirements:**
- REQ-FUN-VITAL-001 (display)
- REQ-DATA-RET-001 (retention policy)
- REQ-DATA-SEC-001 (encryption)

**Traces To:**
- Use Case: UC-VM-001
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (vitals table)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Database I/O thread)
- Test: Test-VITAL-002

---

### [REQ-FUN-VITAL-003] Vital Signs Validation

**Category:** Vital Signs Monitoring  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall validate vital signs values are within physiologically plausible ranges before display or alarm triggering.

**Rationale:**
Invalid sensor readings should not trigger false alarms or display incorrect data that could mislead clinicians.

**Acceptance Criteria:**
- System validates ranges:
  - Heart Rate: 30-250 bpm
  - SpO2: 50-100%
  - Respiration Rate: 5-60 rpm
- Out-of-range values flagged as INVALID
- Invalid values not displayed (show last valid value)
- Invalid values do not trigger clinical alarms
- Technical alarm triggered for persistent invalid data
- Validation failures logged

**Related Requirements:**
- REQ-FUN-VITAL-001 (display)
- REQ-FUN-ALARM-001 (alarm triggering)

**Traces To:**
- Use Case: UC-VM-001 (exception flow E1)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DeviceSimulator)
- Test: Test-VITAL-003

---

### [REQ-FUN-VITAL-010] Vital Signs Trend Visualization

**Category:** Vital Signs Monitoring  
**Priority:** High  
**Status:** Approved

**Description:**
The system shall display historical vital signs trends as line charts for time ranges up to 7 days.

**Rationale:**
Clinicians need to see trends to identify patterns, deterioration, or improvement. Critical for clinical decision-making.

**Acceptance Criteria:**
- System displays trend view with configurable time range: 1hr, 4hr, 12hr, 24hr, 7 days
- User can select which vital signs to display (HR, SpO2, RR, or All)
- Line chart rendered with time on X-axis, values on Y-axis
- Alarm threshold lines shown as dotted red lines
- Alarm events marked on chart with icons
- Chart supports zoom and pan
- Query and render complete within 2 seconds
- Hover shows exact values and timestamps

**Related Requirements:**
- REQ-FUN-VITAL-002 (data recording)
- REQ-DATA-RET-001 (retention)
- REQ-NFR-PERF-110 (query performance)

**Traces To:**
- Use Case: UC-VM-002
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (vitals table, indices)
- Test: Test-VITAL-010

---

### [REQ-FUN-VITAL-011] Waveform Display

**Category:** Vital Signs Monitoring  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall display real-time waveforms (ECG, plethysmogram) at appropriate sampling rates for clinical assessment.

**Rationale:**
Waveforms provide critical diagnostic information beyond numeric values. Clinicians assess rhythm, morphology, quality.

**Acceptance Criteria:**
- System displays ECG waveform at 250 Hz sample rate
- Waveform updates smoothly (60 FPS rendering)
- Waveform spans 10 seconds visible width
- Auto-scrolling display (rightmost is newest)
- Signal quality indicator shown
- Waveform gain adjustable

**Related Requirements:**
- REQ-FUN-VITAL-001 (display)
- REQ-NFR-PERF-101 (graphics performance)

**Traces To:**
- Use Case: UC-VM-001 (alternative flow A1)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Real-time thread)
- Test: Test-VITAL-011

**Notes:**
- High-performance graphics required (QML Canvas or OpenGL)
- Downsampling for storage (store every 10th sample)

---

## 7. Alarm Management Requirements

### [REQ-FUN-ALARM-001] Threshold-Based Alarm Triggering

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall automatically trigger alarms when vital signs exceed configured thresholds.

**Rationale:**
Core safety function. Alarms alert clinicians to adverse patient conditions requiring intervention.

**Acceptance Criteria:**
- System continuously monitors vital signs against thresholds
- Alarm triggered when value exceeds threshold
- Alarm delay applied (2 seconds) to filter transient spikes
- Alarm includes: priority, type, value, threshold, patient, timestamp
- Alarm logged to database immediately
- Alarm synced to central server
- Alarm displayed visually and audibly

**Related Requirements:**
- REQ-FUN-ALARM-002 (audio alarm)
- REQ-FUN-ALARM-003 (visual alarm)
- REQ-FUN-ALARM-010 (alarm configuration)
- REQ-REG-60601-001 (IEC 60601-1-8 compliance)

**Traces To:**
- Use Case: UC-AM-001
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-ALARM-001

---

### [REQ-FUN-ALARM-002] Audio Alarm Notification

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall generate distinctive audio alarm patterns compliant with IEC 60601-1-8 for different alarm priorities.

**Rationale:**
IEC 60601-1-8 mandates specific audio patterns for medical alarms. Distinctive sounds enable clinicians to assess urgency without looking at screen.

**Acceptance Criteria:**
- High priority: Continuous rapid beeps (1-10-1-10 pattern, 10 beep bursts)
- Medium priority: 3 beeps with 1-second interval (1-10-1-10-1 pattern)
- Low priority: Single beep, no repeat
- Audio latency < 100ms from alarm trigger
- Volume: 65-85 dB at 1 meter (configurable)
- Audio continues until acknowledged
- Audio failure triggers technical alarm

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-020 (acknowledgment)
- REQ-REG-60601-002 (audio requirements)

**Traces To:**
- Use Case: UC-AM-001 (step 8)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Design: [12_THREAD_MODEL.md](../architecture_and_design/12_THREAD_MODEL.md) (Main thread)
- Test: Test-ALARM-002

**Notes:**
- Audio patterns per IEC 60601-1-8 Table 3
- Speaker health check required

---

### [REQ-FUN-ALARM-003] Visual Alarm Indication

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall display visual alarm indications using color, text, and icons to communicate alarm status.

**Rationale:**
Complements audio alarms. Critical for noisy environments or hearing-impaired staff. IEC 60601-1-8 requires visual indication.

**Acceptance Criteria:**
- High priority: Flashing red background, "CRITICAL" banner, large alarm text
- Medium priority: Red border around affected value, alarm banner, notification bell icon
- Low priority: Yellow highlight, subtle notification
- Alarm banner shows: alarm type, value, threshold, timestamp
- Notification bell shows count of active alarms
- Visual indication persists until acknowledged
- Visual latency < 100ms

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-020 (acknowledgment)
- REQ-NFR-USE-010 (visibility from 10 feet)

**Traces To:**
- Use Case: UC-AM-001 (step 9)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Test: Test-ALARM-003

---

### [REQ-FUN-ALARM-010] Alarm Threshold Configuration

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall allow authorized users (physicians) to configure alarm thresholds per patient based on clinical needs.

**Rationale:**
One-size-fits-all thresholds cause excessive false alarms. Patient-specific thresholds reduce alarm fatigue while maintaining safety.

**Acceptance Criteria:**
- Settings view provides threshold configuration UI
- Configurable thresholds: HR (low/high), SpO2 (low/high), RR (low/high)
- Default thresholds loaded at patient admission
- Threshold changes require physician authorization
- Threshold changes logged with user ID and timestamp
- New thresholds take effect immediately
- Threshold changes synced to central server

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-USER-004 (role permissions)

**Traces To:**
- Use Case: UC-AM-004
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (SettingsManager, AlarmManager)
- Test: Test-ALARM-010

---

### [REQ-FUN-ALARM-020] Alarm Acknowledgment

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall allow users to acknowledge alarms to indicate awareness, stopping audio while maintaining visual indication.

**Rationale:**
Acknowledgment confirms nurse is aware of alarm and assessing patient. Reduces noise while maintaining safety.

**Acceptance Criteria:**
- User taps alarm banner or notification bell
- System displays alarm details
- User taps "Acknowledge" button
- Audio alarm stops (or pauses for critical alarms)
- Visual indication changes to acknowledged state (yellow)
- Acknowledgment logged with user ID, timestamp, response time
- Acknowledged alarm remains visible until condition resolves
- Critical alarms can only pause (2 min), not silence

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-030 (escalation)
- REQ-SEC-AUDIT-001 (audit logging)

**Traces To:**
- Use Case: UC-AM-002
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-ALARM-020

---

### [REQ-FUN-ALARM-021] Alarm Silence

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall allow users to temporarily silence alarm audio for a configurable duration (e.g., during bedside procedures).

**Rationale:**
Some procedures (suctioning, position changes) temporarily affect vital signs. Silence prevents nuisance alarms while procedure performed.

**Acceptance Criteria:**
- User can access "Silence Alarms" function
- System prompts for silence duration: 2 min, 5 min, 10 min
- Audio alarms muted for selected duration
- Visual indication: "ALARMS SILENCED - X min remaining"
- Timer countdown displayed
- New critical alarms override silence (always audible)
- Silence action logged with user ID, duration, reason

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-002 (audio)
- REQ-REG-60601-003 (silence duration limits)

**Traces To:**
- Use Case: UC-AM-003
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- Test: Test-ALARM-021

**Notes:**
- IEC 60601-1-8 limits silence duration to 10 minutes maximum
- Critical alarms cannot be silenced per regulation

---

### [REQ-FUN-ALARM-030] Alarm Escalation

**Category:** Alarm Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall escalate unacknowledged alarms to ensure they are addressed, preventing missed critical events.

**Rationale:**
Safety requirement. Unacknowledged alarms may indicate nurse is unavailable or alarm not heard. Escalation ensures patient safety.

**Acceptance Criteria:**
- System starts escalation timer when alarm triggered:
  - Critical: 30 seconds
  - Medium: 2 minutes
  - Low: 5 minutes
- If not acknowledged before timer expires, alarm escalates
- Escalation actions:
  - Increase audio volume
  - Notify central station
  - Send alert to charge nurse
  - Log escalation event
- Escalation continues until acknowledged
- Escalation can be disabled by administrator (not recommended)

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-020 (acknowledgment)
- REQ-INT-CENTRAL-002 (central station alerts)

**Traces To:**
- Use Case: UC-AM-006
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AlarmManager)
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-ALARM-030

---

## 8. Device Management Requirements

### [REQ-FUN-DEV-001] Device Provisioning via QR Code

**Category:** Device Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall support secure device provisioning through QR code scanning, enabling rapid device setup without manual configuration.

**Rationale:**
Manual configuration error-prone and time-consuming. QR code provisioning ensures correct configuration and improves security through certificate-based authentication.

**Acceptance Criteria:**
- Device displays QR code containing device ID and IP address
- Technician scans QR code with authorized provisioning app
- Provisioning service generates configuration payload:
  - Server URL
  - Client certificate
  - Client private key
  - CA certificate
  - Device label
- Configuration encrypted and signed
- Device receives and validates configuration
- Device applies configuration automatically
- Provisioning complete in < 5 minutes

**Related Requirements:**
- REQ-FUN-DEV-002 (certificate management)
- REQ-SEC-CERT-001 (mTLS certificates)
- REQ-INT-PROV-001 (provisioning service)

**Traces To:**
- Use Case: UC-DP-001, UC-DP-002, UC-DP-003
- Design: [17_DEVICE_PROVISIONING.md](../architecture_and_design/17_DEVICE_PROVISIONING.md)
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Test: Test-DEV-001

---

### [REQ-FUN-DEV-002] Certificate Management

**Category:** Device Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall manage client certificates for mTLS authentication, including storage, renewal, and revocation.

**Rationale:**
mTLS required for secure server communication. Certificate lifecycle management ensures continuous secure connectivity.

**Acceptance Criteria:**
- Device stores certificates securely (encrypted database)
- Certificates tracked: serial, subject, issuer, not_before, not_after
- Device checks certificate expiry on startup and daily
- Warning displayed when certificate expires in < 30 days
- Automatic renewal initiated at 30-day threshold
- Revoked certificates detected and device prevented from connecting
- Certificate validation logged

**Related Requirements:**
- REQ-FUN-DEV-001 (provisioning)
- REQ-SEC-CERT-001 (mTLS)
- REQ-SEC-CERT-002 (certificate renewal)

**Traces To:**
- Use Case: UC-DP-004, UC-DP-005
- Design: [15_CERTIFICATE_PROVISIONING.md](../architecture_and_design/15_CERTIFICATE_PROVISIONING.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (certificates table)
- Test: Test-DEV-002

---

### [REQ-FUN-DEV-010] System Diagnostics

**Category:** Device Management  
**Priority:** Should Have  
**Status:** Approved

**Description:**
The system shall provide diagnostic tools for technicians to troubleshoot device issues and verify proper operation.

**Rationale:**
Enables rapid issue diagnosis and resolution. Reduces device downtime. Supports preventive maintenance.

**Acceptance Criteria:**
- Diagnostics view accessible to technicians
- Tests available:
  - Display test (color bars, pixel test)
  - Audio test (speaker verification)
  - Network connectivity test
  - Database integrity check
  - Certificate validation
  - Sensor simulator test
- Each test displays pass/fail result
- Test results logged
- Comprehensive diagnostic report exportable

**Related Requirements:**
- REQ-FUN-SYS-001 (system configuration)
- REQ-FUN-REP-010 (logging)

**Traces To:**
- Use Case: UC-SC-003
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (SystemController, DiagnosticsService)
- Test: Test-DEV-010

---

## 9. Data Management Requirements

### [REQ-FUN-DATA-001] Real-Time Data Synchronization

**Category:** Data Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall continuously synchronize vitals and alarm data to the central server to maintain centralized patient record.

**Rationale:**
Central server provides consolidated view of all patients. Real-time sync enables remote monitoring and ensures data preservation if device fails.

**Acceptance Criteria:**
- System syncs vital signs data every 10 seconds (batch)
- System syncs alarms immediately (< 1 second)
- Sync uses mTLS encrypted connection
- Unsync data tracked with is_synced flag in database
- Sync retries on failure (exponential backoff)
- Sync status visible to users
- Sync failures logged

**Related Requirements:**
- REQ-FUN-VITAL-002 (data recording)
- REQ-FUN-ALARM-001 (alarm logging)
- REQ-INT-SRV-001 (server API)
- REQ-SEC-ENC-001 (encryption in transit)

**Traces To:**
- Use Case: UC-DS-001, UC-DS-002
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager, DatabaseManager)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (is_synced flag)
- Test: Test-DATA-001

---

### [REQ-FUN-DATA-002] Offline Data Queuing

**Category:** Data Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall queue data locally when network is unavailable and sync automatically when connectivity restored.

**Rationale:**
Network outages should not cause data loss. Device must continue functioning offline and sync when able.

**Acceptance Criteria:**
- System continues recording data when offline
- Data marked as unsync (is_synced = 0)
- System detects network restoration
- System automatically resumes sync
- Queued data synced in chronological order
- Large queues synced in batches to avoid overwhelming server
- Sync progress displayed to user

**Related Requirements:**
- REQ-FUN-DATA-001 (sync)
- REQ-NFR-REL-005 (offline operation)

**Traces To:**
- Use Case: UC-DS-003, UC-DS-004
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (NetworkManager)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (is_synced flag)
- Test: Test-DATA-002

---

### [REQ-FUN-DATA-010] Data Retention and Archival

**Category:** Data Management  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall retain patient data locally for 7 days and automatically archive or delete older data per retention policy.

**Rationale:**
Compliance and storage management. 7 days provides buffer for catch-up sync and local trend analysis while managing storage space.

**Acceptance Criteria:**
- Vitals data retained for 7 days
- Alarms data retained for 90 days
- Patient records retained for 90 days after discharge
- Automatic cleanup job runs daily at 2 AM
- Data older than retention period deleted
- Cleanup actions logged
- Critical data (code blues) flagged for extended retention

**Related Requirements:**
- REQ-FUN-VITAL-002 (data recording)
- REQ-DATA-RET-001 (retention policy)
- REQ-REG-HIPAA-002 (data retention compliance)

**Traces To:**
- Use Case: Implicit (background process)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (retention section)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (DataArchiver)
- Test: Test-DATA-010

---

## 10. System Configuration Requirements

### [REQ-FUN-SYS-001] Device Settings Management

**Category:** System Configuration  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall provide interface for authorized users to configure device settings including device label, measurement units, language, and display preferences.

**Rationale:**
Devices must be configurable to hospital environment and user preferences. Device label identifies physical location.

**Acceptance Criteria:**
- Settings view accessible to technicians and administrators
- Configurable settings:
  - Device Label (e.g., "ICU-MON-04")
  - Measurement Units (metric/imperial)
  - Language (English, Spanish, etc.)
  - Screen brightness
  - Alarm volume
- Settings persisted to database
- Setting changes logged with user ID
- Setting changes take effect immediately

**Related Requirements:**
- REQ-FUN-USER-004 (permissions)
- REQ-FUN-DEV-001 (device provisioning)

**Traces To:**
- Use Case: UC-SC-001
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (SettingsManager, SettingsController)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (settings table)
- Test: Test-SYS-001

---

## 11. Requirements Summary

### Total Requirements: 50 (of ~150-200 planned)

| Category | Requirements | Must Have | Should Have | Nice to Have |
|----------|--------------|-----------|-------------|--------------|
| User Management | 5 | 5 | 0 | 0 |
| Patient Management | 6 | 4 | 2 | 0 |
| Vital Signs | 5 | 4 | 1 | 0 |
| Alarm Management | 7 | 7 | 0 | 0 |
| Device Management | 3 | 2 | 1 | 0 |
| Data Management | 3 | 3 | 0 | 0 |
| System Configuration | 1 | 1 | 0 | 0 |
| **Total** | **30** | **26** | **4 | **0** |

### Coverage Status:
- ✅ User Management: 100% (5/5)
- ✅ Patient Management: 100% (6/6)
- ✅ Vital Signs: 100% (5/5 core)
- ✅ Alarm Management: 100% (7/7 core)
- 🔶 Device Management: 40% (3/8)
- 🔶 Data Management: 50% (3/6)
- 🔶 System Configuration: 20% (1/5)
- ⏳ Reporting & Analytics: 0% (0/20)

### Remaining Requirements (to be added):
- ~40 more vital signs requirements (waveforms, manual entry, export)
- ~20 device management requirements (firmware, diagnostics, logs)
- ~15 data management requirements (backup, export, import)
- ~20 system configuration requirements (advanced settings)
- ~25 reporting & analytics requirements (logs, audit trails, reports)
- ~30 additional requirements across all categories

---

## 12. Related Documents

- **Use Cases:** [02_USE_CASES.md](./02_USE_CASES.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **Traceability:** [10_REQUIREMENTS_TRACEABILITY.md](./10_REQUIREMENTS_TRACEABILITY.md)
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)

---

*Functional requirements drive system design and implementation. All features must trace to documented requirements.*

