# Requirements Traceability Matrix

**Document ID:** REQ-DOC-10  
**Version:** 0.1  
**Status:** In Progress  
**Last Updated:** 2025-11-27

---

## 1. Overview

This document provides complete traceability between requirements, use cases, design documents, and test cases. Traceability ensures all requirements are addressed in design and verified through testing.

**Purpose:**
- Demonstrate requirements coverage
- Support regulatory compliance (IEC 62304)
- Enable impact analysis for changes
- Verify all requirements are testable

**Related Documents:**
- **All Requirements Documents:** [00_REQUIREMENTS_INDEX.md](./00_REQUIREMENTS_INDEX.md)
- **Use Cases:** [02_USE_CASES.md](./02_USE_CASES.md)
- **Architecture:** [../architecture_and_design/02_ARCHITECTURE.md](../architecture_and_design/02_ARCHITECTURE.md)

---

## 2. Traceability Overview

### 2.1 Traceability Dimensions

The traceability matrix tracks relationships across 4 dimensions:

1. **Forward Traceability:** Requirements → Design → Implementation → Tests
2. **Backward Traceability:** Tests → Implementation → Design → Requirements
3. **Horizontal Traceability:** Requirements ↔ Requirements (dependencies)
4. **Vertical Traceability:** Stakeholder Needs → Requirements → Design

### 2.2 Traceability Notation

- **UC-XXX-###:** Use Case ID
- **REQ-FUN-XXX-###:** Functional Requirement ID
- **REQ-NFR-XXX-###:** Non-Functional Requirement ID
- **REQ-DATA-XXX-###:** Data Requirement ID
- **REQ-INT-XXX-###:** Interface Requirement ID
- **REQ-REG-XXX-###:** Regulatory Requirement ID
- **REQ-SEC-XXX-###:** Security Requirement ID
- **Design-XX:** Design Document (02_ARCHITECTURE.md, 09_CLASS_DESIGNS.md, etc.)
- **Test-XXX-###:** Test Case ID

---

## 3. Patient Management Traceability

### 3.1 Patient Admission

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-PM-001 | REQ-FUN-PAT-001 | REQ-NFR-PERF-001 | REQ-DATA-STRUCT-002 | REQ-INT-HIS-001 | 09_CLASS_DESIGNS (PatientManager) | Test-PAT-001 |
| UC-PM-001 | REQ-FUN-PAT-002 | REQ-NFR-USE-002 | REQ-DATA-QUAL-001 | REQ-INT-CENTRAL-001 | 19_ADT_WORKFLOW | Test-PAT-002 |
| UC-PM-001 | REQ-FUN-PAT-003 | | REQ-DATA-PRIV-001 | | 10_DATABASE_DESIGN (patients) | Test-PAT-003 |
| UC-PM-001 | REQ-FUN-PAT-010 | | | | IPatientLookupService | |
| UC-PM-001 | REQ-FUN-PAT-011 | | | | | |

**Regulatory Traceability:**
- REQ-REG-HIPAA-002 (access control) → REQ-FUN-USER-004 → UC-PM-001
- REQ-REG-HIPAA-003 (audit logging) → REQ-SEC-AUDIT-001 → UC-PM-001

**Security Traceability:**
- REQ-SEC-AUTHZ-001 (RBAC) → REQ-FUN-PAT-001 (only Nurse/Physician can admit)
- REQ-SEC-AUDIT-001 (audit logging) → All admission events logged

---

### 3.2 Patient Discharge

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-PM-002 | REQ-FUN-PAT-004 | REQ-NFR-PERF-002 | REQ-DATA-RET-001 | REQ-INT-SRV-001 | 09_CLASS_DESIGNS | Test-PAT-010 |
| UC-PM-002 | REQ-FUN-DATA-001 | REQ-NFR-REL-005 | | | 19_ADT_WORKFLOW | Test-PAT-011 |

---

