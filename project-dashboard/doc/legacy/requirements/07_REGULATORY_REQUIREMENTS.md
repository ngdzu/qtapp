# Regulatory Requirements

**Document ID:** REQ-DOC-07  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document specifies regulatory compliance requirements for the Z Monitor system. As a medical device handling patient health information (PHI), the system must comply with medical device software standards and healthcare privacy regulations.

**Related Documents:**
- **Security:** [../architecture_and_design/06_SECURITY.md](../architecture_and_design/06_SECURITY.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **Testing Workflow:** [../architecture_and_design/18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)

---

## 2. Regulatory Framework Overview

### 2.1 Applicable Regulations

The Z Monitor system must comply with:

1. **IEC 62304** - Medical Device Software Life Cycle Processes
2. **IEC 60601-1-8** - Medical Electrical Equipment - Alarm Systems
3. **IEC 62443** - Industrial Communication Networks - Network and System Security
4. **HIPAA** - Health Insurance Portability and Accountability Act (US)
5. **FDA 21 CFR Part 11** - Electronic Records and Signatures (if applicable)
6. **ISO 14971** - Medical Devices - Application of Risk Management
7. **ISO 13485** - Medical Devices - Quality Management Systems

### 2.2 Device Classification

- **Device Type:** Patient Monitoring System
- **Risk Class:** Class II Medical Device (moderate risk)
- **Safety Class:** IEC 60601-1 Class I (electrically powered)
- **Software Safety Class:** IEC 62304 Class B (medium risk - injury possible but not death)

---

## 3. IEC 62304 - Medical Device Software Requirements

### [REQ-REG-62304-001] Software Development Lifecycle

**Category:** IEC 62304  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system development shall follow IEC 62304 software development lifecycle processes appropriate for Class B medical device software.

**Rationale:**
IEC 62304 is the international standard for medical device software development. Compliance required for regulatory approval (FDA, EU MDR).

**Acceptance Criteria:**
- **Software Development Planning:** Documented in project plan
- **Requirements Analysis:** All requirements documented and traceable
- **Software Architecture:** Architecture documented with rationale
- **Detailed Design:** Class designs and interface specifications
- **Unit Testing:** Unit tests for all critical components (90%+ coverage)
- **Integration Testing:** Integration tests for component interactions
- **System Testing:** System tests verify requirements
- **Risk Management:** Risk analysis per ISO 14971
- **Configuration Management:** Version control, change control
- **Problem Resolution:** Bug tracking and resolution process

**Documentation Requirements:**
- Software Development Plan (SDP)
- Software Requirements Specification (SRS) - **This document and related**
- Software Architecture Document (SAD) - **02_ARCHITECTURE.md**
- Software Detailed Design (SDD) - **09_CLASS_DESIGNS.md, etc.**
- Software Test Plan (STP) - **18_TESTING_WORKFLOW.md**
- Software Risk Management File
- Software Configuration Management Plan

**Related Requirements:**
- REQ-NFR-MAIN-002 (testability)
- REQ-REG-62304-010 (testing)
- REQ-REG-14971-001 (risk management)

**Traces To:**
- Design: All architecture and design documents
- Design: [18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)
- Test: All test suites
- Process: Software development process documentation

**Notes:**
- Class B: Injury possible but not probable (vs Class C: death/serious injury probable)
- Requires comprehensive documentation for regulatory submission
- Third-party verification recommended

---

### [REQ-REG-62304-010] Software Testing and Verification

**Category:** IEC 62304  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall implement comprehensive testing strategy including unit, integration, system, and regression testing per IEC 62304 requirements.

**Rationale:**
Testing is critical verification activity. IEC 62304 mandates testing appropriate for software safety class.

**Acceptance Criteria:**
- **Unit Tests:**
  - Coverage: 90%+ for Class B critical components
  - Automated execution (GoogleTest framework)
  - Test isolation (mocks, dependency injection)
- **Integration Tests:**
  - Component interaction testing
  - Interface testing (IPatientLookupService, ITelemetryServer, etc.)
  - Database integration testing
- **System Tests:**
  - End-to-end workflow testing
  - Requirements verification (all requirements tested)
  - Use case validation (all use cases tested)
- **Regression Tests:**
  - Automated regression suite
  - Run on every commit (CI/CD)
  - No regression allowed (new bugs breaking old functionality)
- **Test Documentation:**
  - Test plan (strategy, scope, resources)
  - Test cases (inputs, expected outputs, pass/fail criteria)
  - Test results (execution logs, coverage reports)
  - Traceability matrix (requirements → tests)

**Related Requirements:**
- REQ-NFR-MAIN-002 (testability 80%+ coverage)
- REQ-REG-62304-001 (lifecycle)

**Traces To:**
- Design: [18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)
- Test: All test suites (unit, integration, e2e)
- Process: Test execution logs, coverage reports

**Notes:**
- Test-driven development (TDD) recommended
- Continuous integration ensures tests run regularly
- Independent verification team recommended

---

### [REQ-REG-62304-020] Software Configuration Management

**Category:** IEC 62304  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement configuration management including version control, change control, and release management per IEC 62304.

**Rationale:**
Configuration management ensures traceability of changes and enables reproducible builds. Required for regulatory compliance.

**Acceptance Criteria:**
- **Version Control:**
  - Git repository for all source code
  - Git tags for releases (v1.0.0, v1.1.0, etc.)
  - Branch strategy (main, develop, feature branches)
- **Change Control:**
  - All changes tracked (commit messages, PR descriptions)
  - Change approval process for critical components
  - Change review (code review required)
- **Release Management:**
  - Release notes for each version
  - Reproducible builds (same source = same binary)
  - Build automation (CMake, CI/CD)
- **Baseline Management:**
  - Baselines tagged (design baseline, test baseline, release baseline)
  - Configuration items identified (source, binaries, docs, tests)
- **Problem Reports:**
  - Bug tracking system (GitHub Issues, Jira)
  - Severity classification (critical, major, minor)
  - Resolution tracking (open, in progress, resolved, verified)

**Related Requirements:**
- REQ-REG-62304-001 (lifecycle)
- REQ-NFR-MAIN-001 (modularity)

**Traces To:**
- Process: Git repository, CI/CD workflows
- Process: Change control procedures
- Process: Release checklist

**Notes:**
- IEC 62304 Section 8 - Configuration Management
- Automated CI/CD helps ensure compliance

---

## 4. IEC 60601-1-8 - Alarm System Requirements

### [REQ-REG-60601-001] Alarm System Design Requirements

**Category:** IEC 60601-1-8  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system alarm design shall comply with IEC 60601-1-8 requirements for medical electrical equipment alarm systems.

**Rationale:**
**SAFETY CRITICAL.** Alarm system is primary patient safety mechanism. IEC 60601-1-8 is international standard for medical alarm systems.

**Acceptance Criteria:**
- **Alarm Priorities:**
  - HIGH: Immediate response required (patient life threat)
  - MEDIUM: Prompt response required (potential patient harm)
  - LOW: Awareness required (abnormal but not immediately dangerous)
- **Alarm Signals:**
  - Visual: Color-coded (red, yellow, cyan), flashing
  - Auditory: Distinctive patterns per priority (Table 3)
- **Alarm Timing:**
  - Detection latency: < 50ms (REQ-NFR-PERF-100)
  - Audio latency: < 100ms
  - Visual latency: < 100ms
- **Alarm Acknowledgment:**
  - User can acknowledge to silence audio temporarily
  - Critical alarms: Audio pause 2 minutes maximum
  - Visual indication persists until condition resolves
- **Alarm Escalation:**
  - Unacknowledged alarms escalate (increase volume, notify central)
- **Alarm Logging:**
  - All alarms logged (type, time, acknowledgment, resolution)

**IEC 60601-1-8 Audio Patterns:**
- **HIGH Priority:** c-c-c-c-c c-c-c-c-c (10 pulses, burst 2x, 2.5s cycle)
- **MEDIUM Priority:** c-c-c (3 pulses, once, then pause)
- **LOW Priority:** c (1 pulse)
- **Pulse:** 1 unit duration, **Pause:** 10 units

**Related Requirements:**
- REQ-FUN-ALARM-001 (triggering)
- REQ-FUN-ALARM-002 (audio)
- REQ-FUN-ALARM-003 (visual)
- REQ-NFR-PERF-100 (latency)
- REQ-NFR-REL-100 (reliability)

**Traces To:**
- Use Case: UC-AM-001
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-REG-60601-001 (alarm testing per IEC)

**Notes:**
- IEC 60601-1-8:2006+AMD1:2012 current version
- Usability testing required (user comprehension of alarms)
- Third-party testing lab recommended for certification

---

### [REQ-REG-60601-002] Audio Alarm Performance

**Category:** IEC 60601-1-8  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system audio alarms shall meet IEC 60601-1-8 performance requirements for sound pressure level, frequency, and audibility.

**Rationale:**
Audio alarms must be audible in noisy clinical environment. IEC specifies minimum performance.

**Acceptance Criteria:**
- **Sound Pressure Level (SPL):**
  - Minimum: 45 dB(A) at operator position (1 meter)
  - Maximum: 85 dB(A) at operator position
  - Adjustable volume (user-configurable)
  - Default: 65 dB(A)
- **Frequency Range:**
  - 300-4000 Hz (optimal for human hearing)
  - Peak frequency: 500-1000 Hz (most audible)
- **Ambient Noise Rejection:**
  - Alarm audible in 65 dB(A) ambient noise
  - Distinctive from other sounds (phone, doors, equipment)
- **Speaker Performance:**
  - Frequency response: Flat ±3dB in alarm frequency range
  - Distortion: < 5% THD
  - Reliability: Speaker self-test on startup
- **Alarm Pattern Timing:**
  - Pulse duration: 75-200ms
  - Pause duration: 750-2000ms
  - Pattern accuracy: ±10%

**Measurement:**
- Sound meter calibration (ANSI S1.4 Type 2 or better)
- Anechoic chamber testing (lab environment)
- Real-world testing (clinical environment simulation)

**Related Requirements:**
- REQ-FUN-ALARM-002 (audio alarm)
- REQ-INT-HW-003 (audio hardware)

**Traces To:**
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-REG-60601-002 (audio performance testing)

**Notes:**
- Testing requires acoustics lab
- Hospital ambient noise typically 50-65 dB(A)

---

### [REQ-REG-60601-003] Alarm Silence Duration Limits

**Category:** IEC 60601-1-8  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall limit alarm audio silence duration to maximum 10 minutes per IEC 60601-1-8 to ensure alarms cannot be indefinitely silenced.

**Rationale:**
Extended silence could result in missed critical events. IEC limits silence duration for patient safety.

**Acceptance Criteria:**
- **Maximum Silence Duration:** 10 minutes (configurable: 2, 5, 10 min)
- **Critical Alarm Exception:** Critical alarms can only pause 2 minutes (not full silence)
- **Silence Expiry:** Audio resumes automatically after silence expires
- **Visual Indication:** "ALARMS SILENCED - X min remaining" displayed prominently
- **Countdown Timer:** Countdown visible to user
- **Override:** New critical alarms override silence (always audible)
- **Logging:** All silence events logged (user, duration, timestamp)

**Related Requirements:**
- REQ-FUN-ALARM-021 (alarm silence)
- REQ-REG-60601-001 (alarm design)

**Traces To:**
- Use Case: UC-AM-003
- Design: [04_ALARM_SYSTEM.md](../architecture_and_design/04_ALARM_SYSTEM.md)
- Test: Test-REG-60601-003

**Notes:**
- IEC 60601-1-8 Clause 6.3.2.4 - Alarm signal inactivation
- Configurable duration must not exceed 10 minutes

---

## 5. HIPAA - Healthcare Privacy Requirements

### [REQ-REG-HIPAA-001] Encryption of PHI

**Category:** HIPAA  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall encrypt all Protected Health Information (PHI) both at rest and in transit per HIPAA Security Rule requirements.

**Rationale:**
HIPAA Security Rule mandates encryption to protect patient privacy. Violations result in severe penalties ($100-$50,000 per violation).

**Acceptance Criteria:**
- **Encryption at Rest:**
  - Database: SQLCipher with AES-256-CBC
  - Files: AES-256 encryption for any PHI files
  - Key storage: Secure key management (HSM or protected memory)
- **Encryption in Transit:**
  - TLS 1.2+ for all network communication
  - mTLS for device-to-server authentication
  - No plaintext PHI on network
- **HIPAA PHI Elements Encrypted:**
  - Patient names, MRNs, DOB
  - Vital signs (associated with patient)
  - Alarm history
  - Admission/discharge records
  - Audit logs
- **Key Management:**
  - Encryption keys not stored in plaintext
  - Key rotation supported (annual recommended)
  - Key compromise response plan
- **Compliance Documentation:**
  - Encryption algorithms documented
  - Key management procedures documented
  - Annual security risk assessment

**Related Requirements:**
- REQ-DATA-SEC-001 (database encryption)
- REQ-NFR-SEC-002 (encryption in transit)
- REQ-NFR-SEC-003 (encryption at rest)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Sections 2, 6)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md)
- Test: Test-REG-HIPAA-001 (encryption verification)

**Notes:**
- HIPAA Security Rule 45 CFR § 164.312(a)(2)(iv) and § 164.312(e)(2)(ii)
- Encryption is "addressable" but effectively required (breach notification exemption if encrypted)

---

### [REQ-REG-HIPAA-002] Access Control and Authentication

**Category:** HIPAA  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system shall implement access control and authentication mechanisms per HIPAA Security Rule to prevent unauthorized access to PHI.

**Rationale:**
HIPAA requires unique user identification, authentication, and role-based access control. Unauthorized access violations result in penalties.

**Acceptance Criteria:**
- **Unique User Identification:**
  - Each user has unique User ID
  - Shared accounts prohibited
- **Authentication:**
  - PIN-based authentication (4-6 digits)
  - PIN hashed (bcrypt or Argon2)
  - Brute force protection (3 failed attempts → 10 min lock)
- **Role-Based Access Control (RBAC):**
  - Roles: Nurse, Physician, Technician, Administrator
  - Permissions enforced (minimum necessary principle)
  - Authorization checks before PHI access
- **Session Management:**
  - Session timeout: 15 minutes inactivity
  - Automatic logout
  - Re-authentication required after timeout
- **Access Logging:**
  - All PHI access logged (who, what, when)
  - Logs retained 90 days locally, 6 years centrally
  - Audit trail tamper-evident