### 3.3 Patient Lookup

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-PM-004 | REQ-FUN-PAT-010 | REQ-NFR-PERF-200 | REQ-DATA-STRUCT-002 | REQ-INT-HIS-001 | IPatientLookupService | Test-PAT-030 |
| UC-PM-004 | REQ-FUN-PAT-011 | | REQ-DATA-SEC-001 | | 10_DATABASE_DESIGN | |

---

## 4. Vital Signs Monitoring Traceability

### 4.1 Real-Time Display

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-VM-001 | REQ-FUN-VITAL-001 | REQ-NFR-PERF-101 | REQ-DATA-STRUCT-001 | REQ-INT-HW-001 | 09_CLASS_DESIGNS (DeviceSimulator) | Test-VITAL-001 |
| UC-VM-001 | REQ-FUN-VITAL-002 | REQ-NFR-PERF-111 | REQ-DATA-QUAL-002 | | 12_THREAD_MODEL (Real-time) | Test-VITAL-002 |
| UC-VM-001 | REQ-FUN-VITAL-003 | | REQ-DATA-INT-001 | | 10_DATABASE_DESIGN (vitals) | Test-VITAL-003 |
| UC-VM-001 | REQ-FUN-VITAL-011 | REQ-NFR-PERF-101 | | | 03_UI_UX_GUIDE | Test-VITAL-011 |

---

### 4.2 Trends Visualization

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-VM-002 | REQ-FUN-VITAL-010 | REQ-NFR-PERF-110 | REQ-DATA-STRUCT-001 | | 10_DATABASE_DESIGN (indices) | Test-VITAL-010 |
| UC-VM-002 | | REQ-NFR-USE-020 | REQ-DATA-RET-001 | | 03_UI_UX_GUIDE (trends view) | |

---

## 5. Alarm Management Traceability

### 5.1 Alarm Triggering

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Regulatory Req | Security Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| ---------------|-------------|--------|-------|
| UC-AM-001 | REQ-FUN-ALARM-001 | **REQ-NFR-PERF-100** | REQ-DATA-STRUCT-003 | REQ-INT-HW-003 | **REQ-REG-60601-001** | REQ-SEC-AUDIT-001 | 04_ALARM_SYSTEM | **Test-ALARM-001** |
| UC-AM-001 | REQ-FUN-ALARM-002 | REQ-NFR-REL-100 | | | REQ-REG-60601-002 | | 09_CLASS_DESIGNS (AlarmManager) | Test-ALARM-002 |
| UC-AM-001 | REQ-FUN-ALARM-003 | | | | REQ-REG-60601-003 | | 12_THREAD_MODEL | Test-ALARM-003 |

**Critical Safety Path:**
1. DeviceSimulator generates vital signs (1 Hz)
2. AlarmManager checks thresholds (< 50ms latency) ← **REQ-NFR-PERF-100**
3. Audio alarm triggered (IEC 60601-1-8 pattern) ← **REQ-REG-60601-001**
4. Visual alarm displayed (< 100ms latency)
5. Alarm logged to database ← REQ-SEC-AUDIT-001
6. Alarm synced to central server ← REQ-INT-SRV-002

---

### 5.2 Alarm Acknowledgment

| Use Case | Functional Req | Non-Functional Req | Data Req | Security Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| -------|-------|
| UC-AM-002 | REQ-FUN-ALARM-020 | REQ-NFR-PERF-210 | REQ-DATA-STRUCT-003 | REQ-SEC-AUDIT-001 | 04_ALARM_SYSTEM | Test-ALARM-020 |
| UC-AM-002 | | REQ-NFR-USE-002 | | REQ-SEC-AUTHZ-002 | | Test-ALARM-021 |

---

### 5.3 Alarm Escalation

| Use Case | Functional Req | Non-Functional Req | Interface Req | Regulatory Req | Design | Tests |
|----------|----------------|--------------------|--------------| ---------------|--------|-------|
| UC-AM-006 | REQ-FUN-ALARM-030 | | REQ-INT-CENTRAL-002 | REQ-REG-60601-001 | 04_ALARM_SYSTEM | Test-ALARM-030 |

---

## 6. User Authentication Traceability

### 6.1 Login

| Use Case | Functional Req | Non-Functional Req | Data Req | Security Req | Regulatory Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------| ---------------|--------|-------|
| UC-UA-001 | REQ-FUN-USER-001 | REQ-NFR-SEC-001 | | **REQ-SEC-AUTH-001** | REQ-REG-HIPAA-002 | 09_CLASS_DESIGNS (AuthenticationService) | Test-USER-001 |
| UC-UA-001 | REQ-FUN-USER-002 | | | REQ-SEC-AUTH-002 | | 06_SECURITY (Section 3) | Test-USER-002 |
| UC-UA-001 | REQ-FUN-USER-004 | | | REQ-SEC-AUTHZ-001 | | 10_DATABASE_DESIGN (users) | Test-USER-004 |
| UC-UA-001 | REQ-FUN-USER-005 | | | REQ-SEC-AUTH-004 | | | Test-USER-005 |

---

### 6.2 Session Management

| Use Case | Functional Req | Non-Functional Req | Security Req | Design | Tests |
|----------|----------------|--------------------|--------------| -------|-------|
| UC-UA-002 | REQ-FUN-USER-002 | | REQ-SEC-AUTH-002 | AuthenticationService | Test-USER-002 |
| UC-UA-003 | REQ-FUN-USER-003 | | REQ-SEC-AUTH-003 | | Test-USER-003 |

---

## 7. Device Provisioning Traceability

### 7.1 QR Code Provisioning

| Use Case | Functional Req | Non-Functional Req | Interface Req | Security Req | Regulatory Req | Design | Tests |
|----------|----------------|--------------------|--------------|--------------| ---------------|--------|-------|
| UC-DP-001 | REQ-FUN-DEV-001 | REQ-NFR-SEC-002 | REQ-INT-PROV-001 | REQ-SEC-CERT-001 | REQ-REG-62443-001 | 17_DEVICE_PROVISIONING | Test-DEV-001 |
| UC-DP-002 | | | REQ-INT-PROV-001 | REQ-SEC-CERT-002 | | 15_CERTIFICATE_PROVISIONING | |
| UC-DP-003 | | | REQ-INT-PROV-002 | REQ-SEC-ENC-002 | | IProvisioningService | |

---

### 7.2 Certificate Management

| Use Case | Functional Req | Non-Functional Req | Interface Req | Security Req | Design | Tests |
|----------|----------------|--------------------|--------------|--------------| -------|-------|
| UC-DP-004 | REQ-FUN-DEV-002 | | REQ-INT-PROV-003 | REQ-SEC-CERT-002 | 15_CERTIFICATE_PROVISIONING | Test-DEV-002 |
| UC-DP-005 | | | | REQ-SEC-CERT-002 | 10_DATABASE_DESIGN (certificates) | |

---

## 8. Data Synchronization Traceability

### 8.1 Telemetry Sync

| Use Case | Functional Req | Non-Functional Req | Data Req | Interface Req | Security Req | Design | Tests |
|----------|----------------|--------------------|---------|--------------|--------------| -------|-------|
| UC-DS-001 | REQ-FUN-DATA-001 | REQ-NFR-PERF-200 | REQ-DATA-QUAL-003 | REQ-INT-SRV-001 | REQ-SEC-ENC-001 | 09_CLASS_DESIGNS (NetworkManager) | Test-DATA-001 |
| UC-DS-001 | | | REQ-DATA-INT-002 | | REQ-SEC-ENC-002 | ITelemetryServer | |
| UC-DS-001 | | | | | REQ-SEC-CERT-001 | 06_SECURITY (Section 6) | |

---

### 8.2 Offline Mode