**Related Requirements:**
- REQ-FUN-USER-001 (authentication)
- REQ-FUN-USER-004 (RBAC)
- REQ-NFR-SEC-001 (authentication security)
- REQ-NFR-SEC-010 (audit logging)

**Traces To:**
- Use Case: UC-UA-001
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Sections 3, 4)
- Design: [09_CLASS_DESIGNS.md](../architecture_and_design/09_CLASS_DESIGNS.md) (AuthenticationService)
- Test: Test-REG-HIPAA-002

**Notes:**
- HIPAA Security Rule 45 CFR § 164.312(a)(2)(i) - Unique user identification
- HIPAA Security Rule 45 CFR § 164.308(a)(4) - Access control

---

### [REQ-REG-HIPAA-003] Audit Trail Requirements

**Category:** HIPAA  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall maintain comprehensive audit trails of all PHI access and system security events per HIPAA audit control requirements.

**Rationale:**
HIPAA requires audit logs for compliance verification and breach investigation. Audit trails demonstrate compliance and enable incident response.

**Acceptance Criteria:**
- **Events Logged:**
  - User login/logout (success/failure)
  - PHI access (patient admission, vitals view, trends)
  - PHI modification (patient update, settings change)
  - PHI export/disclosure
  - Alarm acknowledgment
  - Configuration changes
  - Certificate operations
  - Security violations (failed login, unauthorized access)