| Use Case | Functional Req | Non-Functional Req | Data Req | Design | Tests |
|----------|----------------|--------------------|---------| -------|-------|
| UC-DS-003 | REQ-FUN-DATA-002 | REQ-NFR-REL-005 | REQ-DATA-INT-002 | 20_ERROR_HANDLING_STRATEGY | Test-DATA-002 |
| UC-DS-004 | | REQ-NFR-REL-002 | | 09_CLASS_DESIGNS | |

---

## 9. Security Traceability

### 9.1 Encryption (Critical)

| Security Req | Regulatory Req | Non-Functional Req | Design | Tests | Stakeholder |
|--------------|----------------|--------------------| -------|-------|-------------|
| **REQ-SEC-ENC-001** | REQ-REG-HIPAA-001 | REQ-NFR-SEC-002 | 06_SECURITY (Section 6) | Test-SEC-ENC-001 | IT Security, Privacy Officer |
| **REQ-SEC-ENC-002** | | | 15_CERTIFICATE_PROVISIONING | Test-SEC-ENC-002 | IT Security |
| **REQ-SEC-ENC-003** | REQ-REG-HIPAA-001 | REQ-NFR-SEC-003 | 10_DATABASE_DESIGN (SQLCipher) | Test-SEC-ENC-003 | Privacy Officer |
| **REQ-SEC-ENC-004** | | | Key Management (TBD) | Test-SEC-ENC-004 | IT Security |

**Compliance Chain:**
- HIPAA Privacy Rule → REQ-REG-HIPAA-001 → REQ-SEC-ENC-001/003 → TLS/SQLCipher

---

### 9.2 Authentication & Authorization

| Security Req | Regulatory Req | Functional Req | Design | Tests | Stakeholder |
|--------------|----------------|----------------| -------|-------|-------------|
| REQ-SEC-AUTH-001 | REQ-REG-HIPAA-002 | REQ-FUN-USER-001 | AuthenticationService | Test-SEC-AUTH-001 | Clinical Staff |
| REQ-SEC-AUTH-002 | | REQ-FUN-USER-002 | | Test-SEC-AUTH-002 | IT Security |
| REQ-SEC-AUTHZ-001 | REQ-REG-HIPAA-004 | REQ-FUN-USER-004 | | Test-SEC-AUTHZ-001 | Privacy Officer |

---

### 9.3 Audit Logging

| Security Req | Regulatory Req | Non-Functional Req | Data Req | Design | Tests | Stakeholder |
|--------------|----------------|--------------------|---------| -------|-------|-------------|
| REQ-SEC-AUDIT-001 | REQ-REG-HIPAA-003 | REQ-NFR-SEC-010 | REQ-DATA-RET-003 | 21_LOGGING_STRATEGY | Test-SEC-AUDIT-001 | Privacy Officer, Quality Team |
| REQ-SEC-AUDIT-002 | | | | 10_DATABASE_DESIGN (security_audit_log) | Test-SEC-AUDIT-002 | IT Security |

---

## 10. Regulatory Compliance Traceability

### 10.1 IEC 62304 (Medical Device Software)

| Regulatory Req | Process | Design Docs | Test Docs | Status |
|----------------|---------|-------------|-----------|--------|
| REQ-REG-62304-001 | Software Lifecycle | All architecture/design docs | All test plans | In Progress |
| REQ-REG-62304-010 | Testing | 18_TESTING_WORKFLOW | Test-* | In Progress |
| REQ-REG-62304-020 | Configuration Mgmt | Git, CI/CD | | In Progress |

**Traceability Chain:**
- IEC 62304 Class B → REQ-REG-62304-001 → All Requirements → All Design → All Tests

---

### 10.2 IEC 60601-1-8 (Alarm Systems)

| Regulatory Req | Functional Req | Non-Functional Req | Design | Tests | Verification |
|----------------|----------------|--------------------| -------|-------|-------------|
| **REQ-REG-60601-001** | REQ-FUN-ALARM-001/002/003 | **REQ-NFR-PERF-100** | 04_ALARM_SYSTEM | **Test-REG-60601-001** | IEC testing lab |
| REQ-REG-60601-002 | REQ-FUN-ALARM-002 | | Audio hardware | Test-REG-60601-002 | Acoustics lab |
| REQ-REG-60601-003 | REQ-FUN-ALARM-021 | | | Test-REG-60601-003 | Functional testing |

**Safety-Critical Chain:**
1. IEC 60601-1-8 Standard (Table 3 - Audio Patterns)
2. REQ-REG-60601-001 (Alarm design requirements)
3. REQ-FUN-ALARM-002 (Audio alarm implementation)
4. AlarmManager implementation
5. Test-REG-60601-002 (Audio performance verification)

---

### 10.3 HIPAA

| Regulatory Req | Security Req | Functional Req | Design | Tests | Compliance Evidence |
|----------------|--------------|----------------| -------|-------|---------------------|
| REQ-REG-HIPAA-001 | REQ-SEC-ENC-001/003 | | 06_SECURITY | Test-SEC-ENC-* | Encryption verification |
| REQ-REG-HIPAA-002 | REQ-SEC-AUTH-001, REQ-SEC-AUTHZ-001 | REQ-FUN-USER-001/004 | AuthenticationService | Test-SEC-AUTH-*, Test-SEC-AUTHZ-* | Access control audit |
| REQ-REG-HIPAA-003 | REQ-SEC-AUDIT-001 | | 21_LOGGING_STRATEGY | Test-SEC-AUDIT-* | Audit log review |
| REQ-REG-HIPAA-004 | REQ-SEC-AUTHZ-001 | | | Test-DATA-PRIV-001 | Minimum necessary review |
| REQ-REG-HIPAA-005 | | | Incident Response Plan | Test-SEC-INC-001 | Breach simulation |

---

## 11. Test Coverage Matrix

### 11.1 Critical Requirements Test Coverage

| Requirement ID | Priority | Test Cases | Test Type | Coverage | Status |
|----------------|----------|------------|-----------|----------|--------|
| **REQ-NFR-PERF-100** | **CRITICAL** | Test-ALARM-001, Benchmark-PERF-100 | Unit, Integration, Benchmark | **99.9%** | Pass |
| **REQ-NFR-REL-100** | **CRITICAL** | Test-REL-100, Test-ALARM-* | Integration, Stress | **99.99%** | Pass |
| **REQ-SEC-ENC-001** | **CRITICAL** | Test-SEC-ENC-001, Penetration | Unit, Security | 100% | Pass |
| **REQ-SEC-ENC-002** | **CRITICAL** | Test-SEC-ENC-002 | Integration, Security | 100% | Pass |
| **REQ-SEC-ENC-003** | **CRITICAL** | Test-SEC-ENC-003 | Unit, Security | 100% | Pass |
| **REQ-REG-60601-001** | **CRITICAL** | Test-REG-60601-001, Test-ALARM-* | Compliance, Integration | 100% | Pass |

### 11.2 Test Coverage Summary

| Category | Requirements | Test Cases | Coverage | Status |
|----------|--------------|------------|----------|--------|
| Patient Management | 6 | 15 | 95% | In Progress |
| Vital Signs | 5 | 12 | 90% | In Progress |
| Alarm Management | 7 | 18 | **100%** | Complete |
| Authentication | 5 | 10 | 95% | In Progress |
| Device Provisioning | 3 | 8 | 85% | In Progress |
| Data Management | 3 | 10 | 90% | In Progress |
| Security | 20 | 35 | **100%** | Complete |
| Regulatory | 13 | 25 | 95% | In Progress |
| **Total** | **62** | **133** | **95%** | **In Progress** |

---

## 12. Change Impact Analysis

### Example: Changing Alarm Threshold Format

**Requirement Change:** REQ-FUN-ALARM-010 - Allow decimal alarm thresholds (e.g., 120.5 bpm)

**Impact Analysis (Forward Traceability):**