- **Log Fields:**
  - Timestamp (precise to millisecond)
  - Event type
  - User ID
  - Patient MRN (if applicable)
  - Action performed
  - Outcome (success/failure)
  - Device ID
  - IP address (if networked)
- **Log Protection:**
  - Logs encrypted (SQLCipher)
  - Logs tamper-evident (append-only)
  - Log integrity verification (checksums)
- **Log Retention:**
  - Local: 90 days minimum
  - Central: 6 years (HIPAA requirement)
  - Automated archival to central server
- **Log Review:**
  - Administrator log review capability
  - Suspicious activity detection
  - Audit report generation

**Related Requirements:**
- REQ-NFR-SEC-010 (audit logging)
- REQ-DATA-RET-003 (audit log retention)
- REQ-SEC-AUDIT-001 (comprehensive audit)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 6.7)
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Design: [10_DATABASE_DESIGN.md](../architecture_and_design/10_DATABASE_DESIGN.md) (security_audit_log)
- Test: Test-REG-HIPAA-003

**Notes:**
- HIPAA Security Rule 45 CFR § 164.312(b) - Audit controls
- HIPAA Privacy Rule 45 CFR § 164.528 - Accounting of disclosures

---

### [REQ-REG-HIPAA-004] Minimum Necessary Access