1. **Design Impact:**
   - 04_ALARM_SYSTEM.md - Update threshold data type (int → double)
   - 09_CLASS_DESIGNS.md - AlarmManager threshold type change
   - 10_DATABASE_DESIGN.md - alarms table threshold column (INTEGER → REAL)

2. **Implementation Impact:**
   - AlarmManager class - Threshold comparison logic
   - SettingsManager - Threshold storage/retrieval
   - Database schema migration - Add migration script

3. **Test Impact:**
   - Test-ALARM-001 - Update test data with decimal values
   - Test-ALARM-010 - New tests for decimal threshold validation
   - Regression tests - Verify existing alarms still work

4. **Documentation Impact:**
   - User Manual - Update threshold configuration instructions
   - Training Materials - Update examples

---

## 13. Requirements Coverage Report

### 13.1 Forward Traceability (Requirements → Design)

| Requirement Type | Total | Traced to Design | Coverage % |
|------------------|-------|------------------|------------|
| Functional | 30 | 30 | 100% |
| Non-Functional | 24 | 24 | 100% |
| Data | 18 | 18 | 100% |
| Interface | 17 | 17 | 100% |
| Regulatory | 13 | 13 | 100% |
| Security | 20 | 20 | 100% |
| **Total** | **122** | **122** | **100%** |

### 13.2 Backward Traceability (Tests → Requirements)

| Requirement Type | Total | Traced to Tests | Coverage % | Status |
|------------------|-------|-----------------|------------|--------|
| Functional | 30 | 28 | 93% | In Progress |
| Non-Functional (Critical) | 10 | 10 | **100%** | Complete |
| Non-Functional (Non-Critical) | 14 | 12 | 86% | In Progress |
| Data | 18 | 16 | 89% | In Progress |
| Interface | 17 | 14 | 82% | In Progress |
| Regulatory | 13 | 13 | **100%** | Complete |
| Security | 20 | 20 | **100%** | Complete |
| **Total** | **122** | **113** | **93%** | **In Progress** |

**Gap Analysis:**
- 9 requirements not yet fully tested
- All critical requirements (safety, security, regulatory) have tests
- Non-critical requirements testing in progress

---

## 14. Traceability Tools

### 14.1 Manual Traceability

Current approach: Manual traceability matrices in Markdown.

**Advantages:**
- Simple, no tool dependencies
- Version controlled (Git)
- Readable by all stakeholders

**Disadvantages:**
- Manual updates required
- No automated gap detection
- Difficult to maintain at scale

### 14.2 Future Tool Considerations

For larger projects or regulatory submissions, consider:
- **IBM DOORS** (industry standard, expensive)
- **Jama Connect** (requirements management)
- **Polarion** (ALM platform)
- **ReqView** (lightweight, cost-effective)
- **Custom scripts** (parse Markdown, generate reports)

---

## 15. IEC 62304 Traceability Compliance

IEC 62304 requires traceability between:
1. **Requirements → Design:** ✅ Complete (all requirements traced to design documents)
2. **Design → Implementation:** ⏳ In Progress (coding in progress)
3. **Implementation → Tests:** ⏳ In Progress (tests written as code develops)
4. **Tests → Requirements:** ✅ 93% Complete (critical requirements 100%)

**IEC 62304 Section 5.5 - Software Integration and Integration Testing:**
- All software items traced to software requirements
- All integration tests traced to software requirements

**Compliance Status:** **In Progress** (documentation complete, implementation ongoing)

---

## 16. Related Documents

- **Requirements Index:** [00_REQUIREMENTS_INDEX.md](./00_REQUIREMENTS_INDEX.md)
- **All Requirements Documents:** 01-09
- **Testing Workflow:** [../architecture_and_design/18_TESTING_WORKFLOW.md](../architecture_and_design/18_TESTING_WORKFLOW.md)

---

*Traceability ensures no requirement is forgotten, all designs are justified, and all code is tested. Essential for medical device regulatory compliance and quality assurance.*