**Category:** HIPAA  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement HIPAA "minimum necessary" principle, limiting PHI access to minimum required for job function.

**Rationale:**
HIPAA Privacy Rule requires limiting PHI access to minimum necessary. Reduces privacy risk and supports compliance.

**Acceptance Criteria:**
- **Role-Based Access:**
  - Nurses/Physicians: Access only current patient data
  - Technicians: No access to patient data (device diagnostics only)
  - Administrators: Full access (justified and logged)
- **View Restrictions:**
  - Dashboard: Current patient only (not all patients)
  - Trends: Selected time range only
  - Reports: Summary data (not individual records unless justified)
- **Data Minimization:**
  - Export: Minimal fields (not entire database)
  - Sharing: Only necessary data elements
  - Display: Hide sensitive fields when not needed
- **Access Justification:**
  - High-sensitivity access requires reason (logged)
  - Administrator access requires justification
- **Break-Glass Access:**
  - Emergency override available (heavily logged)
  - Requires supervisor approval

**Related Requirements:**
- REQ-DATA-PRIV-001 (minimum necessary)
- REQ-FUN-USER-004 (RBAC)
- REQ-DATA-SEC-002 (access control)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md) (Section 4)
- Design: [03_UI_UX_GUIDE.md](../architecture_and_design/03_UI_UX_GUIDE.md)
- Test: Test-REG-HIPAA-004

**Notes:**
- HIPAA Privacy Rule 45 CFR § 164.502(b) and § 164.514(d)
- Controversial rule (interpretation varies)

---

### [REQ-REG-HIPAA-005] Breach Notification Preparation

**Category:** HIPAA  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall support breach notification requirements by enabling identification of affected individuals and audit trail retrieval for breach investigation.

**Rationale:**
HIPAA Breach Notification Rule requires notification within 60 days if PHI breach occurs. System must support breach investigation.

**Acceptance Criteria:**
- **Breach Detection:**
  - Security event logging enables breach detection
  - Unauthorized access attempts logged
  - Data export tracked
- **Impact Assessment:**
  - Ability to query: Which users accessed which patients' data (when, what)
  - Audit log query tools for breach investigation
  - Patient list generation (affected individuals)
- **Data Reconstruction:**
  - Audit trail enables reconstruction of events
  - Timestamp precision for timeline
- **Secure Deletion:**
  - Ability to securely delete compromised data
  - Deletion logged (who, what, when, why)
- **Breach Response Plan:**
  - Documented breach response procedures
  - Notification templates
  - Contact information (privacy officer, legal)

**Related Requirements:**
- REQ-NFR-SEC-010 (audit logging)
- REQ-DATA-PRIV-002 (data deletion)

**Traces To:**
- Design: [21_LOGGING_STRATEGY.md](../architecture_and_design/21_LOGGING_STRATEGY.md)
- Process: Breach response plan (to be documented)
- Test: Test-REG-HIPAA-005 (breach scenario testing)

**Notes:**
- HIPAA Breach Notification Rule 45 CFR §§ 164.400-414
- Breach = unauthorized access, use, or disclosure of PHI
- Notification within 60 days of discovery

---

## 6. IEC 62443 - Cybersecurity Requirements

### [REQ-REG-62443-001] Network and System Security

**Category:** IEC 62443  
**Priority:** Must Have  
**Status:** Approved

**Description:**
The system shall implement cybersecurity measures per IEC 62443 (industrial cybersecurity) adapted for medical device context.

**Rationale:**
Medical devices increasingly targeted by cyber attacks. IEC 62443 provides framework for industrial cybersecurity applicable to medical devices.

**Acceptance Criteria:**
- **Security Levels:**
  - Target: IEC 62443-4-2 Security Level 2 (protection against intentional violation using simple means)
- **Foundational Requirements (FR):**
  - FR1: Identification and Authentication (REQ-FUN-USER-001)
  - FR2: Use Control (REQ-FUN-USER-004 RBAC)
  - FR3: System Integrity (REQ-DATA-INT-002 transactions, checksums)
  - FR4: Data Confidentiality (REQ-NFR-SEC-002/003 encryption)
  - FR5: Restricted Data Flow (firewall, network segmentation)
  - FR6: Timely Response to Events (REQ-FUN-ALARM-001)
  - FR7: Resource Availability (REQ-NFR-REL-001 uptime)
- **Security Lifecycle:**
  - Threat modeling (attack surface analysis)
  - Vulnerability assessment
  - Penetration testing
  - Security patching process
- **Security Documentation:**
  - Security risk assessment
  - Security architecture
  - Secure development practices

**Related Requirements:**
- REQ-NFR-SEC-001/002/003 (security)
- REQ-REG-HIPAA-001 (encryption)
- REQ-REG-14971-001 (risk management)

**Traces To:**
- Design: [06_SECURITY.md](../architecture_and_design/06_SECURITY.md)
- Test: Test-REG-62443-001 (penetration testing)

**Notes:**
- IEC 62443 originally for industrial control systems (ICS)
- Adapted for medical device cybersecurity
- FDA recommends IEC 62443 for medical device cybersecurity

---

## 7. ISO 14971 - Risk Management Requirements

### [REQ-REG-14971-001] Risk Management Process

**Category:** ISO 14971  
**Priority:** Critical (Must Have)  
**Status:** Approved

**Description:**
The system development shall follow ISO 14971 risk management process throughout the software lifecycle.

**Rationale:**
ISO 14971 is international standard for medical device risk management. Required for regulatory approval and patient safety.

**Acceptance Criteria:**
- **Risk Management Plan:**
  - Risk management activities defined
  - Risk acceptability criteria
  - Risk control measures
- **Risk Analysis:**
  - Hazard identification (failure modes)
  - Risk estimation (severity × probability)
  - Risk evaluation (acceptable vs unacceptable)
- **Risk Control:**
  - Risk control options (eliminate, reduce, information)
  - Implementation verification
  - Residual risk evaluation
- **Risk/Benefit Analysis:**
  - Benefits outweigh residual risks
  - Documented justification
- **Risk Management File:**
  - All risk activities documented
  - Traceability (hazard → risk → control → verification)
  - Periodic review (annual or after incidents)
- **Post-Market Surveillance:**
  - Field feedback collection
  - Incident reporting
  - Risk re-evaluation

**Key Hazards for Z Monitor:**
- **Alarm System Failure:** Missed critical alarm → patient harm
- **False Alarms:** Alarm fatigue → desensitization → missed real alarms
- **Data Loss:** Network failure → incomplete patient record
- **Incorrect Patient Assignment:** Wrong patient data → treatment error
- **Software Defect:** Crash during monitoring → loss of surveillance
- **Cybersecurity:** Unauthorized access → privacy breach, data tampering

**Risk Control Measures:**
- Redundant alarm mechanisms (audio + visual)
- Alarm testing (IEC 60601-1-8 compliance)
- Offline data queuing
- Patient admission confirmation
- Watchdog process, automatic restart
- Encryption, authentication, audit logging

**Related Requirements:**
- All requirements (risk management informs requirements)
- REQ-REG-62304-001 (lifecycle integration)

**Traces To:**
- Process: Risk Management File (to be maintained)
- Design: [20_ERROR_HANDLING_STRATEGY.md](../architecture_and_design/20_ERROR_HANDLING_STRATEGY.md)
- Test: Hazard verification testing

**Notes:**
- ISO 14971:2019 current version
- Risk management ongoing (not one-time activity)
- Essential for FDA 510(k) submission or EU MDR compliance

---

## 8. Regulatory Requirements Summary

### Total Requirements: 13 (of ~20-25 planned)

| Category | Requirements | Critical | Must Have | Should Have |
|----------|--------------|----------|-----------|-------------|
| IEC 62304 | 3 | 2 | 1 | 0 |
| IEC 60601-1-8 | 3 | 2 | 1 | 0 |
| HIPAA | 5 | 2 | 3 | 0 |
| IEC 62443 | 1 | 0 | 1 | 0 |
| ISO 14971 | 1 | 1 | 0 | 0 |
| **Total** | **13** | **7** | **6** | **0** |

### Remaining Requirements (to be added):
- ~5 IEC 62304 detailed requirements (maintenance, problem resolution)
- ~3 FDA 21 CFR Part 11 requirements (electronic signatures, if applicable)
- ~3 ISO 13485 quality management requirements
- ~2 EU MDR/UKCA requirements (if EU/UK market targeted)

---

## 9. Compliance Verification

### 9.1 Testing Requirements

Each regulatory requirement must be verified through:
- **Design Review:** Architecture/design addresses requirement
- **Code Inspection:** Implementation reviewed for compliance
- **Testing:** Functional testing verifies requirement
- **Documentation:** Evidence documented in compliance matrix

### 9.2 Regulatory Submission Artifacts

For regulatory approval (FDA 510(k), EU MDR), prepare:
- **Software Documentation Package:**
  - Software Requirements Specification (this document set)
  - Software Architecture Document
  - Software Design Specification
  - Software Test Report
  - Traceability Matrix
- **Risk Management File** (ISO 14971)
- **Quality Management System** (ISO 13485)
- **Clinical Evaluation** (if required)
- **Usability Engineering File** (IEC 62366)

### 9.3 Post-Market Requirements

After market release:
- **Adverse Event Reporting:** Report device malfunctions to FDA/regulators
- **Cybersecurity:** Monitor threats, release patches
- **Field Corrective Actions:** If safety issue identified
- **Annual Review:** Review risk management file, update as needed

---

## 10. Related Documents

- **Security:** [../architecture_and_design/06_SECURITY.md](../architecture_and_design/06_SECURITY.md)
- **Testing:** [../architecture_and_design/18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)
- **Non-Functional Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](./04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **All Requirements Documents:** Collectively form Software Requirements Specification (SRS)

---

*Regulatory compliance is not optional. These requirements ensure patient safety, privacy, and legal compliance. Non-compliance risks regulatory rejection, legal liability, and patient harm.*

